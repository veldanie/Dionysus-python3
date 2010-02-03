#include <utilities/log.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
namespace bl = boost::lambda;

#include <boost/iterator/zip_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/foreach.hpp>

#ifdef LOGGING
static rlog::RLogChannel* rlLSVineyard =            DEF_CHANNEL("lsvineyard/info", rlog::Log_Debug);
static rlog::RLogChannel* rlLSVineyardDebug =       DEF_CHANNEL("lsvineyard/debug", rlog::Log_Debug);
#endif // LOGGING

#ifdef COUNTERS
static Counter*  cVertexTransposition =                     GetCounter("lsfiltration/transposition");       // counts number of vertex transpositions
static Counter*  cAttachment =                              GetCounter("lsfiltration/attachment");          // counts the number of attachment changes
#endif


template<class V, class VE, class S, class F>
template<class VertexIterator>
LSVineyard<V,VE,S,F>::
LSVineyard(VertexIterator begin, VertexIterator end, 
           LSFiltration& fltr,
           const VertexEvaluator& veval):
    filtration_(fltr),
    vertices_(begin, end),
    persistence_(filtration_),
    veval_(veval), vcmp_(veval_), scmp_(vcmp_),
    pfmap_(persistence_.make_simplex_map(filtration_)),
    time_count_(0)
{
    vertices_.sort(KineticVertexComparison(vcmp_));     // sort vertices w.r.t. vcmp_
#if LOGGING    
    rLog(rlLSVineyardDebug, "Vertex order:");
    for (typename VertexContainer::iterator cur = vertices_.begin(); cur != vertices_.end(); ++cur)
        rLog(rlLSVineyardDebug, "  %d", cur->vertex());
#endif

    // Record Vertex -> LSFIndex map
    VertexLSFIndexMap vimap;
    for (LSFIndex i = filtration().begin(); i != filtration().end(); ++i)
    {
        const Simplex& s = *i;
        rLog(rlLSVineyardDebug, "Simplex: %s", tostring(*i).c_str());
        if (s.dimension() == 0)
            vimap[s.vertices().front()] = i;
    }

    // Assign vertex attachments and simplex_index
    OffsetMap<LSFIndex, iterator>   fpmap(filtration().begin(), persistence().begin());
    for (typename VertexContainer::iterator vi = vertices_.begin(); vi != vertices_.end(); ++vi)
    {
        LSFIndex i = vimap[vi->vertex()];
        const Simplex& s = *i;
        AssertMsg(s.vertices().front() == vi->vertex(), "In constructor, simplices and vertices must match.");
        vertices_.modify(vi,    b::bind(&KineticVertexType::set_simplex_index, bl::_1, i));    // vi->set_simplex_index(i)
        set_attachment(fpmap[i], vi);
        rLog(rlLSVineyardDebug, "%s attached to %d", tostring(*i).c_str(), vi->vertex());
    }

    // Assign attachments for all the simplices
    VertexAttachmentComparison  vacmp(vimap, *this);
    for (LSFIndex i = filtration().begin(); i != filtration().end(); ++i)
        set_attachment(fpmap[i], fpmap[vimap[*std::max_element(i->vertices().begin(), i->vertices().end(), vacmp)]]->attachment);

    // Order filtration_ and persistence_ based on attachment
    rLog(rlLSVineyardDebug, "Ordering the simplices");
   
    std::vector<SimplexPersistenceElementTuple> fporder
                (b::make_zip_iterator(b::make_tuple(filtration().begin(),    persistence().begin())),
                 b::make_zip_iterator(b::make_tuple(filtration().end(),      persistence().end())));
    std::sort(fporder.begin(), fporder.end(), AttachmentCmp());

    // Rearrage filtration
    std::vector< b::reference_wrapper<const Simplex> >                sv; 
    BOOST_FOREACH(const SimplexPersistenceElementTuple& t, fporder)   sv.push_back(b::get<0>(t));
    filtration_.rearrange (sv.begin());

    // Rearrange persistence
    std::vector< b::reference_wrapper<const typename Persistence::Element> >    pev; 
    BOOST_FOREACH(const SimplexPersistenceElementTuple& t, fporder)   pev.push_back(b::get<1>(t));
    persistence_.rearrange(pev.begin());

#if LOGGING
    rLog(rlLSVineyardDebug, "Simplices:");
    for(iterator i = persistence().begin(); i != persistence().end(); ++i)
        rLog(rlLSVineyardDebug, "  %s attached to %d", tostring(pfmap(i)).c_str(), i->attachment->vertex());
#endif

    // Pair simplices
    rLog(rlLSVineyardDebug, "Initializing LSVineyard");
    persistence_.pair_simplices();
    rLog(rlLSVineyardDebug, "Simplices paired");

    evaluator_ = new StaticEvaluator(*this, time_count_);
    vineyard_.set_evaluator(evaluator_);
    vineyard_.start_vines(persistence_.begin(), persistence_.end());
}

