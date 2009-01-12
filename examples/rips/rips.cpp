#include <topology/rips.h>

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

int main(int argc, char* argv[])
{
#ifdef LOGGING
	rlog::RLogInit(argc, argv);

	stdoutLog.subscribeTo( RLOG_CHANNEL("error") );
	stdoutLog.subscribeTo( RLOG_CHANNEL("rips/info") );
#endif

    Distances distances;
    
#if 0
    // Storing ExplicitDistances speeds up the computation (at the price of memory)
    typedef         RipsGenerator<ExplicitDistances<Distances> >    RipsGenerator;
    ExplicitDistances<Distances> explicit_distances(distances);
    RipsGenerator rips(explicit_distances);
#else
    //typedef         RipsGeneratorMemory<Distances>                        RipsGenerator;
    typedef         RipsGenerator<Distances>                        RipsGenerator;
    RipsGenerator   rips(distances);
#endif

    RipsGenerator::SimplexVector complex;
    //rips.generate(complex, 3, distances.size());
    rips.generate(complex, 3, 50);
    
    std::cout << "Size: " << complex.size() << std::endl;
//    for (RipsGenerator::SimplexVector::const_iterator cur = complex.begin(); cur != complex.end(); ++cur)
//        std::cout << *cur << std::endl;
}
