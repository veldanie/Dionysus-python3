#include <topology/rips.h>
#include <topology/zigzag-persistence.h>
#include <utilities/types.h>
#include <utilities/containers.h>

#include <utilities/log.h>

#include <map>
#include <cmath>
#include <fstream>

#include <boost/program_options.hpp>


#ifdef COUNTERS
static Counter*  cComplexSize =                     GetCounter("rips/size");
static Counter*  cOperations =                      GetCounter("rips/operations");
#endif // COUNTERS

typedef     std::vector<double>                                     Point;
typedef     std::vector<Point>                                      PointContainer;
struct L2Distance
{
    typedef     double                                              value_type;

    value_type  operator()(const Point& p1, const Point& p2) const
    {
        AssertMsg(p1.size() == p2.size(), "Points must be in the same dimension (in L2Distance");
        value_type sum = 0;
        for (size_t i = 0; i < p1.size(); ++i)
            sum += (p1[i] - p2[i])*(p1[i] - p2[i]);

        return sqrt(sum);
    }
};
typedef     PairwiseDistances<PointContainer, L2Distance>           PairDistances;
typedef     PairDistances::DistanceType                             DistanceType;

typedef     PairDistances::IndexType                                Vertex;
typedef     Simplex<Vertex>                                         Smplx;

typedef     RipsBase<PairDistances, Smplx>                          RipsHelper;
typedef     RipsHelper::Evaluator                                   SimplexEvaluator;

typedef     std::pair<DistanceType, Dimension>                      BirthInfo;
typedef     ZigzagPersistence<BirthInfo>                            Zigzag;
typedef     Zigzag::SimplexIndex                                    Index;
typedef     std::map<Smplx, Index, 
                            Smplx::VertexDimensionComparison>       Complex;
typedef     Zigzag::ZColumn                                         Boundary;


void        make_boundary(const Smplx& s, Complex& c, const Zigzag& zz, Boundary& b)
{
    Dimension bdry_dim = s.dimension() - 1;
    for (Smplx::BoundaryIterator cur = s.boundary_begin(); cur != s.boundary_end(); ++cur)
        b.append(c[*cur], zz.cmp);

    rDebug("  Boundary:");
    for (Boundary::const_iterator cur = b.begin(); cur != b.end(); ++cur)
        rDebug("    %d", (*cur)->order);
}

std::ostream&   operator<<(std::ostream& out, const BirthInfo& bi)
{ return (out << bi.first); }

namespace po = boost::program_options;


int main(int argc, char* argv[])
{
#ifdef LOGGING
	rlog::RLogInit(argc, argv);

	stdoutLog.subscribeTo( RLOG_CHANNEL("error") );
#endif
    
    SetFrequency(cOperations, 500);
    SetTrigger(cOperations, cComplexSize);

    unsigned        ambient_dimension;
    unsigned        skeleton_dimension;
    float           multiplier;
    std::string     infilename;

    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("input-file",          po::value<std::string>(&infilename),        "Point set whose Rips zigzag we want to compute");
    
    po::options_description visible("Allowed options", 100);
    visible.add_options()
        ("help,h",                                                                              "produce help message")
        ("ambient-dimsnion,a",  po::value<unsigned>(&ambient_dimension)->default_value(3),      "The ambient dimension of the point set")
        ("skeleton-dimsnion,s", po::value<unsigned>(&skeleton_dimension)->default_value(2),     "Dimension of the Rips complex we want to compute")
        ("multiplier,m",        po::value<float>(&multiplier)->default_value(4),                "Multiplier for the epsilon (distance to next maxmin point) when computing the Rips complex");
#if LOGGING
    std::vector<std::string>    log_channels;
    visible.add_options()
        ("log,l",               po::value< std::vector<std::string> >(&log_channels),   "log channels to turn on (info, debug, etc)");
#endif

    po::positional_options_description pos;
    pos.add("input-file", 1);
    
    po::options_description all; all.add(visible).add(hidden);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
                  options(all).positional(pos).run(), vm);
    po::notify(vm);

#if LOGGING
    for (std::vector<std::string>::const_iterator cur = log_channels.begin(); cur != log_channels.end(); ++cur)
        stdoutLog.subscribeTo( RLOG_CHANNEL(cur->c_str()) );
	/* Interesting channels
     * "info", "debug", "topology/persistence"
     */
