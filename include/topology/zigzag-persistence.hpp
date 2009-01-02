#include <utilities/log.h>
#include <boost/utility.hpp>
#include <algorithm>
#include <utilities/indirect.h>

#ifdef LOGGING
static rlog::RLogChannel* rlZigzagAdd =                   DEF_CHANNEL("topology/persistence/zigzag/add",    rlog::Log_Debug);
static rlog::RLogChannel* rlZigzagRemove =                DEF_CHANNEL("topology/persistence/zigzag/remove", rlog::Log_Debug);
#endif // LOGGING


template<class BID>
typename ZigzagPersistence<BID>::IndexDeathPair
ZigzagPersistence<BID>::
add(ZColumn bdry, const BirthID& birth)
{
    rLog(rlZigzagAdd,       "Entered ZigzagPersistence::add()");
    rLog(rlZigzagAdd,       "  Boundary: %s", bdry.tostring(out).c_str());
    rLog(rlZigzagAdd,       "  Boundary size: %d", bdry.size());

    {   // scoping to not pollute with the name order
        unsigned order      = s_list.empty() ? 0 : boost::prior(s_list.end())->order + 1;
        s_list.push_back(SimplexNode(order, z_list.end()));
    }
    SimplexIndex last_s     = boost::prior(s_list.end());
    last_s->low             = z_list.end();

    rLog(rlZigzagAdd,   "  Reducing among cycles");
    // Reduce bdry among the cycles
    BColumn v;                // representation of the boundary in the cycle basis
    while (!bdry.empty())
    {
        SimplexIndex l      = bdry.back();
        ZIndex k            = l->low;
        v.append(k, cmp);
        bdry.add(k->z_column, cmp);
        rLog(rlZigzagAdd,       "    Boundary: %s", bdry.tostring(out).c_str());
    }
    rLog(rlZigzagAdd,   "  Reduced among cycles");

    // Reduce v among boundaries
    BRow u;
    while (!(v.empty()))
    {
        ZIndex l = v.back();
        BIndex k = l->low;

        if (k == b_list.end())
            break;

        u.append(k, cmp);
        v.add(k->b_column, cmp);
    }
    rLog(rlZigzagAdd,   "  Reduced among boundaries");

    if (v.empty())
    {
        rLog(rlZigzagAdd,       "  Birth");

        // Birth
        int order                   = z_list.empty() ? 0 : boost::prior(z_list.end())->order + 1;
        z_list.push_back(ZNode(order, birth, b_list.end()));
        ZIndex last_z               = boost::prior(z_list.end());

        // Set z_column
        ZColumn& z                  = last_z->z_column;
        std::for_each(u.begin(), u.end(), make_adder(&BNode::c_column, z));
        z.append(last_s, cmp);

        // Set s_row
        std::for_each(z.begin(), z.end(), make_appender(&SimplexNode::z_row, last_z));

        // Set low
        last_z->low                 = b_list.end();
        last_s->low                 = last_z;

        return std::make_pair(last_s, Death());
    } else
    {
        rLog(rlZigzagAdd,       "  Death");

        // Death
        unsigned order              = b_list.empty() ? 0 : boost::prior(b_list.end())->order + 1;
        b_list.push_back(BNode(order));
        BIndex last_b               = boost::prior(b_list.end());
        
        // Set b_column and low
        last_b->b_column.swap(v);            
        last_b->b_column.back()->low = last_b;

        // Set b_row
        std::for_each(last_b->b_column.begin(), last_b->b_column.end(), make_appender(&ZNode::b_row, last_b));

        // Set c_column
        CColumn& c                  = last_b->c_column;
        std::for_each(u.begin(), u.end(), make_adder(&BNode::c_column, c));
        c.append(last_s, cmp);

        // Set c_row
        std::for_each(c.begin(), c.end(), make_appender(&SimplexNode::c_row, last_b));

        return std::make_pair(last_s, Death(last_b->b_column.back()->birth));
    }
}