template<class V, class VE, class S, class F>
LSVineyard<V,VE,S,F>::
~LSVineyard()
{
    delete evaluator_;
}

template<class V, class VE, class S, class F_>
void                    
LSVineyard<V,VE,S,F_>::
compute_vineyard(const VertexEvaluator& veval, bool explicit_events)
{
    typedef Traits::Kinetic_kernel::Point_1                                 Point;
    typedef Traits::Kinetic_kernel::Function_kernel::Construct_function     CF; 
    typedef Traits::Kinetic_kernel::Motion_function                         F; 
    
    Traits tr(0,1);
    Simulator::Handle sp = tr.simulator_handle();
    ActivePointsTable::Handle apt = tr.active_points_1_table_handle();
    Sort sort(tr, SortVisitor(*this));
    
    // Setup the (linear) trajectories
    rLog(rlLSVineyard, "Setting up trajectories");
    CF cf; 
    kinetic_map_.clear();
    for (VertexIndex cur = vertices_.begin(); cur != vertices_.end(); ++cur)
    {
        VertexValue val0 = veval_(cur->vertex());
        VertexValue val1 = veval(cur->vertex());
        rLog(rlLSVineyardDebug, "Vertex %d: %f -> %f", cur->vertex(), val0, val1);
        F x = cf(F::NT(val0), F::NT(val1 - val0));          // x = val0 + (val1 - val0)*t = (1-t)*val0 + t*val1
        Point p(x);
        vertices_.modify(cur,   b::bind(&KineticVertexType::set_kinetic_key, bl::_1, apt->insert(p)));    // cur->set_kinetic_key(apt->insert(p))
        kinetic_map_[cur->kinetic_key()] = cur;
        rLog(rlLSVineyardDebug, "Scheduling: %d %s", cur->vertex(), tostring(x).c_str());
    }
    
    // Process all the events (compute the vineyard in the process)
    change_evaluator(new KineticEvaluator(*this, sp, apt, time_count_));
    if (explicit_events)
    {
        while (sp->next_event_time() < 1)
        {
            rLog(rlLSVineyardDebug, "Next event time: %f", sp->next_event_time());
            sp->set_current_event_number(sp->current_event_number() + 1);
            rLog(rlLSVineyardDebug, "Processed event");
        }
    } else
        sp->set_current_time(1.0);
    rLog(rlLSVineyard, "Processed %d events", sp->current_event_number());
    
    veval_ = veval;
    change_evaluator(new StaticEvaluator(*this, ++time_count_));
    vineyard_.record_diagram(persistence().begin(), persistence().end());
}
        
template<class V, class VE, class S, class F>
void                    
LSVineyard<V,VE,S,F>::
swap(Key a, Key b)
{
    rLog(rlLSVineyardDebug, "Entered swap");
    VertexIndex ao = kinetic_map_[a], bo = kinetic_map_[b];
    rLog(rlLSVineyardDebug, "Vertices: %d %d compare %d", ao->vertex(), bo->vertex(), vcmp_(ao->vertex(), bo->vertex()));
    AssertMsg(vcmp_(ao->vertex(), bo->vertex()), "In swap(a,b), a must precede b");
    transpose_vertices(ao);
    // AssertMsg(vcmp_(bo->vertex(), ao->vertex()), "In swap(a,b), b must precede a after the transposition");
}

template<class V, class VE, class S, class F>
void
LSVineyard<V,VE,S,F>::
change_evaluator(Evaluator* eval)
{
    AssertMsg(evaluator_ != 0, "change_evaluator() assumes that existing evaluator is not null");
        
    delete evaluator_;
    evaluator_ = eval;
    vineyard_.set_evaluator(evaluator_);
}

