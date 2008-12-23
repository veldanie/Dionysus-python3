#include "utilities/containers.h"

template<class C, class I, class CT>
template<class Comparison>
Filtration<C,I,CT>::
Filtration(ComplexIndex bg, ComplexIndex end, const Comparison& cmp):
    order_(RecursiveIterator<ComplexIndex>(bg), 
           RecursiveIterator<ComplexIndex>(end)),
    reverse_order_(order_.size()),
    complex_order_map_(bg, reverse_order_.begin()),
    simplex_index_map_(bg, end)
{
    std::sort(order_.begin(), order_.end(), IndirectComparison<ComplexIndex, Comparison>(cmp));
    for (Index obg = order_.begin(), cur = obg; cur != order_.end(); ++cur)
        reverse_order_[*cur - bg] = cur - obg;
}

template<class C, class I, class CT>
template<class Cycle, class Map>
void
Filtration<C,I,CT>::
boundary(const Index& i, Cycle& bdry, const Map& map) const
{
    AssertMsg(bdry.empty(), "We are initializing the boundary from scratch");
    ContainerTraits<Cycle>::reserve(bdry, (*i)->boundary_end() - (*i)->boundary_begin());
    typename Map::template rebind_from<IntermediateIndex>::other    order_bdry_map(0, map.to());

    for (typename Simplex::BoundaryIterator cur = (*i)->boundary_begin(); cur != (*i)->boundary_end(); ++cur)
    {
        //std::cout << *cur << std::endl;
        //std::cout << simplex_index_map_[*cur] - complex_order_map_.from() << std::endl;
        bdry.push_back(order_bdry_map[*complex_order_map_[simplex_index_map_[*cur]]]);
        //std::cout << bdry.back() - order_bdry_map.to() << std::endl;
    }
}

template<class C, class I, class CT>
std::ostream&
Filtration<C,I,CT>::
operator<<(std::ostream& out) const
{
    for (Index i = begin(); i != end(); ++i)
        out << **i << std::endl;
    return out;
}
