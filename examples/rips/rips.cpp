#include <topology/rips.h>
#include <topology/filtration.h>
#include <topology/static-persistence.h>
#include <topology/persistence-diagram.h>
#include <utilities/containers.h>           // for BackInsertFunctor

#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>

// Trivial example of size() points on a line with integer coordinates
struct Distances
{
    typedef         int             IndexType;
    typedef         double          DistanceType;

    DistanceType    operator()(IndexType a, IndexType b) const      { return std::abs(a - b); }

    size_t          size() const                                    { return 2000; }
    IndexType       begin() const                                   { return 0; }
    IndexType       end() const                                     { return size(); }
};

//typedef         Rips<ExplicitDistances<Distances> >                   Generator;
typedef         Rips<Distances>                                         Generator;
typedef         Generator::Simplex                                      Smplx;
typedef         std::vector<Smplx>                                      SimplexVector;
typedef         Filtration<SimplexVector, unsigned>                     Fltr;
typedef         StaticPersistence<>                                     Persistence;
typedef         PersistenceDiagram<>            PDgm;


int main(int argc, char* argv[])
{
#ifdef LOGGING
	rlog::RLogInit(argc, argv);

	stdoutLog.subscribeTo( RLOG_CHANNEL("error") );
	stdoutLog.subscribeTo( RLOG_CHANNEL("info") );
	//stdoutLog.subscribeTo( RLOG_CHANNEL("rips/info") );
	//stdoutLog.subscribeTo( RLOG_CHANNEL("rips/debug") );
#endif

    Distances distances;
    
    // Storing ExplicitDistances speeds up the computation (at the price of memory)
    //ExplicitDistances<Distances> explicit_distances(distances);

    Generator               rips(distances);
    Generator::Evaluator    size(distances);
    SimplexVector           complex;
    
    // Generate 2-skeleton of the Rips complex for epsilon = 50
    rips.generate(2, 50, make_push_back_functor(complex));
    std::sort(complex.begin(), complex.end(), Smplx::VertexComparison());       // unnecessary
    rInfo("Generated complex of size: %d",  complex.size());

    // Generate filtration with respect to distance and compute its persistence
    Fltr f(complex.begin(), complex.end(), Generator::Comparison(distances));
    Persistence p(f);
    p.pair_simplices();
    rInfo("Simplices paired");

    // Record the persistence intervals in the persistence diagrams
    std::map<Dimension, PDgm> dgms;
    init_diagrams(dgms, p.begin(), p.end(), 
                  evaluate_through_map(make_offset_map(p.begin(), f.begin()), 
                                       evaluate_through_filtration(f, size)), 
                  evaluate_through_map(make_offset_map(p.begin(), f.begin()), 
                                       evaluate_through_filtration(f, Smplx::DimensionExtractor())));

    // Serialize the diagrams to a file
    std::ofstream ofs("rips-diagrams");
    boost::archive::binary_oarchive oa(ofs);
    oa << dgms;
}