template<class BID>
typename ZigzagPersistence<BID>::Death
ZigzagPersistence<BID>::
remove(SimplexIndex s, const BirthID& birth)
{
    rLog(rlZigzagRemove,        "Entered ZigzagPersistence::remove(%d)", s->order);
    AssertMsg(check_consistency(), "Must be consistent before removal");

    if (s->z_row.empty())
    {
        AssertMsg(!(s->c_row.empty()),  "Birth after removal, row in C must be non-empty");

        // Birth
        int order                   = z_list.empty() ? 0 : z_list.begin()->order - 1; 
        z_list.push_front(ZNode(order, birth, b_list.end()));
        ZIndex first_z              = z_list.begin();
        ZColumn& z                  = first_z->z_column;
        first_z->low                = b_list.end();
        
        // Prepend DC[j] = ZB[j] to Z
        BIndex j                    = s->c_row.front();
        std::for_each(j->b_column.begin(), j->b_column.end(), make_adder(&ZNode::z_column, z));
        std::for_each(z.begin(),           z.end(),           make_appender(&SimplexNode::z_row, first_z));

        // Prepend row of s in C to B
        first_z->b_row = s->c_row;      // copying instead of swapping is inefficient, 
                                        // but it simplifies logic when subtracting chains later
        std::for_each(first_z->b_row.begin(), first_z->b_row.end(), make_appender(&BNode::b_column, first_z));

        // Subtract C[j] from every column of C that contains s
        add_chains(first_z->b_row.begin(), first_z->b_row.end(), j, &BNode::c_column, &SimplexNode::c_row);

        // Subtract B[j] from every other column of B that has l
        ZIndex l                    = j->b_column.back();
        BRow   l_row                = l->b_row;     // again, copying is inefficient, but it simplifies the logic
        add_chains(l_row.begin(), l_row.end(), j, &BNode::b_column, &ZNode::b_row);

        // Drop j, l, and s
        // 
        // l->z_column is the only non-empty thing, but we drop it,
        // the basis is preserved because we added first_z
        l->z_column.back()->low     = z_list.end();
        std::for_each(l->z_column.begin(), l->z_column.end(), make_remover(&SimplexNode::z_row, l));
        AssertMsg(l->b_row.empty(),     "b_row of l must be empty before erasing in add()");
        AssertMsg(s->z_row.empty(),     "z_row of s must be empty before erasing in add()");
        AssertMsg(s->c_row.empty(),     "c_row of s must be empty before erasing in add()");
        b_list.erase(j);
        z_list.erase(l);
        s_list.erase(s);
        AssertMsg(check_consistency(),  "Must be consistent when done in add()");

        // Reduce Z
        SimplexIndex ls = first_z->z_column.back();
        while(ls->low != first_z)
        {
            if (ls->low == z_list.end())    { ls->low = first_z; break; }

            // if ls->low precedes first_z, swap them
            if (cmp(ls->low, first_z))      std::swap(ls->low, first_z);
            
            add_chain(ls->low, first_z, &ZNode::z_column, &SimplexNode::z_row);
            std::swap(ls->low, first_z);

            ls = first_z->z_column.back();
        }

        return Death();
    } else
    {
        // Death
        ZIndex j                    = s->z_row.front();
        CRow c_row                  = s->c_row;
        
        // Subtract Z[j] from every chain in C that contains s
        add_chains(c_row.begin(), c_row.end(), j, &BNode::c_column, &SimplexNode::c_row, &ZNode::z_column);
        
        /** 
         * Change basis to remove s from Z 
         * All the processing is done from back to front, so that when elements are removed from s->z_row, 
         * it doesn't invalidate the rest of the elements (iterators) in the row.
         */
        // Compute reducers --- columns that we will be adding to other columns
        typedef typename ZRow::reverse_iterator             ZRowReverseIterator;
        typedef std::list<ZRowReverseIterator>              ReducersContainer;
        ReducersContainer  reducers;                        // list of ZColumns that we will be adding to other columns
        reducers.push_back(boost::prior(s->z_row.rend()));  // j is the first reducer
        SimplexIndex low            = j->z_column.back();
        for (typename ZRow::iterator cur = s->z_row.begin(); cur != s->z_row.end(); ++cur)
            if (cmp((*cur)->z_column.back(), low))
            { 
                reducers.push_back(ZRowReverseIterator(cur)); 
                low = (*(reducers.back()))->z_column.back();
            }
        reducers.push_back(s->z_row.rbegin());
        //rLog(rlZigzagRemove,        "  Reducers size: %d", reducers.size());

        // Add each reducer to the columns that follow them until the next reducer
        for (typename ReducersContainer::reverse_iterator cur = boost::next(reducers.rbegin()); cur != reducers.rend(); ++cur)
        {
            change_basis(*boost::prior(cur), *cur, **cur, 
                         &ZNode::z_column, &SimplexNode::z_row,
                         &ZNode::b_row,    &BNode::b_column);
            (**cur)->z_column.back()->low = **boost::prior(cur);
        }
        
        // Drop j and s
        BirthID birth               = j->birth;
        std::for_each(j->z_column.begin(), j->z_column.end(), make_remover(&SimplexNode::z_row, j));
        AssertMsg(j->b_row.empty(),     "b_row of j must be empty before erasing in remove()");
        AssertMsg(s->z_row.empty(),     "z_row of s must be empty before erasing in remove()");
        AssertMsg(s->c_row.empty(),     "c_row of s must be empty before erasing in remove()");
        z_list.erase(j);
        s_list.erase(s);
        AssertMsg(check_consistency(),  "Must be consistent when done in remove()");
        
        return Death(birth);
    }
}
        