#endif

    if (vm.count("help") || !vm.count("input-file"))
    { 
        std::cout << "Usage: " << argv[0] << " [options] input-file" << std::endl;
        std::cout << visible << std::endl; 
        return 1; 
    }

    // Read in points
    std::ifstream in(infilename.c_str());
    PointContainer      points;
    while(in)
    {
        points.push_back(Point());
        for (unsigned i = 0; i < ambient_dimension; ++i)
        {
            DistanceType    x;
            in >> x;
            points.back().push_back(x);
        }
    }
    
    // Create pairwise distances
    PairDistances distances(points);
    
    // Order vertices and epsilons
    typedef     std::vector<Vertex>                                 VertexVector;
    typedef     std::vector<DistanceType>                           EpsilonVector;
    
    VertexVector        vertices;
    EpsilonVector       epsilons;

    {
        EpsilonVector   dist(distances.size(), Infinity);
    
        vertices.push_back(distances.begin());
        while (vertices.size() < distances.size())
        {
            for (Vertex v = distances.begin(); v != distances.end(); ++v)
                dist[v] = std::min(dist[v], distances(v, vertices.back()));
            EpsilonVector::const_iterator max = std::max_element(dist.begin(), dist.end());
            vertices.push_back(max - dist.begin());
            epsilons.push_back(*max);
        }
    }
    
    rInfo("Point and epsilon ordering:");
    for (unsigned i = 0; i < vertices.size(); ++i)
        rInfo("  %d %f", vertices[i], epsilons[i]);


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
        complex.insert(std::make_pair(sv, zz.add(Boundary(), std::make_pair(epsilons[i], 0)).first));
        CountNum(cComplexSize, 0);
        Count(cComplexSize);
        Count(cOperations);
        if (!zz.check_consistency())
        {
            //zz.show_all();
            rError("Zigzag representation must be consistent after adding a vertex");
        }
        for (Complex::iterator si = complex.begin(); si != complex.end(); ++si)
        {
            if (si->first.contains(sv))                             continue;
            if (si->first.dimension() + 1 > skeleton_dimension)     continue;
    
            rDebug("  Distance between %s and %s: %f", 
                     tostring(si->first).c_str(),
                     tostring(sv).c_str(),
                     aux.distance(si->first, sv));
            if (aux.distance(si->first, sv) <= multiplier*epsilons[i-1])
            {
                Boundary b;
                Smplx s(si->first); s.join(sv);
    
                //zz.show_all();
                rDebug("Adding %s", tostring(s).c_str());
                make_boundary(s, complex, zz, b);
                rDebug("Made boundary, %d", b.size());
                Zigzag::IndexDeathPair idp = zz.add(b, std::make_pair(epsilons[i], sv.dimension()));
                if (!zz.check_consistency())
                {
                    //zz.show_all();
                    rError("Zigzag representation must be consistent after adding a simplex");
                }
                complex.insert(std::make_pair(s, idp.first));
                CountNum(cComplexSize, s.dimension());
                Count(cComplexSize);
                Count(cOperations);
                
                // Death
                if (idp.second)     std::cout << (idp.second)->second << " " << (idp.second)->first << " " << epsilons[i] << std::endl;
            }
        }
        rDebug("Complex after addition:");
        for (Complex::const_iterator si = complex.begin(); si != complex.end(); ++si)
            rDebug("    %s", tostring(si->first).c_str());

        rDebug("Removing simplices");
        // Shrink epsilon
        {
            Complex::reverse_iterator si = complex.rbegin();
            
            while(si != complex.rend())
            {
                rDebug("  Size of %s is %f", tostring(si->first).c_str(), size(si->first));
                if (size(si->first) > multiplier*epsilons[i])
                {
                    //zz.show_all();
                    rDebug("  Removing: %s", tostring(si->first).c_str());
                    Zigzag::Death d = zz.remove(si->second, 
                                                std::make_pair(epsilons[i], si->first.dimension() - 1));
                    AssertMsg(zz.check_consistency(), "Zigzag representation must be consistent after removing a simplex");
                    if (d)              std::cout << d->second << " " << d->first << " " << epsilons[i] << std::endl;
                    CountNumBy(cComplexSize, si->first.dimension(), -1);
                    complex.erase(boost::prior(si.base()));
                    CountBy(cComplexSize, -1);
                    Count(cOperations);
                } else
                    ++si;
            }
        }
        rDebug("Complex after removal:");
        for (Complex::const_iterator si = complex.begin(); si != complex.end(); ++si)
            rDebug("    %s", tostring(si->first).c_str());
    }
}
