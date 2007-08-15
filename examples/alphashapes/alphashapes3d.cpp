#include <sys.h>
#include <debug.h>

#include "alphashapes3d.h"
#include <filtration.h>
#include <iostream>
#include <fstream>


typedef Filtration<AlphaSimplex3D>				AlphaFiltration;

int main(int argc, char** argv) 
{
	// Read in the point set and compute its Delaunay triangulation
	std::istream& in = std::cin;
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
	std::cout << "Simplices: " << alpha_ordering.size() << std::endl;

	// Create the alpha-shape filtration
	AlphaFiltration af;
	for (AlphaSimplex3DVector::const_iterator cur = alpha_ordering.begin(); cur != alpha_ordering.end(); ++cur)
		af.append(*cur);
	af.fill_simplex_index_map();
	af.pair_simplices(af.begin(), af.end());
	std::cout << "Simplices paired" << std::endl;
}

