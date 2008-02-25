#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include "avida-population-detail.h"

#include <topology/lowerstarfiltration.h>


typedef         AvidaPopulationDetail::OrganismIndex                OrganismIndex;
struct          OrganismVertexType;
typedef         std::vector<OrganismVertexType>                     VertexVector;
typedef         VertexVector::iterator                              VertexIndex;
typedef         LowerStarFiltration<VertexIndex>                    LSFiltration;
typedef         LSFiltration::Simplex                               Simplex;

struct          OrganismVertexType: public LSFiltration::VertexType<OrganismIndex> 
{
    typedef     LSFiltration::VertexType<OrganismIndex>             Parent;
                OrganismVertexType(OrganismIndex i): Parent(i)      {}
};

struct          OrganismVertexComparison
{
    public:
        bool    operator()(VertexIndex i, VertexIndex j) const      
        { return i->index()->fitness() > j->index()->fitness(); }       
        // > because of -fitness, so that maxima turn into minima
};

typedef         LSFiltration::Vineyard                              LSVineyard;
class           StaticEvaluator: public LSVineyard::Evaluator
{
    public:
        virtual RealType        
                value(const Simplex& s) const       
        { return s.get_attachment()->index()->fitness(); }
};

std::ostream& operator<<(std::ostream& out, VertexIndex i)
{ return (out << (i->index())); }


int main(int argc, char** argv)
{
#ifdef LOGGING
    rlog::RLogInit(argc, argv);
    stderrLog.subscribeTo(RLOG_CHANNEL("error"));
    //stdoutLog.subscribeTo(RLOG_CHANNEL("info"));
#endif

    if (argc < 3)
    {
        std::cout << "USAGE: avida FILENAME DISTANCE" << std::endl;
        return 0;
    }

    AvidaPopulationDetail population(argv[1]);
    AvidaOrganismDetail::DistanceType connected_distance;
    std::istringstream(argv[2]) >> connected_distance;
    const AvidaPopulationDetail::OrganismVector& organisms = population.get_organisms();

    rInfo("Number of organisms: %d", organisms.size());
    for (int i = 0; i < population.get_organisms().size(); ++i)
        rInfo("%d (%s) %f %d %d", organisms[i].id(),
                                  organisms[i].genome().c_str(),
                                  organisms[i].fitness(),
                                  organisms[i].length(),
                                  organisms[i].genome().size());

    // Order vertices 
    StaticEvaluator     evaluator;
    LSVineyard          vineyard(&evaluator);
    VertexVector        vertices;
    for (OrganismIndex cur = organisms.begin(); cur != organisms.end(); ++cur)  vertices.push_back(cur);
    LSFiltration        fitness_filtration(vertices.begin(), vertices.end(), OrganismVertexComparison(), &vineyard);

    // Add simplices
    for (VertexIndex cur = vertices.begin(); cur != vertices.end(); ++cur)
        for (VertexIndex link = boost::next(cur); link != vertices.end(); ++link)
            if (cur->index()->genome_distance(*(link->index())) <= connected_distance)
            {
                Simplex s(2, cur); s.add(link);
                fitness_filtration.append(s);
            }
    rInfo("Number of simplices: %d", fitness_filtration.size());

    // Pair simplices
    fitness_filtration.fill_simplex_index_map();
    fitness_filtration.pair_simplices(false);            // pair simplices without storing trails

    //std::cout << "Outputting persistence pairs" << std::endl;
    for (LSFiltration::Index i = fitness_filtration.begin(); i != fitness_filtration.end(); ++i)
    {
        if (i->is_paired())
        {
            if (i->sign())
            {
                AssertMsg(i->dimension() == 0, "Expecting only 0-dimensional diagram");
                if (i->pair()->get_attachment() == i->vertices()[0]) continue;     // skip non-critical pairs
                std::cout << i->dimension() << " " 
                          << evaluator.value(*i) << " " 
                          << evaluator.value(*(i->pair())) << std::endl;
            }
        }
        else
        {
            if (i->dimension() != 0) continue;
            std::cout << i->dimension() << " "
                      << evaluator.value(*i) << " "
                      << "unpaired" << std::endl;
        }
    }

#if 0
    // Produce histogram
    std::sort(deaths.begin(), deaths.end());
    for (DeathVector::iterator cur = deaths.begin(); cur != deaths.end(); )
    {
        DeathVector::iterator nw = std::find_if(cur, deaths.end(), 
                                                std::bind2nd(std::greater<RealType>(), *cur));
        std::cout << *cur << "\t" << (nw - cur) << std::endl;
        cur = nw;
    }
    std::cout << "Total: " << deaths.size() + 1;        // +1 for the unpaired
#endif
}
