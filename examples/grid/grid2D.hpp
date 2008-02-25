#include <iostream>
#include <limits>

/* Implementations */

Grid2D::
Grid2D(CoordinateIndex xx, CoordinateIndex yy):
	x(xx), y(yy), data(x*y)
{}

void					
Grid2D::
change_dimensions(CoordinateIndex xx, CoordinateIndex yy)
{
	x = xx; y = yy;
	data.resize(x*y);
}

Grid2D::CoordinateIndex
Grid2D::
seq(CoordinateIndex i, CoordinateIndex j) const
{ 
	// Do not forget to check if less than 0, if Index is made signed --- dangerous
	if (i >= x || j >= y)
		return INVALID_INDEX;

	return i*x + j; 
}

std::ostream&			
Grid2D::
operator<<(std::ostream& out) const
{
	for (Grid2D::CoordinateIndex i = 0; i < xsize(); ++i)
	{
		for (Grid2D::CoordinateIndex j = 0; j < ysize(); ++j)
			std::cout << operator()(i, j) << ' ';
		std::cout << std::endl;
	}
	return out;	
}	

#if 0
using boost::serialization::make_nvp;

template<class Archive>
void 
Grid2D::
save(Archive& ar, version_type ) const
{
	ar << BOOST_SERIALIZATION_NVP(x);
	ar << BOOST_SERIALIZATION_NVP(y);
	ar << make_nvp("data", data);
}

template<class Archive>	
void 
Grid2D::
load(Archive& ar, version_type )
{
	ar >> make_nvp("x", x);
	ar >> make_nvp("y", y);
	ar >> make_nvp("data", data);
}
#endif
