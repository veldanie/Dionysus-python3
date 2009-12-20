/**
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2009
 */

#ifndef __LSVINEYARD_H__
#define __LSVINEYARD_H__

#include <iostream>

#include "topology/simplex.h"
#include "topology/dynamic-persistence.h"
#include "topology/lowerstarfiltration.h"
#include "topology/vineyard.h"

#include <utilities/indirect.h>

#include <CGAL/Kinetic/Inexact_simulation_traits.h>
#include <CGAL/Kinetic/Sort.h>
#include <CGAL/Kinetic/Sort_visitor_base.h>

#include <list>


template<class Vertex_, class VertexEvaluator_, class Simplex_ = Simplex<Vertex_>, class Filtration_ = Filtration<Simplex_> >
class LSVineyard
{
    public:
        typedef                     LSVineyard                                          Self;

        typedef                     Vertex_                                             Vertex;
        typedef                     VertexEvaluator_                                    VertexEvaluator;
        typedef                     typename VertexEvaluator::result_type               VertexValue;
        
        typedef                     Simplex_                                            Simplex;
        typedef                     Filtration_                                         LSFiltration;
        typedef                     typename LSFiltration::Index                        LSFIndex;

        class                       KineticVertexType;
        class                       KineticVertexComparison;
        typedef                     std::list<KineticVertexType>                        VertexContainer;
        typedef                     typename VertexContainer::iterator                  VertexIndex;

        struct                      AttachmentData: public VineData                     
        {   
            void                    set_attachment(VertexIndex v)                       { attachment = v; }
            VertexIndex             attachment; 
        };
        typedef                     DynamicPersistenceTrails<AttachmentData>            Persistence;
        typedef                     typename Persistence::OrderIndex                    Index;
        typedef                     typename Persistence::iterator                      iterator;

        typedef                     typename Persistence::template SimplexMap<LSFiltration>      
                                                                                        PFMap;

        class                       Evaluator;
        class                       StaticEvaluator;
        class                       KineticEvaluator;
        class                       DimensionFromIterator;

        typedef                     ThroughEvaluatorComparison<VertexEvaluator>         VertexComparison;
        typedef                     MaxVertexComparison<Simplex, VertexComparison>      SimplexComparison;

        class                       TranspositionVisitor;
        friend class                TranspositionVisitor;
        
        typedef                     Vineyard<Index, iterator, Evaluator>                Vnrd;

        class                       SortVisitor;
        typedef                     CGAL::Kinetic::Inexact_simulation_traits            Traits;
        typedef                     CGAL::Kinetic::Sort<Traits, SortVisitor>            Sort;
        typedef                     Traits::Simulator                                   Simulator;
        typedef                     Traits::Active_points_1_table                       ActivePointsTable;
        typedef                     ActivePointsTable::Key                              Key;
        typedef                     std::map<Key, VertexIndex>                          KeyVertexMap;

    public:
        template<class VertexIterator>
                                    LSVineyard(VertexIterator begin, VertexIterator end, 
                                               LSFiltration& filtration,
                                               const VertexEvaluator& veval = VertexEvaluator());
                                    ~LSVineyard();

        void                        compute_vineyard(const VertexEvaluator& veval, bool explicit_events = false);
        bool                        transpose_vertices(VertexIndex vi);
        
        const LSFiltration&         filtration() const                                  { return filtration_; }
        const Vnrd&                 vineyard() const                                    { return vineyard_; }
        const Persistence&          persistence() const                                 { return persistence_; }
        const VertexComparison&     vertex_comparison() const                           { return vcmp_; }
        const VertexEvaluator&      vertex_evaluator() const                            { return veval_; }
        const SimplexComparison&    simplex_comparison() const                          { return scmp_; }

        VertexValue                 vertex_value(const Vertex& v) const                 { return veval_(v); }
        VertexValue                 simplex_value(const Simplex& s) const               { return vertex_value(*std::max_element(s.vertices().begin(), s.vertices().end(), vcmp_)); } 
        const Simplex&              pfmap(iterator i) const                             { return pfmap_[i]; }
        const Simplex&              pfmap(Index i) const                                { return pfmap_[i]; }

        Index                       index(iterator i) const                             { return persistence_.index(i); }

    public:
        // For Kinetic Sort
        void                        swap(Key a, Key b);
        
    private:
        void                        change_evaluator(Evaluator* eval);
        void                        set_attachment(iterator i, VertexIndex vi)          { persistence_.modifier()(i, boost::bind(&AttachmentData::set_attachment, bl::_1, vi)); }
        void                        transpose_filtration(iterator i)                    { filtration_.transpose(filtration_.begin() + (i - persistence_.begin())); }

    private:
        VertexContainer             vertices_;
        
