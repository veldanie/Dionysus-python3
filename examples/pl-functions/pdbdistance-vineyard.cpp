//#include <boost/archive/binary_oarchive.hpp>
#include "utilities/log.h"

#include "pdbdistance.h"
#include <topology/lsvineyard.h>

#include <fstream>
#include <string>
#include <sstream>

std::string frame_filename(const std::string& prefix, int frame, int subframe)
{
	std::ostringstream os;
	os << prefix << frame << "_" << subframe << ".pdb";
	return os.str();
}

int main(int argc, char** argv)
{
#ifdef LOGGING
	rlog::RLogInit(argc, argv);

	stdoutLog.subscribeTo( RLOG_CHANNEL("topology/filtration") );
	stdoutLog.subscribeTo( RLOG_CHANNEL("topology/cycle") );
	stdoutLog.subscribeTo( RLOG_CHANNEL("topology/vineyard") );
	stdoutLog.subscribeTo( RLOG_CHANNEL("topology/lowerstar") );
#endif

	if (argc < 5)
	{
		std::cout << "Usage: pdbdistance FILENAME LASTFRAME LASTSUBFRAME OUTFILENAME [CAs_ONLY]" << std::endl;
		std::cout << "  FILENAME     - prefix of the filenames of the PDB frames" << std::endl;
		std::cout << "  LASTFRAME    - the last frame number" << std::endl;
		std::cout << "  LASTSUBFRAME - the last subframe number" << std::endl;
		std::cout << "  OUTFILENAME  - filename prefix for the resulting vineyards" << std::endl;
		std::cout << "  CAs_ONLY     - only use alpha carbons [1 = true, 0 = false, default: 1]" << std::endl;
		std::cout << std::endl;
		std::cout << "Computes a vineyard of the pairwise distance function for a sequence of PDB frames." << std::endl;
		std::cout << "Frames are in files FILENAME#1_#2.pdb, where #1 goes from 0 to LASTFRAME, " << std::endl;
		std::cout << "and #2 goes from 0 to LASTSUBFRAME." << std::endl;
		exit(0);
	}
	std::string infilename = argv[1];
	int lastframe; std::istringstream(argv[2]) >> lastframe;
	int lastsubframe; std::istringstream(argv[3]) >> lastsubframe;
	std::string outfilename = argv[4];
	bool cas_only = true;
	if (argc > 5)
		std::istringstream(argv[5]) >> cas_only;

	// Compute initial filtration
	int f = 0; int sf = 0;
	std::ifstream in(frame_filename(infilename, f, sf++).c_str());
    PDBDistanceGrid ginit(in, cas_only);
	in.close();

    typedef                     LSVineyard<Grid2D::CoordinateIndex, Grid2D>             Grid2DVineyard;
    
    Grid2DVineyard::LSFiltration        simplices;    
    ginit.complex_generator(make_push_back_functor(simplices));
    Grid2DVineyard::VertexComparison    vcmp(ginit);
    Grid2DVineyard::SimplexComparison   scmp(vcmp);
    simplices.sort(scmp);
    std::cout << "Complex generated, size: " << simplices.size() << std::endl;

    Grid2DVineyard              v(ginit.begin(), ginit.end(), simplices, ginit);
	std::cout << "Filtration generated, size: " << v.filtration().size() << std::endl;
	std::cout << "Pairing computed" << std::endl;

	// Process frames computing the vineyard
	while (f <= lastframe)
	{
		std::string fn = frame_filename(infilename, f, sf++);
		std::cout << "Processing " << fn << std::endl;
		in.open(fn.c_str());
		v.compute_vineyard(PDBDistanceGrid(in, cas_only));
		in.close();
		if (sf == lastsubframe) { sf = 0; ++f; }
	}
	std::cout << "Vineyard computed" << std::endl;

	v.vineyard().save_edges(outfilename);

#if 0
	std::ofstream ofs(outfilename.c_str(), std::ios::binary);
	boost::archive::binary_oarchive oa(ofs);
	oa << make_nvp("Filtration", pgf);
	ofs.close();
#endif
}
