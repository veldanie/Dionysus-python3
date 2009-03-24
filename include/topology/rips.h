#ifndef __RIPS_H__
#define __RIPS_H__

#include <vector>
#include <string>
#include "simplex.h"
#include <boost/iterator/counting_iterator.hpp>


/**
 * Rips class
 *
 * Class providing basic operations to work with Rips complexes. It implements Bron-Kerbosch algorithm, 
 * and provides simple wrappers for various functions.
 *
 * Distances_ is expected to define types IndexType and DistanceType as well as 
 *               provide operator()(...) which given two IndexTypes should return 
 *               the distance between them. There should be methods begin() and end() 
 *               for iterating over IndexTypes as well as a method size().
 */
template<class Distances_, class Simplex_ = Simplex<typename Distances_::IndexType> >
class Rips
{
    public:
        typedef             Distances_                                      Distances; 
        typedef             typename Distances::IndexType                   IndexType;
        typedef             typename Distances::DistanceType                DistanceType;

        typedef             Simplex_                                        Simplex;
        typedef             typename Simplex::Vertex                        Vertex;             // should be the same as IndexType
        typedef             typename Simplex::VertexContainer               VertexContainer;

        class               Evaluator;
        class               Comparison;
        class               ComparePair;       

    public:
                            Rips(const Distances& distances):
                                distances_(distances)                       {}

        // Calls functor f on each simplex in the k-skeleton of the Rips complex
        template<class Functor, class Iterator>
        void                generate(Dimension k, DistanceType max, const Functor& f, 
                                     Iterator candidates_begin, Iterator candidates_end) const;
        
        // Calls functor f on all the simplices of the Rips complex that contain the given vertex v
        template<class Functor, class Iterator>
        void                vertex_cofaces(IndexType v, Dimension k, DistanceType max, const Functor& f, 
                                           Iterator candidates_begin, Iterator candidates_end) const;

        // Calls functor f on all the simplices of the Rips complex that contain the given edge [u,v]
        template<class Functor, class Iterator>
        void                edge_cofaces(IndexType u, IndexType v, Dimension k, DistanceType max, const Functor& f, 
                                         Iterator candidates_begin, Iterator candidates_end) const;
        
        // Calls functor f on all the simplices of the Rips complex that contain the given Simplex s
        // (unlike the previous methods it does not call the functor on the Simplex s itself)
        template<class Functor, class Iterator>
        void                cofaces(const Simplex& s, Dimension k, DistanceType max, const Functor& f,
                                    Iterator candidates_begin, Iterator candidates_end) const;

        
        /* No Iterator argument means Iterator = IndexType and the range is [distances().begin(), distances().end()) */
        template<class Functor>
        void                generate(Dimension k, DistanceType max, const Functor& f) const
        { generate(k, max, f, boost::make_counting_iterator(distances().begin()), boost::make_counting_iterator(distances().end())); }
        
        template<class Functor>
        void                vertex_cofaces(IndexType v, Dimension k, DistanceType max, const Functor& f) const
        { vertex_cofaces(v, k, max, f, boost::make_counting_iterator(distances().begin()), boost::make_counting_iterator(distances().end())); }

        template<class Functor>
        void                edge_cofaces(IndexType u, IndexType v, Dimension k, DistanceType max, const Functor& f) const
        { edge_cofaces(u, v, k, max, f, boost::make_counting_iterator(distances().begin()), boost::make_counting_iterator(distances().end())); }

        template<class Functor>
        void                cofaces(const Simplex& s, Dimension k, DistanceType max, const Functor& f) const
        { cofaces(s, k, max, f, boost::make_counting_iterator(distances().begin()), boost::make_counting_iterator(distances().end())); }

        
        const Distances&    distances() const                               { return distances_; }
        DistanceType        max_distance() const;
        
        DistanceType        distance(const Simplex& s1, const Simplex& s2) const;


    private:
        class               WithinDistance;

        template<class Functor, class NeighborTest>
        void                bron_kerbosch(VertexContainer&                          current, 
                                          const VertexContainer&                    candidates, 
                                          typename VertexContainer::const_iterator  excluded,
                                          Dimension                                 max_dim,
                                          const NeighborTest&                       neighbor,
                                          const Functor&                            functor,
                                          bool                                      check_initial = true) const;
        