template<class BID>
void
ZigzagPersistence<BID>::
show_all()
{
    std::cout << "s_list:" << std::endl;
    for (SimplexIndex cur = s_list.begin(); cur != s_list.end(); ++cur)
    {
        std::cout << "  " << cur->order << ":" << std::endl;

        std::cout << "    z_row: ";
        for (typename ZRow::const_iterator zcur = cur->z_row.begin(); zcur != cur->z_row.end(); ++zcur)
            std::cout << (*zcur)->order << " ";
        std::cout << std::endl;

        std::cout << "    c_row: ";
        for (typename CRow::const_iterator ccur = cur->c_row.begin(); ccur != cur->c_row.end(); ++ccur)
            std::cout << (*ccur)->order << " ";
        std::cout << std::endl;
        
        std::cout << "    low: ";
        if (cur->low != z_list.end())
            std::cout << cur->low->order;
        else
            std::cout << "none";
        std::cout << std::endl;
    }
    
    std::cout << "z_list:" << std::endl;
    for (ZIndex cur = z_list.begin(); cur != z_list.end(); ++cur)
    {
        std::cout << "  " << cur->order << ":" << std::endl;
        
        std::cout << "    birth: " << cur->birth << std::endl;

        std::cout << "    z_column: ";
        for (typename ZColumn::const_iterator zcur = cur->z_column.begin(); zcur != cur->z_column.end(); ++zcur)
            std::cout << (*zcur)->order << " ";
        std::cout << std::endl;

        std::cout << "    b_row: ";
        for (typename BRow::const_iterator bcur = cur->b_row.begin(); bcur != cur->b_row.end(); ++bcur)
            std::cout << (*bcur)->order << " ";
        std::cout << std::endl;

        std::cout << "    low: ";
        if (cur->low != b_list.end()) 
            std::cout << cur->low->order;
        else
            std::cout << "none";
        std::cout << std::endl;
    }

    std::cout << "b_list:" << std::endl;
    for (BIndex cur = b_list.begin(); cur != b_list.end(); ++cur)
    {
        std::cout << "  " << cur->order << ":" << std::endl;

        std::cout << "    b_column: ";
        for (typename BColumn::const_iterator bcur = cur->b_column.begin(); bcur != cur->b_column.end(); ++bcur)
            std::cout << (*bcur)->order << " ";
        std::cout << std::endl;

        std::cout << "    c_column: ";
        for (typename CColumn::const_iterator ccur = cur->c_column.begin(); ccur != cur->c_column.end(); ++ccur)
            std::cout << (*ccur)->order << " ";
        std::cout << std::endl;
    }
}

