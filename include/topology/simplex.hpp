#include <boost/serialization/base_object.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/nvp.hpp>


/* Implementations */

/* SimplexWithVertices */
template<class V>
typename SimplexWithVertices<V>::Cycle	
SimplexWithVertices<V>::
boundary() const
{
	Cycle bdry;
	if (dimension() == 0)	return bdry;

	for (typename VertexContainer::const_iterator cur = vertices_.begin(); cur != vertices_.end(); ++cur)
	{
		bdry.push_back(Self(dimension() - 1));
		Self& s = bdry.back();
		std::remove_copy(vertices_.begin(), vertices_.end(), s.vertices_.begin(), *cur);
	}

	return bdry;
}

template<class V>
bool
SimplexWithVertices<V>::
contains(const Vertex& v) const
{ 
	typename VertexContainer::const_iterator location = std::lower_bound(vertices_.begin(), vertices_.end(), v); 
	return ((location == vertices_.end()) || (*location == v)); 
}
		
template<class V>
void
SimplexWithVertices<V>::
add(const Vertex& v)
{ vertices_.push_back(v); std::sort(vertices_.begin(), vertices_.end()); }
	
template<class V>
void
SimplexWithVertices<V>::
join(const Self& other)
{ 
    vertices_.insert(vertices_.end(), other.vertices_.begin(), other.vertices_.end());
    std::sort(vertices_.begin(), vertices_.end()); 
}

template<class V>
std::ostream&			
SimplexWithVertices<V>::
operator<<(std::ostream& out) const
{
	for (typename VertexContainer::const_iterator cur = vertices_.begin(); cur != vertices_.end(); ++cur)
		out << *cur << ' ';
	
	return out;
}
		
template<class V>
template<class Archive>
void 
SimplexWithVertices<V>::
serialize(Archive& ar, version_type )									
{ ar & BOOST_SERIALIZATION_NVP(vertices_); }

template<class V>
std::ostream& operator<<(std::ostream& out, const SimplexWithVertices<V>& s)		
{ return s.operator<<(out); }


/* SimplexWithValue */
template<class V>
std::ostream&
SimplexWithValue<V>::
operator<<(std::ostream& out) const
{
	Parent::operator<<(out);
	out << "(val = " << val << ")";
	return out;
}

template<class V>
const typename SimplexWithValue<V>::Self&	
SimplexWithValue<V>::
operator=(const Self& s)									
{ 
	Parent::operator=(s); 
	val = s.val; 
	return *this; 
}
		
template<class V>
template<class Archive>
void 
SimplexWithValue<V>::
serialize(Archive& ar, version_type )								
{ 
	ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Parent);
	ar & BOOST_SERIALIZATION_NVP(val);
}

template<typename V>
template<class Archive>
void 
SimplexWithAttachment<V>::
serialize(Archive& ar, version_type )
{			
	ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Parent);
	ar & BOOST_SERIALIZATION_NVP(attachment);
}


template<class V>
std::ostream& operator<<(std::ostream& out, const SimplexWithValue<V>& s)		
{ return s.operator<<(out); }