    private:
        const Distances&    distances_;
};
        

template<class Distances_, class Simplex_>
class Rips<Distances_, Simplex_>::WithinDistance: public std::binary_function<Vertex, Vertex, bool>
{
    public:
                            WithinDistance(const Distances_&    distances, 
                                           DistanceType         max):
                                distances_(distances), max_(max)                        {}

        bool                operator()(Vertex u, Vertex v) const                        { return distances_(u, v) <= max_; }

    private:
        const Distances&    distances_;  
        DistanceType        max_;
};

template<class Distances_, class Simplex_>
class Rips<Distances_, Simplex_>::Evaluator: public std::unary_function<const Simplex&, DistanceType>
{
    public:
        typedef             Simplex_                                        Simplex;

                            Evaluator(const Distances& distances): 
                                distances_(distances)                       {}

        DistanceType        operator()(const Simplex& s) const;

    private:
        const Distances&    distances_;
};

template<class Distances_, class Simplex_>
class Rips<Distances_, Simplex_>::Comparison: public std::binary_function<const Simplex&, const Simplex&, bool>
{
    public:
        typedef             Simplex_                                        Simplex;

                            Comparison(const Distances& distances):
                                eval_(distances)                            {}

        bool                operator()(const Simplex& s1, const Simplex& s2) const    
        { 
            DistanceType e1 = eval_(s1), 
                         e2 = eval_(s2);
            if (e1 == e2)
                return s1.dimension() < s2.dimension();

            return e1 < e2;
        }

    private:
        Evaluator           eval_;
};

template<class Distances_, class Simplex_>
struct Rips<Distances_, Simplex_>::ComparePair: 
    public std::binary_function<const std::pair<IndexType, IndexType>&,
                                const std::pair<IndexType, IndexType>&,
                                bool>
{
                            ComparePair(const Distances& distances): 
                                distances_(distances)                       {}

        bool                operator()(const std::pair<IndexType, IndexType>& a,
                                       const std::pair<IndexType, IndexType>& b)            {  return   distances_(a.first, a.second) <
                                                                                                        distances_(b.first, b.second);  }

        const Distances&    distances_;
};


/**
 * Class: ExplicitDistances 
 * Stores the pairwise distances of Distances_ instance passed at construction. 
 * It's a protypical Distances template argument for the Rips complex.
 */
template<class Distances_>
class ExplicitDistances
{
    public:
        typedef             Distances_                                      Distances;
        typedef             size_t                                          IndexType;
        typedef             typename Distances::DistanceType                DistanceType;

                            ExplicitDistances(const Distances& distances);

        DistanceType        operator()(IndexType a, IndexType b) const;

        size_t              size() const                                    { return size_; }
        IndexType           begin() const                                   { return 0; }
        IndexType           end() const                                     { return size(); }

    private:
        std::vector<DistanceType>                   distances_;
        size_t                                      size_;
};


/**
 * Class: PairwiseDistances
 * Given a Container_ of points and a Distance_, it computes distances between elements 
 * in the container (given as instances of Index_ defaulted to unsigned) using the Distance_ functor.
 *
 * Container_ is assumed to be an std::vector. That simplifies a number of things.
 */
template<class Container_, class Distance_, typename Index_ = unsigned>
class PairwiseDistances
{
    public:
        typedef             Container_                                      Container;
        typedef             Distance_                                       Distance;
        typedef             Index_                                          IndexType;
        typedef             typename Distance::result_type                  DistanceType;


                            PairwiseDistances(const Container& container, 
                                              const Distance& distance = Distance()):
                                container_(container), distance_(distance)  {}

        DistanceType        operator()(IndexType a, IndexType b) const      { return distance_(container_[a], container_[b]); }

        size_t              size() const                                    { return container_.size(); }
        IndexType           begin() const                                   { return 0; }
        IndexType           end() const                                     { return size(); }

    private:
        const Container&    container_;
        Distance            distance_;
};

#include "rips.hpp"

#endif // __RIPS_H__
