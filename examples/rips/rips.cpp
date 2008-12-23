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
    ExplicitDistances<Distances> explicit_distances(distances);
    Rips<ExplicitDistances<Distances> > rips(explicit_distances);
#else
    Rips<Distances> rips(distances);
#endif

    //rips.generate(3, distances.size());
    rips.generate(3, 50);
    //rips.print();
    
    std::cout << "Size: " << rips.size() << std::endl;
}
