#include <utilities/log.h>

#include "alphashapes3d.h"
#include <topology/filtration.h>
#include <iostream>
#include <fstream>


struct SimplexWithDistance: public AlphaSimplex3D
{
	SimplexWithDistance(const SimplexWithDistance& s): AlphaSimplex3D(s)				{ distance = s.distance; }
	SimplexWithDistance(const AlphaSimplex3D& s): AlphaSimplex3D(s)						{ }
	SimplexWithDistance(const AlphaSimplex3D::Parent& s): AlphaSimplex3D(s)				{ }
	SimplexWithDistance(const AlphaSimplex3D& s, const Point& p): AlphaSimplex3D(s)		{ set_distance(p); }

	void set_distance(const Point& p)
	{
		AlphaSimplex3D::VertexContainer::const_iterator cur = vertices().begin();
		K::Vector_3 v = ((*cur)->point() - p);
		RealValue min_distance = v*v;
	
		while (cur != vertices().end())
		{
			v = ((*cur)->point() - p);
			min_distance = std::min(v*v, min_distance);
			++cur;
		}
	
		distance = min_distance;
	}

	RealValue distance;
};
typedef std::vector<SimplexWithDistance> 		SimplexVector;
typedef Filtration<SimplexWithDistance> 		RadiusFiltration;

struct RadiusOrder
{
	bool operator()(const SimplexWithDistance& first, const SimplexWithDistance& second) const
	{
		if (first.distance == second.distance)
			return (first.dimension() < second.dimension());
		else
			return (first.distance > second.distance); 
	}
};

int main(int argc, char** argv) 
{
#ifdef LOGGING
	rlog::RLogInit(argc, argv);

	stdoutLog.subscribeTo( RLOG_CHANNEL("error") );
	stdoutLog.subscribeTo( RLOG_CHANNEL("topology/filtration") );
	//stdoutLog.subscribeTo( RLOG_CHANNEL("topology/cycle") );
#endif

	std::istream& in = std::cin;
	std::ofstream point_vineyard("point_vineyard.vrt");

	double x,y,z;
	Delaunay Dt;
	while(in)
	{
		in >> x >> y >> z;
		Point p(x,y,z);
		Dt.insert(p);
	}
	std::cout << "Delaunay triangulation computed" << std::endl;
 
	AlphaSimplex3DVector alpha_ordering;
	fill_alpha_order(Dt, alpha_ordering);
    
	// Compute r-filtration for each distinct alpha
	Point p(0,0,0);
	RadiusOrder ro;
	SimplexVector radius_ordering;
	for (AlphaSimplex3DVector::const_iterator cur = alpha_ordering.begin(); cur != alpha_ordering.end(); ++cur)
	{
		radius_ordering.push_back(*cur);
		radius_ordering.back().set_distance(p);
		if (boost::next(cur)->alpha() == cur->alpha())
			continue;
		
		double current_alpha = CGAL::to_double(cur->value());
		std::cout << "Current alpha: " << current_alpha << std::endl;
		std::sort(radius_ordering.begin(), radius_ordering.end(), ro);
		std::cout << "Radius ordering size: " << radius_ordering.size() << std::endl;

		RadiusFiltration rf;
		for (SimplexVector::const_iterator cur = radius_ordering.begin(); cur != radius_ordering.end(); ++cur)
			rf.append(*cur);
		rf.fill_simplex_index_map();
		std::cout << "Simplex index map filled" << std::endl;
		rf.pair_simplices(rf.begin(), rf.end());
		std::cout << "Pairing computed" << std::endl;
	
		for (RadiusFiltration::const_Index cur = rf.begin(); cur != rf.end(); ++cur)
		{
			if (!cur->sign()) continue;
	
			RealValue d1 = cur->distance;
			//if (cur == cur->pair())
			//	std::cout << "Unpaired " << cur->dimension() << ' ' << CGAL::to_double(d1) << std::endl;
			
			RealValue d2 = cur->pair()->distance;
			if (d1 == d2)	continue;
			point_vineyard << CGAL::to_double(d1) << ' ' << CGAL::to_double(d2) << ' ' << current_alpha << std::endl;
		}
	}

	point_vineyard.close();
}
