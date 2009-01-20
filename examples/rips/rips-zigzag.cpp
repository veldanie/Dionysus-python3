#include <topology/rips.h>
#include <topology/image-zigzag-persistence.h>
#include <utilities/types.h>
#include <utilities/containers.h>

#include <utilities/log.h>
#include <utilities/memory.h>

#include <map>
#include <cmath>
#include <fstream>
#include <stack>

#include <boost/tuple/tuple.hpp>
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
typedef     ImageZigzagPersistence<BirthInfo>                       Zigzag;
typedef     Zigzag::SimplexIndex                                    Index;
typedef     Zigzag::Death                                           Death;
typedef     std::map<Smplx, Index, 
                            Smplx::VertexDimensionComparison>       Complex;
typedef     Zigzag::ZColumn                                         Boundary;


void        make_boundary(const Smplx& s, Complex& c, const Zigzag& zz, Boundary& b)
{
    rDebug("  Boundary of <%s>", tostring(s).c_str());
    for (Smplx::BoundaryIterator cur = s.boundary_begin(); cur != s.boundary_end(); ++cur)
    {
        b.append(c[*cur], zz.cmp);
        rDebug("   %d (inL=%d)", c[*cur]->order, b.back()->subcomplex);
    }
}

bool        face_leaving_subcomplex(Complex::reverse_iterator si, const SimplexEvaluator& size, DistanceType after, DistanceType before)
{
    const Smplx& s = si->first;
    for (Smplx::VertexContainer::const_iterator v1 = s.vertices().begin(); v1 != s.vertices().end(); ++v1)
        for (Smplx::VertexContainer::const_iterator v2 = boost::next(v1); v2 != s.vertices().end(); ++v2)
        {
            Smplx e; e.add(*v1); e.add(*v2);
            if (size(e) > after && size(e) <= before)
                return true;
        }

    return false;
}