template<class BID>
bool
ZigzagPersistence<BID>::
check_consistency()
{
    for (SimplexIndex cur = s_list.begin(); cur != s_list.end(); ++cur)
    {
        for (typename ZRow::const_iterator zcur = cur->z_row.begin(); zcur != cur->z_row.end(); ++zcur)
            if (std::find((*zcur)->z_column.begin(), (*zcur)->z_column.end(), cur) == (*zcur)->z_column.end())
                return false;
        for (typename CRow::const_iterator ccur = cur->c_row.begin(); ccur != cur->c_row.end(); ++ccur)
            if (std::find((*ccur)->c_column.begin(), (*ccur)->c_column.end(), cur) == (*ccur)->c_column.end())
                return false;
        if (cur->low != z_list.end() && cur->low->z_column.back() != cur) return false;
    }

    for (ZIndex cur = z_list.begin(); cur != z_list.end(); ++cur)
    {
        for (typename ZColumn::const_iterator scur = cur->z_column.begin(); scur != cur->z_column.end(); ++scur)
            if (std::find((*scur)->z_row.begin(), (*scur)->z_row.end(), cur) == (*scur)->z_row.end())
                return false;
        for (typename BRow::const_iterator bcur = cur->b_row.begin(); bcur != cur->b_row.end(); ++bcur)
            if (std::find((*bcur)->b_column.begin(), (*bcur)->b_column.end(), cur) == (*bcur)->b_column.end())
                return false;
        if (cur->low != b_list.end() && cur->low->b_column.back() != cur) return false;
    }

    for (BIndex cur = b_list.begin(); cur != b_list.end(); ++cur)
    {
        for (typename BColumn::const_iterator zcur = cur->b_column.begin(); zcur != cur->b_column.end(); ++zcur)
            if (std::find((*zcur)->b_row.begin(), (*zcur)->b_row.end(), cur) == (*zcur)->b_row.end())
                return false;
        for (typename CColumn::const_iterator scur = cur->c_column.begin(); scur != cur->c_column.end(); ++scur)
            if (std::find((*scur)->c_row.begin(), (*scur)->c_row.end(), cur) == (*scur)->c_row.end())
                return false;
    }

    return true;
}

/* Private */

// Class: Appender
//   
// Functor that appends given element to the given member of whatever parameter it is invoked with
template<class BID>
template<class Member, class Element>
struct ZigzagPersistence<BID>::Appender
{
                Appender(Member mm, Element ee): 
                    m(mm), e(ee)                        {}

    template<class T>
    void        operator()(T& a)                        { ((*a).*m).append(e, cmp); }
                
    Member          m;
    Element         e;
    OrderComparison cmp;
};

// Class: Remover
//   
// Functor that removes given element from the given member of whatever parameter it is invoked with
template<class BID>
template<class Member, class Element>
struct ZigzagPersistence<BID>::Remover
{
                Remover(Member mm, Element ee): 
                    m(mm), e(ee)                        {}

    template<class T>
    void        operator()(T& a)                        { ((*a).*m).remove(e); }
                
    Member  m;
    Element e;
};

// Class: Adder
//   
// Functor that adds the given member of whatever it is invoked with to the given chain
template<class BID>
template<class Member, class Chain>
struct ZigzagPersistence<BID>::Adder
{
                Adder(Member mm, Chain& cc):
                    m(mm), c(cc)                        {}

    template<class T>
    void        operator()(T& a)                        { c.add((*a).*m, cmp); }

