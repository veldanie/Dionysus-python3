#include <utilities/log.h>

#include "alphashapes3d.h"
#include <topology/filtration.h>
#include <topology/static-persistence.h>
#include <topology/persistence-diagram.h>
#include <iostream>

#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>

#include <boost/program_options.hpp>


typedef Filtration<AlphaSimplex3DVector>        AlphaFiltration;
typedef StaticPersistence<>                     Persistence;
typedef PersistenceDiagram<>                    PDgm;

namespace po = boost::program_options;

int main(int argc, char** argv) 
{
#ifdef LOGGING
    rlog::RLogInit(argc, argv);

    stdoutLog.subscribeTo( RLOG_CHANNEL("info") );
    stdoutLog.subscribeTo( RLOG_CHANNEL("error") );
    //stdoutLog.subscribeTo( RLOG_CHANNEL("topology/persistence") );
    //stdoutLog.subscribeTo( RLOG_CHANNEL("topology/chain") );
#endif

    // SetFrequency(GetCounter("persistence/pair"), 10000);
    // SetTrigger(GetCounter("persistence/pair"), GetCounter(""));

    std::string     infilename, outfilename;

    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("input-file",   po::value<std::string>(&infilename),     "Point set whose alpha shape filtration and persistence we want to compute")
        ("output-file",  po::value<std::string>(&outfilename),    "Where to write the collection of persistence diagrams");

    po::positional_options_description pos;
    pos.add("input-file", 1);
    pos.add("output-file", 2);
    
    po::options_description all; all.add(hidden);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
                  options(all).positional(pos).run(), vm);
    po::notify(vm);

    if (!vm.count("input-file") || !vm.count("output-file"))
    { 
        std::cout << "Usage: " << argv[0] << " input-file output-file" << std::endl;
        std::cout << hidden << std::endl; 
        return 1; 
    }


    // Read in the point set and compute its Delaunay triangulation
    std::ifstream in(infilename.c_str());
    double x,y,z;
    Delaunay3D Dt;
    while(in)
    {
        in >> x >> y >> z;
        Point p(x,y,z);
        Dt.insert(p);
    }
    rInfo("Delaunay triangulation computed");
   
    AlphaSimplex3DVector complex;
    fill_complex(Dt, complex);
    rInfo("Simplices: %d", complex.size());

    // Create the alpha-shape filtration
    AlphaFiltration af(complex.begin(), complex.end(), AlphaSimplex3D::AlphaOrder());
    rInfo("Filtration initialized");
    
    Persistence p(af);
    rInfo("Persistence initializaed");

    p.pair_simplices();
    rInfo("Simplices paired");

    std::map<Dimension, PDgm> dgms;
    init_diagrams(dgms, p.begin(), p.end(), 
                  evaluate_through_map(OffsetMap<Persistence::OrderIndex, AlphaFiltration::Index>(p.begin(), af.begin()), 
                                       evaluate_through_filtration(af, AlphaSimplex3D::AlphaValueEvaluator())), 
                  evaluate_through_map(OffsetMap<Persistence::OrderIndex, AlphaFiltration::Index>(p.begin(), af.begin()), 
                                       evaluate_through_filtration(af, AlphaSimplex3D::DimensionExtractor())));
#if 0
    std::cout << 0 << std::endl << dgms[0] << std::endl;
    std::cout << 1 << std::endl << dgms[1] << std::endl;
    std::cout << 2 << std::endl << dgms[2] << std::endl;
#endif

    std::ofstream ofs(outfilename.c_str());
    boost::archive::binary_oarchive oa(ofs);
    oa << dgms;
}
