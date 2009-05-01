#include <topology/rips.h>
#include <topology/filtration.h>
#include <topology/static-persistence.h>
#include <topology/dynamic-persistence.h>
#include <topology/persistence-diagram.h>

#include <geometry/l2distance.h>

#include <utilities/containers.h>           // for BackInsertFunctor
#include <utilities/timer.h>

#include <vector>

#include <boost/program_options.hpp>


typedef         PairwiseDistances<PointContainer, L2Distance>           PairDistances;
typedef         PairDistances::DistanceType                             DistanceType;
typedef         PairDistances::IndexType                                Vertex;

typedef         Rips<PairDistances>                                     Generator;
typedef         Generator::Simplex                                      Smplx;
typedef         std::vector<Smplx>                                      SimplexVector;
typedef         Filtration<SimplexVector, unsigned>                     Fltr;
//typedef         StaticPersistence<>                                     Persistence;
typedef         DynamicPersistenceChains<>                              Persistence;
typedef         PersistenceDiagram<>                                    PDgm;

void            program_options(int argc, char* argv[], std::string& infilename, Dimension& ambient, Dimension& skeleton, DistanceType& max_distance);

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
    SimplexVector           complex;
    
    // Generate 2-skeleton of the Rips complex for epsilon = 50
    rips.generate(skeleton, max_distance, make_push_back_functor(complex));
    std::sort(complex.begin(), complex.end(), Smplx::VertexComparison());       // unnecessary
    std::cout << "Generated complex of size: " << complex.size() << std::endl;

    // Generate filtration with respect to distance and compute its persistence
    Fltr f(complex.begin(), complex.end(), Generator::Comparison(distances));

    Timer persistence_timer; persistence_timer.start();
    Persistence p(f);
    p.pair_simplices();
    persistence_timer.stop();

    // Output cycles
    for (Persistence::OrderIndex cur = p.begin(); cur != p.end(); ++cur)
    {
        Persistence::Cycle& cycle = cur->cycle;

        if (!cur->sign())        // only negative simplices have non-empty cycles
        {
            Persistence::OrderIndex birth = cur->pair;      // the cycle that cur killed was born when we added birth (another simplex)

            const Smplx& b = f.simplex(f.begin() + (birth - p.begin()));        // eventually this will be encapsulated behind an interface
            const Smplx& d = f.simplex(f.begin() + (cur   - p.begin()));
            
            if (b.dimension() != 1) continue;
            std::cout << "Pair: (" << size(b) << ", " << size(d) << ")" << std::endl;
        } else if (cur->pair == cur)    // positive could be unpaired
        {
            const Smplx& b = f.simplex(f.begin() + (cur - p.begin()));
            if (b.dimension() != 1) continue;
            
            std::cout << "Unpaired birth: " << size(b) << std::endl;
            cycle = cur->chain;
        }

        // Iterate over the cycle
        for (Persistence::Cycle::const_iterator si =  cycle.begin();
                                                                 si != cycle.end();     ++si)
        {
            const Smplx& s = f.simplex(f.begin() + (*si - p.begin()));
            //std::cout << s.dimension() << std::endl;
            const Smplx::VertexContainer& vertices = s.vertices();          // std::vector<Vertex> where Vertex = Distances::IndexType
            AssertMsg(vertices.size() == s.dimension() + 1, "dimension of a simplex is one less than the number of its vertices");
            std::cout << vertices[0] << " " << vertices[1] << std::endl;
        }
    }
    
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