        VertexEvaluator             veval_;
        VertexComparison            vcmp_;
        SimplexComparison           scmp_;

        LSFiltration&               filtration_;
        Persistence                 persistence_;
        PFMap                       pfmap_;
        
        Vnrd                        vineyard_;
        Evaluator*                  evaluator_;
        unsigned                    time_count_;

        KeyVertexMap                kinetic_map_;
                
#if 0
    private:
        // Serialization
        friend class boost::serialization::access;
        
        LSVineyard()                                                                    {}

        template<class Archive> 
        void serialize(Archive& ar, version_type )
        { 
            ar & BOOST_SERIALIZATION_NVP(grid_stack_); 
            ar & BOOST_SERIALIZATION_NVP(vertices_); 
            ar & BOOST_SERIALIZATION_NVP(filtration_); 
        };
#endif
};

//BOOST_CLASS_EXPORT(LSVineyard)
        
template<class V, class VE, class S, class C>
class LSVineyard<V,VE,S,C>::KineticVertexType
{
    public:
                                KineticVertexType(const Vertex& v):
                                    vertex_(v)                                              {}

        Key                     kinetic_key() const                                         { return key_; }
        void                    set_kinetic_key(Key k)                                      { key_ = k; }

        Vertex                  vertex() const                                              { return vertex_; }
        void                    set_vertex(Vertex v)                                        { vertex_ = v; }

        iterator                simplex_index() const                                       { return simplex_index_; }
        void                    set_simplex_index(iterator i)                               { simplex_index_ = i; }
        
    private:
        Key                     key_;
        Vertex                  vertex_;
        iterator                simplex_index_;
};

template<class V, class VE, class S, class C>
std::ostream& 
operator<<(std::ostream& out, const typename LSVineyard<V,VE,S,C>::VertexIndex& vi)    
{ return out << vi->vertex(); }
        
template<class V, class VE, class S, class C>
class LSVineyard<V,VE,S,C>::KineticVertexComparison: public std::binary_function<const KineticVertexType&, const KineticVertexType&, bool>
{
    public:
                                KineticVertexComparison(const VertexComparison& vcmp):
                                    vcmp_(vcmp)                                             {}

        bool                    operator()(const KineticVertexType& v1, const KineticVertexType& v2) const
        { return vcmp_(v1.vertex(), v2.vertex()); }

    private:
        VertexComparison            vcmp_;
};

template<class V, class VE, class S, class C>
class LSVineyard<V,VE,S,C>::TranspositionVisitor: public Persistence::TranspositionVisitor
{
    public:
        typedef                 typename Persistence::TranspositionVisitor                  Parent;
        typedef                 typename LSVineyard<V,VE,S,C>::iterator                     iterator;
        typedef                 typename LSVineyard<V,VE,S,C>::Index                        Index;

                                TranspositionVisitor(LSVineyard& v): lsvineyard_(v)         {}

        void                    transpose(iterator i)                                       { lsvineyard_.transpose_filtration(i); }
        void                    switched(iterator i, SwitchType type)                       { lsvineyard_.vineyard_.switched(index(i), index(boost::next(i))); }

    private:
        Index                   index(iterator i)                                           { return lsvineyard_.index(i); }

        LSVineyard&             lsvineyard_;
};

template<class V, class VE, class S, class C>
class LSVineyard<V,VE,S,C>::Evaluator: public std::unary_function<Index, RealType>
{
    public:
        virtual RealType        time() const                                                =0;
        virtual RealType        operator()(Index i) const                                   =0;
        virtual Dimension       dimension(Index i) const                                    =0;
        virtual RealType        operator()(iterator i) const                                { return operator()(&*i); }
        virtual Dimension       dimension(iterator i) const                                 { return dimension(&*i); }
};

template<class V, class VE, class S, class C>
class LSVineyard<V,VE,S,C>::SortVisitor: public CGAL::Kinetic::Sort_visitor_base
{
    public:
                                SortVisitor(LSVineyard& v): 
                                    vineyard_(v)                                            {}

        template<class Vertex_handle>
        void                    pre_swap(Vertex_handle a, Vertex_handle b) const            { vineyard_.swap(*a,*b); }

    private:
        LSVineyard&             vineyard_;
};

template<class V, class VE, class S, class C>
class LSVineyard<V,VE,S,C>::DimensionFromIterator: std::unary_function<iterator, Dimension>
{
    public:
                                DimensionFromIterator(const PFMap& pfmap): pfmap_(pfmap)    {}

        Dimension               operator()(iterator i) const                                { return pfmap_[i].dimension(); }                                

    private:
        const PFMap&            pfmap_;
};

#include "lsvineyard.hpp"

#endif // __LSVINEYARD_H__