    Member          m;
    Chain&          c;
    OrderComparison cmp;
};

        
// Function: add_chains()
//   
// Special case of add_chains where all Indexes are the same, and 
// therefore PrimaryMemberFrom and PrimaryMemberTo are the same
template<class BID>
template<class Index, class IndexFrom, class PrimaryMember, class SecondaryMember>
void
ZigzagPersistence<BID>::
add_chains(Index bg, Index end, IndexFrom j, PrimaryMember pm, SecondaryMember sm)
{
    add_chains(bg, end, j, pm, sm, pm);
}

// Function: add_chains()
//   
// Adds PrimaryMember pm of j to pm of every element in the range [bg,end)
// Fixes SecondaryMembers by adding and removing the corresponding elements.
// For example, if we add a column to a number of other columns, then PrimaryMember is that
// column member, and SecondaryMember is the corresponding row member.
template<class BID>
template<class IndexTo, class IndexFrom, class PrimaryMemberTo, class SecondaryMemberTo, class PrimaryMemberFrom>
void
ZigzagPersistence<BID>::
add_chains(IndexTo bg, IndexTo end, IndexFrom j, PrimaryMemberTo pmt, SecondaryMemberTo smt, PrimaryMemberFrom pmf)
{
    for (IndexTo cur = bg; cur != end; ++cur)
        add_chain(*cur, j, pmt, smt, pmf);
}

// Function: change_basis()
//
// Changes basis by adding PrimaryMember pm of j to pm of every element in range [bg, end).
// In parallel it performs the reverse (complementary) update on the dual members, i.e.
// column and row operations are performed in sync, so that the product of the two matrices doesn't change
template<class BID>
template<class IndexTo, class IndexFrom, class PrimaryMember, class SecondaryMember, class DualPrimaryMember, class DualSecondaryMember>
void
ZigzagPersistence<BID>::
change_basis(IndexTo bg, IndexTo end, IndexFrom j, PrimaryMember pm, SecondaryMember sm, DualPrimaryMember dpm, DualSecondaryMember dsm)
{
    for (IndexTo cur = bg; cur != end; ++cur)
    {
        add_chain(*cur, j,  pm,  sm,  pm);
        add_chain(j, *cur, dpm, dsm, dpm);
    }
}

template<class BID>
template<class Index, class PrimaryMember, class SecondaryMember>
void
ZigzagPersistence<BID>::
add_chain(Index to, Index from, PrimaryMember pm, SecondaryMember sm)
{
    add_chain(to, from, pm, sm, pm);
}

// Function: add_chain()
//
// Adds PrimaryMemberFrom pmf of `from` to PrimaryMemberTo pmt of `to`. 
// Fixes SecondaryMemberTos. See add_chains().
template<class BID>
template<class IndexTo, class IndexFrom, class PrimaryMemberTo, class SecondaryMemberTo, class PrimaryMemberFrom>
void
ZigzagPersistence<BID>::
add_chain(IndexTo to, IndexFrom from, PrimaryMemberTo pmt, SecondaryMemberTo smt, PrimaryMemberFrom pmf)
{
    // Fix secondaries
    std::for_each(make_intersection_iterator(((*from).*pmf).begin(),  ((*from).*pmf).end(),
                                               ((*to).*pmt).begin(),    ((*to).*pmt).end(),
                                             cmp),
                  make_intersection_iterator(((*from).*pmf).end(),    ((*from).*pmf).end(),
                                               ((*to).*pmt).end(),      ((*to).*pmt).end(),
                                             cmp),
                  make_remover(smt, to));
    std::for_each(make_difference_iterator(((*from).*pmf).begin(),    ((*from).*pmf).end(),
                                             ((*to).*pmt).begin(),      ((*to).*pmt).end(),
                                           cmp),
                  make_difference_iterator(((*from).*pmf).end(),      ((*from).*pmf).end(),
                                             ((*to).*pmt).end(),        ((*to).*pmt).end(),
                                           cmp),
                  make_appender(smt, to));
    
    // Add primaries
    ((*to).*pmt).add((*from).*pmf, cmp);
}
