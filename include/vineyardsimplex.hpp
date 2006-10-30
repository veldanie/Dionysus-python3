#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>

using boost::serialization::make_nvp;

/* Implementations */
template<class S>
template<class Archive>
void 
Knee<S>::
serialize(Archive& ar, version_type )
{
	ar & BOOST_SERIALIZATION_NVP(birth);
	ar & BOOST_SERIALIZATION_NVP(death);
	ar & BOOST_SERIALIZATION_NVP(time);
	ar & BOOST_SERIALIZATION_NVP(cycle);
}

template<class S>
template<class Archive>
void 
Vine<S>::
serialize(Archive& ar, version_type )
{ ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VineRepresentation); }


