#include <topology/cohomology-persistence.h>
#include <topology/rips.h>

#include <geometry/l2distance.h>

#include <utilities/containers.h>           // for BackInsertFunctor
#include <utilities/timer.h>

#include <string>

#include <boost/tuple/tuple.hpp>
#include <boost/program_options.hpp>

typedef     PairwiseDistances<PointContainer, L2Distance>           PairDistances;
typedef     PairDistances::DistanceType                             DistanceType;
typedef     PairDistances::IndexType                                Vertex;
    
typedef     CohomologyPersistence<DistanceType>                     Persistence;
typedef     Persistence::SimplexIndex                               Index;
typedef     Persistence::Death                                      Death;

typedef     Rips<PairDistances>                                     Generator;
typedef     Generator::Simplex                                      Smplx;

typedef     std::map<Smplx, Index, 
                     Smplx::VertexComparison>                       Complex;
typedef     std::vector<Smplx>                                      SimplexVector;

void        program_options(int argc, char* argv[], std::string& infilename, Dimension& ambient, Dimension& skeleton, DistanceType& max_distance);

int main(int argc, char* argv[])
{
    Dimension               ambient, skeleton;
    DistanceType            max_distance;
    std::string             infilename;

    program_options(argc, argv, infilename, ambient, skeleton, max_distance);

    PointContainer          points;
    read_points(infilename, points, ambient);

    PairDistances           distances(points);
    Generator               rips(distances);
    Generator::Evaluator    size(distances);
    SimplexVector           v;
    Complex                 c;
    
    rips.generate(skeleton, max_distance, make_push_back_functor(v));
    std::sort(v.begin(), v.end(), Generator::Comparison(distances));
    std::cout << "Simplex vector generated, size: " << v.size() << std::endl;

    Timer persistence_timer; persistence_timer.start();
    Persistence p;
    for (SimplexVector::const_iterator cur = v.begin(); cur != v.end(); ++cur)
    {
        std::vector<Index>      boundary;
        for (Smplx::BoundaryIterator bcur  = cur->boundary_begin(); 
                                     bcur != cur->boundary_end();       ++bcur)
            boundary.push_back(c[*bcur]);
        
        Index idx; Death d;
        boost::tie(idx, d)      = p.add(boundary.begin(), boundary.end(), size(*cur));
        c[*cur] = idx;
        if (d && (size(*cur) - *d) > 0)
            std::cout << (cur->dimension() - 1) << " " << *d << " " << size(*cur) << std::endl;
    }
    persistence_timer.stop();
    persistence_timer.check("Persistence timer");
}

void        program_options(int argc, char* argv[], std::string& infilename, Dimension& ambient, Dimension& skeleton, DistanceType& max_distance)
{
    namespace po = boost::program_options;

    po::options_description     hidden("Hidden options");
    hidden.add_options()
        ("input-file",          po::value<std::string>(&infilename),        "Point set whose Rips zigzag we want to compute");
    
    po::options_description visible("Allowed options", 100);
    visible.add_options()
        ("help,h",                                                                                  "produce help message")
        ("ambient-dimsnion,a",  po::value<Dimension>(&ambient)->default_value(3),                   "The ambient dimension of the point set")
        ("skeleton-dimsnion,s", po::value<Dimension>(&skeleton)->default_value(2),                  "Dimension of the Rips complex we want to compute")
        ("max-distance,m",      po::value<DistanceType>(&max_distance)->default_value(Infinity),    "Maximum value for the Rips complex construction");

    po::positional_options_description pos;
    pos.add("input-file", 1);
    pos.add("output-file", 2);
    
    po::options_description all; all.add(visible).add(hidden);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
                  options(all).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("help") || !vm.count("input-file"))
    { 
        std::cout << "Usage: " << argv[0] << " [options] input-file" << std::endl;
        std::cout << visible << std::endl; 
        std::abort();
    }
}