template<class V, class VE, class S, class F>
bool
LSVineyard<V,VE,S,F>::
transpose_vertices(VertexIndex vi)
{
    Count(cVertexTransposition);
    rLog(rlLSVineyard, "Transposing vertices (%d:%d, %d:%d)", vi->vertex(),             (vi -  vertices_.begin()),
                                                              b::next(vi)->vertex(),    (b::next(vi) - vertices_.begin()));

    DimensionFromIterator                       dim(pfmap_);
    TranspositionVisitor                        visitor(*this);

    OffsetMap<LSFIndex, iterator>   fpmap(filtration().begin(), persistence().begin());
    iterator i = fpmap[vi->simplex_index()];
    iterator i_prev = b::prior(i);
    iterator i_next = fpmap[b::next(vi)->simplex_index()];
    iterator i_next_prev = b::prior(i_next);           // transpositions are done in terms of the first index in the pair
    iterator j = b::next(i_next);
    
    VertexIndex     vi_next = b::next(vi);
    const Vertex&   v = vi->vertex();
    
    bool result = false;        // has a switch in pairing occurred
    
    // First move the vertex --- this can be sped up if we devise special "vertex transpose" operation
    rLog(rlLSVineyardDebug, "Starting to move the vertex");
    while (i_next_prev != i_prev)                       
    { 
        rLog(rlLSVineyardDebug, "  Transposing %s %s", tostring(pfmap(i_next_prev)).c_str(),
                                                       tostring(pfmap(b::next(i_next_prev))).c_str());
        result |= persistence_.transpose(i_next_prev, dim, visitor);
        AssertMsg((i_next_prev  <= persistence().iterator_to(i_next_prev->pair)) == i_next_prev->sign(), "Pairing must respect order");
        AssertMsg((i_next       <= persistence().iterator_to(i_next->pair))      == i_next->sign(),      "Pairing must respect order");
        i_next_prev = b::prior(i_next);
    }
    rLog(rlLSVineyardDebug, "Done moving the vertex");

    // Second, move the simplices attached to it
    rLog(rlLSVineyardDebug, "Moving attached simplices");
    // rLog(rlLSVineyardDebug, "  Considering %s", tostring(pfmap(j)).c_str());
    // rLog(rlLSVineyardDebug, "    attachment %d", j->attachment->vertex());
    while (j != persistence_.end() && j->attachment == vi_next)
    {
        rLog(rlLSVineyardDebug, "  Considering %s", tostring(pfmap(j)).c_str());
        if (pfmap(j).contains(v))       // j becomes attached to v and does not move
        {
            Count(cAttachment);
            rLog(rlLSVineyardDebug, "  Attachment changed for %s to %d", tostring(pfmap(j)).c_str(), vi->vertex());
            set_attachment(j, vi);
            AssertMsg(fpmap[vi->simplex_index()] < j, "The simplex must be attached to a preceding vertex");
            ++j;
            continue;
        }   

        iterator j_prev = j; ++j;
        while ((--j_prev)->attachment != vi_next)                // i.e., until we have reached vi_next (and the simplices that follow it) again
        {
            rLog(rlLSVineyardDebug, "    Moving: %s, %s", 
                                      tostring(pfmap(j_prev)).c_str(),
                                      tostring(pfmap(b::next(j_prev))).c_str());
            AssertMsg(j_prev->attachment == vi, "Simplex preceding the one being moved must be attached to v");
            result |= persistence_.transpose(j_prev, dim, visitor);
            AssertMsg((j_prev  <= persistence().iterator_to(j_prev->pair)) == j_prev->sign(), "Pairing must respect order");
            --j_prev;
        }
    }
    rLog(rlLSVineyard, "Done moving attached simplices");
    vertices_.relocate(vi, vi_next);                    // swap vi and vi_next
    
#if LSVINEYARD_CONSISTENCY    
    AssertMsg(verify_pairing(), "Pairing must be correct after vertex transposition");
#endif

    return result;
}

