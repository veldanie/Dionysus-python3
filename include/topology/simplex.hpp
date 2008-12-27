#include <boost/serialization/base_object.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/nvp.hpp>

#include <boost/iterator/filter_iterator.hpp>
#include <functional>

/* Implementations */

/* Simplex */
template<class V, class T>
typename Simplex<V,T>::Boundary 
Simplex<V,T>::
boundary() const
{
    typedef         std::not_equal_to<Vertex>                           NotEqualVertex;

    Boundary bdry;
    if (dimension() == 0) return bdry;
    
    for (typename VertexContainer::const_iterator cur = vertices().begin(); cur != vertices().end(); ++cur)
        bdry.push_back(Self(boost::make_filter_iterator(std::bind2nd(NotEqualVertex(), *cur), vertices().begin(), vertices().end()),
                            boost::make_filter_iterator(std::bind2nd(NotEqualVertex(), *cur), vertices().end(),   vertices().end())));
    
    return bdry;
}

template<class V, class T>
bool
Simplex<V,T>::
contains(const Vertex& v) const
{ 
    // TODO: would std::find() be faster? (since most simplices we deal with are low dimensional)
    typename VertexContainer::const_iterator location = std::lower_bound(vertices().begin(), vertices().end(), v); 
    return ((location != vertices().end()) && (*location == v)); 
}
 
template<class V, class T>
bool
Simplex<V,T>::
contains(const Self& s) const
{ 
    return std::includes(  vertices().begin(),   vertices().end(),
                         s.vertices().begin(), s.vertices().end());
}

template<class V, class T>
void
Simplex<V,T>::
add(const Vertex& v)
{
    // TODO: would find() or lower_bound() followed by insert be faster?
    vertices().push_back(v); std::sort(vertices().begin(), vertices().end()); 
}
    
template<class V, class T>
template<class Iterator>
void
Simplex<V,T>::
join(Iterator bg, Iterator end)
{ 
    vertices().insert(vertices().end(), bg, end);
    std::sort(vertices().begin(), vertices().end()); 
}

template<class V, class T>
std::ostream&           
Simplex<V,T>::
operator<<(std::ostream& out) const
{
    typename VertexContainer::const_iterator cur = vertices().begin();
    out << *cur;
    for (++cur; cur != vertices().end(); ++cur)
    {
        out << ", " << *cur;
    }
    out << " [" << data() << "] ";

    return out;
}
        
template<class V, class T>
template<class Archive>
void 
Simplex<V,T>::
serialize(Archive& ar, version_type )                                   
{ 
    ar & make_nvp("vertices", vertices()); 
    ar & make_nvp("data", data()); 
}

template<class V, class T>
std::ostream& operator<<(std::ostream& out, const Simplex<V,T>& s)      
{ return s.operator<<(out); }
