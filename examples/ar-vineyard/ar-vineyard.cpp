#include <utilities/log.h>

#include "ar-vineyard.h"

#include <iostream>
#include <fstream>


int main(int argc, char** argv) 
{
#ifdef LOGGING
	rlog::RLogInit(argc, argv);

	stderrLog.subscribeTo( RLOG_CHANNEL("error") );
    //stdoutLog.subscribeTo( RLOG_CHANNEL("ar/vineyard") );
    //stdoutLog.subscribeTo( RLOG_CHANNEL("ar/function-kernel/value") );
    //stdoutLog.subscribeTo( RLOG_CHANNEL("geometry/simulator") );

	//stdoutLog.subscribeTo( RLOG_CHANNEL("topology/filtration") );
	//stdoutLog.subscribeTo( RLOG_CHANNEL("topology/cycle") );
	//stdoutLog.subscribeTo( RLOG_CHANNEL("topology/vineyard") );
	//stdoutLog.subscribeTo( RLOG_CHANNEL("topology/filtration/transpositions") );
	//stdoutLog.subscribeTo( RLOG_CHANNEL("topology/lowerstar") );
#endif

	// Read command-line arguments
	if (argc < 6)
	{
		std::cout << "Usage: ar-vineyard POINTS X Y Z OUTFILENAME" << std::endl;
		std::cout << "  POINTS       - filename containing points" << std::endl;
		std::cout << "  X,Y,Z        - center-point z at which to compute the vineyard" << std::endl;
		std::cout << "  OUTFILENAME  - filename for the resulting vineyard" << std::endl;
		std::cout << std::endl;
		std::cout << "Computes an (alpha,r)-vineyard of the given pointset around the given point." << std::endl;
		exit(0);
	}
	std::string infilename = argv[1];
	double zx,zy,zz; std::istringstream(argv[2]) >> zx;
	std::istringstream(argv[3]) >> zy; std::istringstream(argv[4]) >> zz;
	std::string outfilename = argv[5];
	
	
	// Read in the point set and compute its Delaunay triangulation
	std::ifstream in(infilename.c_str());
	double x,y,z;
	ARVineyard::PointList points;
	while(in)
	{
		in >> x >> y >> z;
		points.push_back(Point(x,y,z));
	}
   

	// Setup vineyard and compute initial pairing
	ARVineyard arv(points, Point(zx,zy,zz));
	arv.compute_pairing();

	// Compute vineyard
	arv.compute_vineyard();
	std::cout << "Vineyard computed" << std::endl;
	arv.vineyard()->save_edges(outfilename);
}

