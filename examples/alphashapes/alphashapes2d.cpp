#include <utilities/log.h>

#include "alphashapes2d.h"
#include <topology/filtration.h>
#include <topology/static-persistence.h>
#include <topology/persistence-diagram.h>
#include <iostream>

#include <fstream>


typedef Filtration<AlphaSimplex2DVector>        AlphaFiltration;
typedef StaticPersistence<>                     Persistence;
typedef PersistenceDiagram<>                    PDgm;


int main(int argc, char** argv) 
{
#ifdef LOGGING
    rlog::RLogInit(argc, argv);

    stdoutLog.subscribeTo( RLOG_CHANNEL("error") );
    stdoutLog.subscribeTo( RLOG_CHANNEL("info") );
    //stdoutLog.subscribeTo( RLOG_CHANNEL("topology/filtration") );
    //stdoutLog.subscribeTo( RLOG_CHANNEL("topology/cycle") );
#endif

    SetFrequency(GetCounter("filtration/pair"), 10000);
    SetTrigger(GetCounter("filtration/pair"), GetCounter(""));

    // Read in the point set and compute its Delaunay triangulation
    std::istream& in = std::cin;
    double x,y;
    Delaunay2D Dt;
    while(in)
    {
        in >> x >> y;
        if (!in) break;
        Point p(x,y);
        Dt.insert(p);
    }
    rInfo("Delaunay triangulation computed");
   
    AlphaSimplex2DVector complex;
    fill_complex(Dt, complex);
    rInfo("Simplices: %i", complex.size());

    // Create the alpha-shape filtration
    AlphaFiltration af(complex.begin(), complex.end(), AlphaSimplex2D::AlphaOrder());
    rInfo("Filtration initialized");

    Persistence p(af);
    rInfo("Persistence initializaed");

    p.pair_simplices();
    rInfo("Simplices paired");

    std::map<Dimension, PDgm> dgms;
    init_diagrams(dgms, p.begin(), p.end(), 
                  evaluate_through_map(OffsetMap<Persistence::OrderIndex, AlphaFiltration::Index>(p.begin(), af.begin()), 
                                       evaluate_through_filtration(af, AlphaSimplex2D::AlphaValueEvaluator())), 
                  evaluate_through_map(OffsetMap<Persistence::OrderIndex, AlphaFiltration::Index>(p.begin(), af.begin()), 
                                       evaluate_through_filtration(af, AlphaSimplex2D::DimensionExtractor())));

#if 1
    std::cout << 0 << std::endl << dgms[0] << std::endl;
    std::cout << 1 << std::endl << dgms[1] << std::endl;
#endif
}

