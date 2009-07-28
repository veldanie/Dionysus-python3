#include <topology/cohomology-persistence.h>
#include <topology/weighted-rips.h>

#include <geometry/weighted-l2distance.h>
#include <geometry/distances.h>

#include <utilities/containers.h>           // for BackInsertFunctor
#include <utilities/timer.h>
#include <utilities/log.h>

#include <string>

#include <boost/tuple/tuple.hpp>
#include <boost/program_options.hpp>
#include <boost/progress.hpp>

typedef     PairwiseDistances<PointContainer, WeightedL2Distance>   PairDistances;
typedef     PairDistances::DistanceType                             DistanceType;
typedef     PairDistances::IndexType                                Vertex;
 
typedef     WeightedRips<PairDistances>                             Generator;
typedef     Generator::Simplex                                      Smplx;
typedef     std::vector<Smplx>                                      SimplexVector;
typedef     SimplexVector::const_iterator                           SV_const_iterator;

typedef     boost::tuple<Dimension, DistanceType>                   BirthInfo;
typedef     CohomologyPersistence<BirthInfo, SV_const_iterator>     Persistence;
typedef     Persistence::SimplexIndex                               Index;
typedef     Persistence::Death                                      Death;
typedef     std::map<Smplx, Index, Smplx::VertexComparison>         Complex;

#include "output.h"         // for output_*()

void        program_options(int argc, char* argv[], std::string& infilename, Dimension& skeleton, DistanceType& max_distance, ZpField::Element& prime, std::string& boundary_name, std::string& cocycle_prefix, std::string& vertices_name, std::string& diagram_name);

int main(int argc, char* argv[])
{
#ifdef LOGGING
    rlog::RLogInit(argc, argv);

    stderrLog.subscribeTo( RLOG_CHANNEL("error") );
#endif

    Dimension               skeleton;
    DistanceType            max_distance;
    ZpField::Element        prime;
    std::string             infilename, boundary_name, cocycle_prefix, vertices_name, diagram_name;

    program_options(argc, argv, infilename, skeleton, max_distance, prime, boundary_name, cocycle_prefix, vertices_name, diagram_name);
    std::ofstream           bdry_out(boundary_name.c_str());
    std::ofstream           vertices_out(vertices_name.c_str());
    std::ofstream           diagram_out(diagram_name.c_str());
    std::cout << "Boundary matrix: " << boundary_name << std::endl;
    std::cout << "Cocycles:        " << cocycle_prefix << "*.ccl" << std::endl;
    std::cout << "Vertices:        " << vertices_name << std::endl;
    std::cout << "Diagram:         " << diagram_name << std::endl;

    Timer total_timer; total_timer.start();
    PointContainer          points;
    read_weighted_points(infilename, points);

    PairDistances           distances(points);
    Generator               rips(distances);
    Generator::Evaluator    size(distances);
    Generator::Comparison   cmp(distances);
    SimplexVector           v;
    Complex                 c;
    
    Timer rips_timer; rips_timer.start();
    rips.generate(skeleton, max_distance, make_push_back_functor(v));
    std::sort(v.begin(), v.end(), DataDimensionComparison<Smplx>());
    rips_timer.stop();
    std::cout << "Simplex vector generated, size: " << v.size() << std::endl;

    /*output_boundary_matrix(bdry_out, v, cmp);
    output_vertex_indices(vertices_out, v);*/

    Timer persistence_timer; persistence_timer.start();
    ZpField                 zp(prime);
    Persistence             p(zp);
    boost::progress_display show_progress(v.size());
    for (SimplexVector::const_iterator cur = v.begin(); cur != v.end(); ++cur)
    {
        std::vector<Index>      boundary;
        for (Smplx::BoundaryIterator bcur  = cur->boundary_begin(); bcur != cur->boundary_end(); ++bcur)
            boundary.push_back(c[*bcur]);
        
        Index idx; Death d;
        bool store = cur->dimension() < skeleton;
        boost::tie(idx, d)      = p.add(boundary.begin(), boundary.end(), boost::make_tuple(cur->dimension(), size(*cur)), store, cur);
        
        // c[*cur] = idx;
        if (store)
            c.insert(std::make_pair(*cur, idx));

        if (d && (size(*cur) - d->get<1>()) > 0)
        {
            AssertMsg(d->get<0>() == cur->dimension() - 1, "Dimensions must match");
            diagram_out << (cur->dimension() - 1) << " " << d->get<1>() << " " << size(*cur) << std::endl;
        }
	++show_progress;
    }
    // output infinte persistence pairs 
    for (Persistence::CocycleIndex cur = p.begin(); cur != p.end(); ++cur)
        diagram_out << cur->birth.get<0>() << " " << cur->birth.get<1>() << " inf" << std::endl;
    persistence_timer.stop();


    // p.show_cocycles();
    // Output alive cocycles of dimension 1
    unsigned i = 0;
    for (Persistence::Cocycles::const_iterator cur = p.begin(); cur != p.end(); ++cur)
    {
        if (cur->birth.get<0>() != 1) continue;
        output_cocycle(cocycle_prefix, i, v, *cur, prime, cmp);
        // std::cout << "Cocycle of dimension: " << cur->birth.get<0>() << " born at " << cur->birth.get<1>() << std::endl;
        ++i;
    }
    total_timer.stop();
    rips_timer.check("Rips timer");
    persistence_timer.check("Persistence timer");
    total_timer.check("Total timer");
}

void        program_options(int argc, char* argv[], std::string& infilename, Dimension& skeleton, DistanceType& max_distance, ZpField::Element& prime, std::string& boundary_name, std::string& cocycle_prefix, std::string& vertices_name, std::string& diagram_name)
{
    namespace po = boost::program_options;

    po::options_description     hidden("Hidden options");
    hidden.add_options()
        ("input-file",          po::value<std::string>(&infilename),        "Point set whose Rips zigzag we want to compute");
    
    po::options_description visible("Allowed options", 100);
    visible.add_options()
        ("help,h",                                                                                  "produce help message")
        ("skeleton-dimsnion,s", po::value<Dimension>(&skeleton)->default_value(2),                  "Dimension of the Rips complex we want to compute")
        ("prime,p",             po::value<ZpField::Element>(&prime)->default_value(11),             "Prime p for the field F_p")
        ("max-distance,m",      po::value<DistanceType>(&max_distance)->default_value(Infinity),    "Maximum value for the Rips complex construction")
        ("boundary,b",          po::value<std::string>(&boundary_name),                             "Filename where to output the boundary matrix")
        ("cocycle,c",           po::value<std::string>(&cocycle_prefix),                            "Prefix of the filename where to output the 1-dimensional cocycles")
        ("vertices,v",          po::value<std::string>(&vertices_name),                             "Filename where to output the simplex-vertex mapping")
        ("diagram,d",           po::value<std::string>(&diagram_name),                              "Filename where to output the persistence diagram");
#if LOGGING
    std::vector<std::string>    log_channels;
    visible.add_options()
        ("log,l",               po::value< std::vector<std::string> >(&log_channels),           "log channels to turn on (info, debug, etc)");
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
        stderrLog.subscribeTo( RLOG_CHANNEL(cur->c_str()) );
#endif

    if (vm.count("help") || !vm.count("input-file"))
    { 
        std::cout << "Usage: " << argv[0] << " [options] input-file" << std::endl;
        std::cout << visible << std::endl; 
        std::abort();
    }
}
