#include <topology/rips.h>

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
    typedef         RipsGenerator<ExplicitDistances<Distances> >    RipsGenerator;
    ExplicitDistances<Distances> explicit_distances(distances);
    RipsGenerator rips(explicit_distances);
#else
    typedef         RipsGeneratorMemory<Distances>                        RipsGenerator;
    RipsGenerator   rips(distances);
#endif

    RipsGenerator::SimplexVector complex;
    //rips.generate(complex, 3, distances.size());
    rips.generate(complex, 3, 50);
    //rips.print();
    
    std::cout << "Size: " << complex.size() << std::endl;
//    for (RipsGenerator::SimplexVector::const_iterator cur = complex.begin(); cur != complex.end(); ++cur)
//        std::cout << *cur << std::endl;
}