template<class V, class VE, class S, class F>
bool
LSVineyard<V,VE,S,F>::
verify_pairing() const
{
    rLog(rlLSVineyardDebug, "Verifying pairing");
    StaticPersistence<> p(filtration());
    p.pair_simplices(false);
    iterator                        i     = persistence().begin();
    StaticPersistence<>::iterator   ip    = p.begin();
    StaticPersistence<>::SimplexMap<LSFiltration>       m = p.make_simplex_map(filtration());

    while (ip != p.end())
    {
        if (&pfmap(i) != &m[ip])
        {
            rError("DP: %s %s", tostring(pfmap(i)).c_str(), tostring(pfmap(i->pair)).c_str());
            rError("SP: %s %s", tostring(m[ip]).c_str(), tostring(m[ip->pair]).c_str());
            rError("The order must match");
            return false;
        }
        if (&pfmap(i->pair) != &m[ip->pair])
        {
            rError("DP: %s %s", tostring(pfmap(i)).c_str(), tostring(pfmap(i->pair)).c_str());
            rError("SP: %s %s", tostring(m[ip]).c_str(), tostring(m[ip->pair]).c_str());
            rError("The pairing must match");
            return false;
        }
        ++i; ++ip;
    }

    return true;
}


/* Evaluators */
template<class V, class VE, class S, class C>
class LSVineyard<V,VE,S,C>::StaticEvaluator: public Evaluator
{
    public:
                                StaticEvaluator(const LSVineyard& v, RealType time): 
                                    time_(time), vineyard_(v)                               {}

        virtual RealType        time() const                                                { return time_; }
        virtual RealType        operator()(Index i) const                                   { return vineyard_.simplex_value(vineyard_.pfmap(i)); }
        virtual Dimension       dimension(Index i) const                                    { return vineyard_.pfmap(i).dimension(); }
                                
    private:
        RealType                time_;
        const LSVineyard&       vineyard_;
};

template<class V, class VE, class S, class C>
class LSVineyard<V,VE,S,C>::KineticEvaluator: public Evaluator
{
    public:
                                KineticEvaluator(const LSVineyard& v, Simulator::Handle sp, ActivePointsTable::Handle apt, RealType time_offset): 
                                    vineyard_(v), sp_(sp), apt_(apt), time_offset_(time_offset)           {}

        virtual RealType        time() const                                                { return time_offset_ + CGAL::to_double(get_time()); }
        virtual RealType        operator()(Index i) const                                   
        {
            rLog(rlLSVineyard, "%s (attached to %d): %s(%f) = %f", tostring(vineyard_.pfmap(i)).c_str(),
                                                                   i->attachment->vertex(),
                                                                   tostring(apt_->at(i->attachment->kinetic_key()).x()).c_str(),
                                                                   get_time(),
                                                                   CGAL::to_double(apt_->at(i->attachment->kinetic_key()).x()(get_time())));
            return CGAL::to_double(apt_->at(i->attachment->kinetic_key()).x()(get_time())); 
        }
        virtual Dimension       dimension(Index i) const                                    { return vineyard_.pfmap(i).dimension(); }

    private:
        Simulator::Time         get_time() const                                            { return sp_->current_time(); }
        
        const LSVineyard&           vineyard_;
        Simulator::Handle           sp_;
        ActivePointsTable::Handle   apt_;
        RealType                    time_offset_;
};


template<class V, class VE, class S, class C>
class LSVineyard<V,VE,S,C>::VertexAttachmentComparison: 
    public std::binary_function<Vertex, Vertex, bool>
{
    public:
                                VertexAttachmentComparison(const VertexLSFIndexMap& vimap, 
                                                           const LSVineyard& vnrd):
                                    vimap_(vimap), vnrd_(vnrd)                              {}
        bool                    operator()(Vertex v1, Vertex v2) const                      
        { return vnrd_.filtration_attachment(vimap_.find(v1)->second) < vnrd_.filtration_attachment(vimap_.find(v2)->second); }

    private:
        const VertexLSFIndexMap&    vimap_;
        const LSVineyard&           vnrd_;
};


template<class V, class VE, class S, class C>
struct LSVineyard<V,VE,S,C>::AttachmentCmp: 
    public std::binary_function<const SimplexPersistenceElementTuple&, const SimplexPersistenceElementTuple&, bool>
{
    bool        operator()(const SimplexPersistenceElementTuple& t1, const SimplexPersistenceElementTuple& t2) const
    {
        if (b::get<1>(t1).get().attachment == b::get<1>(t2).get().attachment)
            return b::get<0>(t1).get().dimension() < b::get<0>(t2).get().dimension();
        else
            return b::get<1>(t1).get().attachment  < b::get<1>(t2).get().attachment;
    }
};

