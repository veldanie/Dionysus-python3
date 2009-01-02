#include <topology/rips.h>
#include <topology/zigzag-persistence.h>
#include <utilities/types.h>
#include <utilities/containers.h>

#include <utilities/log.h>

#include <map>

// FIXME
struct Distances
{
    typedef         int             IndexType;
    typedef         double          DistanceType;

    DistanceType    operator()(IndexType a, IndexType b) const      { return std::abs(a - b); }

    size_t          size() const                                    { return 10; }
    IndexType       begin() const                                   { return 0; }
    IndexType       end() const                                     { return size(); }
};


typedef     Distances::IndexType                                    Vertex;
typedef     Simplex<Vertex>                                         Smplx;
typedef     ZigzagPersistence<unsigned>                             Zigzag;
typedef     Zigzag::SimplexIndex                                    Index;
typedef     std::map<Smplx, Index, Smplx::VertexComparison>         Complex;
typedef     Zigzag::ZColumn                                         Boundary;

typedef     RipsBase<Distances, Smplx>                              RipsHelper;
typedef     RipsHelper::Evaluator                                   SimplexEvaluator;


void        make_boundary(const Smplx& s, Complex& c, const Zigzag& zz, Boundary& b)
{
    for (Smplx::BoundaryIterator cur = s.boundary_begin(); cur != s.boundary_end(); ++cur)
        b.append(c[*cur], zz.cmp);

    rDebug("  Boundary:");
    for (Boundary::const_iterator cur = b.begin(); cur != b.end(); ++cur)
        rDebug("    %d", (*cur)->order);
}

int main(int argc, char* argv[])
{
#ifdef LOGGING
	rlog::RLogInit(argc, argv);

	stderrLog.subscribeTo( RLOG_CHANNEL("error") );
	stderrLog.subscribeTo( RLOG_CHANNEL("info") );
	stderrLog.subscribeTo( RLOG_CHANNEL("debug") );
	//stderrLog.subscribeTo( RLOG_CHANNEL("topology/persistence") );
#endif

    Distances distances;
    
    // Order vertices and epsilons
    typedef     std::vector<Vertex>                                 VertexVector;
    typedef     std::vector<Distances::DistanceType>                EpsilonVector;
    
    VertexVector        vertices;
    EpsilonVector       epsilons;
    EpsilonVector       closest(distances.size(), Infinity);

    vertices.push_back(0);
    while (vertices.size() < distances.size())
    {
        for (Distances::IndexType v = distances.begin(); v != distances.end(); ++v)
            closest[v] = std::min(closest[v], distances(v, vertices.back()));
        EpsilonVector::const_iterator max = std::max_element(closest.begin(), closest.end());
        vertices.push_back(max - closest.begin());
        epsilons.push_back(*max);
    }
    
    std::cout << "Point and epsilon ordering:" << std::endl;
    for (unsigned i = 0; i < vertices.size(); ++i)
        std::cout << "  " << vertices[i] << " " << epsilons[i] << std::endl;


    // Construct zigzag
    Complex             complex;
    Zigzag              zz;
    RipsHelper          aux(distances);
    SimplexEvaluator    size(distances);
    
    rInfo("Commencing computation");
    for (unsigned i = 0; i != vertices.size(); ++i)
    {
        rInfo("Current stage %d: %d %f", i, vertices[i], epsilons[i]);

        // Add a point
        Smplx sv; sv.add(vertices[i]);
        rDebug("Added  %s", tostring(sv).c_str());
        complex.insert(std::make_pair(sv, zz.add(Boundary(), i).first));
        if (!zz.check_consistency())
        {
            //zz.show_all();
            rError("Zigzag representation must be consistent after adding a vertex");
        }
        for (Complex::iterator si = complex.begin(); si != complex.end(); ++si)
        {
            if (si->first.contains(sv))      continue;
            rInfo("  Distance between %s and %s: %f", 
                     tostring(si->first).c_str(),
                     tostring(sv).c_str(),
                     aux.distance(si->first, sv));
            if (aux.distance(si->first, sv) <= epsilons[i-1])
            {
                Boundary b;
                Smplx s(si->first); s.join(sv);

                //zz.show_all();
                rDebug("Adding %s", tostring(s).c_str());
                make_boundary(s, complex, zz, b);
                rDebug("Made boundary, %d", b.size());
                Zigzag::IndexDeathPair idp = zz.add(b, i);
                if (!zz.check_consistency())
                {
                    //zz.show_all();
                    rError("Zigzag representation must be consistent after adding a simplex");
                }
                complex.insert(std::make_pair(s, idp.first));
                
                // Death
                if (idp.second)     std::cout << *(idp.second) << " " << i << std::endl;
            }
        }
        rDebug("Complex after addition:");
        for (Complex::const_iterator cur = complex.begin(); cur != complex.end(); ++cur)
            rDebug("  %s", tostring(cur->first).c_str());

        rInfo("Removing simplices");
        // Shrink epsilon
        {
            Complex::reverse_iterator si = complex.rbegin(); 
            
            while(si != complex.rend())
            {
                rInfo("  Size of %s is %f", tostring(si->first).c_str(), size(si->first));
                if (size(si->first) > epsilons[i])
                {
                    //zz.show_all();
                    rInfo("  Removing: %s", tostring(si->first).c_str());
                    Zigzag::Death d = zz.remove(si->second, i);
                    AssertMsg(zz.check_consistency(), "Zigzag representation must be consistent after removing a simplex");
                    if (d)              std::cout << *d << " " << i << std::endl;
                    complex.erase(boost::prior(si.base()));
                } else
                    ++si;
            }
            rDebug("Complex after removal:");
            for (Complex::const_iterator cur = complex.begin(); cur != complex.end(); ++cur)
                rDebug("  %s", tostring(cur->first).c_str());
        }
    }
}