void        show_image_betti(Zigzag& zz, Dimension skeleton)
{
    for (Zigzag::ZIndex cur = zz.image_begin(); cur != zz.image_end(); ++cur)
        if (cur->low == zz.boundary_end() && cur->birth.second < skeleton)
            std::cout << "Class in the image of dimension: " << cur->birth.second << std::endl;
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
    
    SetFrequency(cOperations, 25000);
    SetTrigger(cOperations, cComplexSize);

    unsigned        ambient_dimension;
    unsigned        skeleton_dimension;
    float           from_multiplier, to_multiplier;
    std::string     infilename;

    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("input-file",          po::value<std::string>(&infilename),        "Point set whose Rips zigzag we want to compute");
    
    po::options_description visible("Allowed options", 100);
    visible.add_options()
        ("help,h",                                                                              "produce help message")
        ("ambient-dimsnion,a",  po::value<unsigned>(&ambient_dimension)->default_value(3),      "The ambient dimension of the point set")
        ("skeleton-dimsnion,s", po::value<unsigned>(&skeleton_dimension)->default_value(2),     "Dimension of the Rips complex we want to compute")
        ("from,f",              po::value<float>(&from_multiplier)->default_value(4),           "From multiplier for the epsilon (distance to next maxmin point) when computing the Rips complex")
        ("to,t",                po::value<float>(&to_multiplier)->default_value(16),            "To multiplier for the epsilon (distance to next maxmin point) when computing the Rips complex");
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
        //epsilons.push_back(Infinity);
        while (vertices.size() < distances.size())
        {
            for (Vertex v = distances.begin(); v != distances.end(); ++v)
                dist[v] = std::min(dist[v], distances(v, vertices.back()));
            EpsilonVector::const_iterator max = std::max_element(dist.begin(), dist.end());
            vertices.push_back(max - dist.begin());
            epsilons.push_back(*max);
        }
        epsilons.push_back(0);
    }
    
    rInfo("Point and epsilon ordering:");
    for (unsigned i = 0; i < vertices.size(); ++i)
        rInfo("  %d %f", vertices[i], epsilons[i]);


    // Construct zigzag
    Complex             complex;
    Zigzag              zz;
    RipsHelper          aux(distances);
    SimplexEvaluator    size(distances);
    
    // TODO: it probably makes sense to do things in reverse. 
    // I.e., we should start from the smallest epsilon, and grow, rather than 
    // starting from the largest epsilon and shrinking since the interesting 
    // part of the computation is that with small epsilon.
    rInfo("Commencing computation");
    for (unsigned i = 0; i != vertices.size(); ++i)
    {
        rInfo("Current stage %d: %d %f", i, vertices[i], epsilons[i]);

        // Add a point
        Smplx sv; sv.add(vertices[i]);
        rDebug("Added  %s", tostring(sv).c_str());
        complex.insert(std::make_pair(sv, 
                                      zz.add(Boundary(), 
                                             true,         // vertex is always in the subcomplex
                                             std::make_pair(epsilons[i], 0)).first));
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
            if (aux.distance(si->first, sv) <= to_multiplier*epsilons[i-1])
            {
                Boundary b;
                Smplx s(si->first); s.join(sv);
    
                //zz.show_all();
                rDebug("Adding %s", tostring(s).c_str());
                make_boundary(s, complex, zz, b);
                rDebug("Made boundary, %d", b.size());
                Index idx; Death d;
                boost::tie(idx, d) = zz.add(b, 
                                            (size(s) <= from_multiplier*epsilons[i-1]), 
                                            std::make_pair(epsilons[i-1], s.dimension()));
                if (!zz.check_consistency())
                {
                    //zz.show_all();
                    rError("Zigzag representation must be consistent after adding a simplex");
                }
                complex.insert(std::make_pair(s, idx));
                CountNum(cComplexSize, s.dimension());
                Count(cComplexSize);
                Count(cOperations);
                
                // Death
                if (d && ((d->first - epsilons[i-1]) != 0) && (d->second < skeleton_dimension))     
                    std::cout << d->second << " " << d->first << " " << epsilons[i-1] << std::endl;
            }
        }
        rDebug("Complex after addition:");
        for (Complex::const_iterator si = complex.begin(); si != complex.end(); ++si)
           rDebug("    %s", tostring(si->first).c_str());

        rInfo("Inserted point; complex size: %d", complex.size());
        show_image_betti(zz, skeleton_dimension);
        report_memory();

        if (i == 0) continue;       // want to skip the removal from the image check (involving epsilons[i-1]), 
                                    // and in any case, there is only one vertex at this point
        rDebug("Removing simplices");
        // Shrink epsilon
        {
            std::stack<Complex::reverse_iterator>       leaving_subcomplex;
            Complex::reverse_iterator si = complex.rbegin();
            
            while(si != complex.rend())
            {
                rDebug("  Size of %s is %f", tostring(si->first).c_str(), size(si->first));
                if (size(si->first) > to_multiplier*epsilons[i])
                {
                    //zz.show_all();
                    rDebug("  Removing from complex:   %s", tostring(si->first).c_str());
                    Death d = zz.remove(si->second, 
                                        std::make_pair(epsilons[i], si->first.dimension() - 1));
                    AssertMsg(zz.check_consistency(), "Zigzag representation must be consistent after removing a simplex");
                    if (d && ((d->first - epsilons[i]) != 0) && (d->second < skeleton_dimension))
                        std::cout << d->second << " " << d->first << " " << epsilons[i] << std::endl;
                    CountNumBy(cComplexSize, si->first.dimension(), -1);
                    complex.erase(boost::prior(si.base()));
                    CountBy(cComplexSize, -1);
                    Count(cOperations);
                } else if (face_leaving_subcomplex(si, size, from_multiplier*epsilons[i], from_multiplier*epsilons[i-1]))
                {
                    // Remove from subcomplex (i.e., remove and reinsert as outside of the subcomplex)
                    rDebug("  Removing from subcomplex: %s", tostring(si->first).c_str());
                    Death d = zz.remove(si->second, 
                                        std::make_pair(epsilons[i], si->first.dimension() - 1));
                    Count(cOperations);
                    AssertMsg(zz.check_consistency(), "Zigzag representation must be consistent after removing a simplex");
                    if (d && ((d->first - epsilons[i]) != 0) && (d->second < skeleton_dimension))
                        std::cout << d->second << " " << d->first << " " << epsilons[i] << std::endl;
                    leaving_subcomplex.push(si++);
                } else
                    ++si;
            }
            while(!leaving_subcomplex.empty())
            {
                si = leaving_subcomplex.top();          // copying an iterator onto stack is probably Ok
                Boundary b;
                make_boundary(si->first, complex, zz, b);
                Index idx; Death d;
                boost::tie(idx, d) = zz.add(b,
                                            false,      // now it is outside of the subcomplex
                                            std::make_pair(epsilons[i], si->first.dimension()));
                Count(cOperations);
                si->second = idx;
                if (d && ((d->first - epsilons[i]) != 0) && (d->second < skeleton_dimension))
                    std::cout << d->second << " " << d->first << " " << epsilons[i] << std::endl;
                leaving_subcomplex.pop();
            }
        }
        rDebug("Complex after removal:");
        for (Complex::const_iterator si = complex.begin(); si != complex.end(); ++si)
            rDebug("    %s", tostring(si->first).c_str());
        
        rInfo("Shrunk epsilon; complex size: %d", complex.size());
        show_image_betti(zz, skeleton_dimension);
        report_memory();
    }
}
