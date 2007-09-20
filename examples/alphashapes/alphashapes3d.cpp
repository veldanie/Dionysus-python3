#include <utilities/log.h>

#include "alphashapes3d.h"
#include <topology/filtration.h>
#include <iostream>
#include <fstream>


typedef Filtration<AlphaSimplex3D>				AlphaFiltration;

int main(int argc, char** argv) 
{
#ifdef LOGGING
	rlog::RLogInit(argc, argv);

	stdoutLog.subscribeTo( RLOG_CHANNEL("error") );
	//stdoutLog.subscribeTo( RLOG_CHANNEL("topology/filtration") );
	//stdoutLog.subscribeTo( RLOG_CHANNEL("topology/cycle") );
#endif

	SetFrequency(GetCounter("filtration/pair"), 10000);
	SetTrigger(GetCounter("filtration/pair"), GetCounter(""));

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
	std::cout << "Filled simplex-index map" << std::endl;
	af.pair_simplices(af.begin(), af.end(), false);
	std::cout << "Simplices paired" << std::endl;

	for (AlphaFiltration::Index i = af.begin(); i != af.end(); ++i)
		if (i->is_paired())
		{
			if (i->sign())
				std::cout << i->dimension() << " " << i->value() << " " << i->pair()->value() << std::endl;
		} //else std::cout << i->value() << std::endl;

}

