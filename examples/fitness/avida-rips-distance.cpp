#include <iostream>
#include <vector>
#include <algorithm>
#include "avida-population-detail.h"

#include <topology/filtration.h>
#include <topology/rips.h>
#include <topology/static-persistence.h>


typedef         ExplicitDistances<AvidaPopulationDetail>            ExplicitDist;
typedef         Rips<ExplicitDist>                                  RipsGen;
typedef         RipsGen::Simplex                                    Smplx;
typedef         std::vector<Smplx>                                  Complex;

typedef         Filtration<Complex, unsigned>                       Fltr;
typedef         StaticPersistence<>                                 Persistence;

int main(int argc, char** argv)
{
#ifdef LOGGING
    rlog::RLogInit(argc, argv);
    stdoutLog.subscribeTo(RLOG_CHANNEL("info"));
    //stdoutLog.subscribeTo(RLOG_CHANNEL("rips/info"));
#endif

    if (argc < 2)
    {
        std::cout << "USAGE: avida FILENAME" << std::endl;
        return 0;
    }

    AvidaPopulationDetail population(argv[1]);
    ExplicitDist distances(population);

    RipsGen rips(distances);
    RipsGen::Evaluator evaluator(rips.distances());
    rInfo("Max distance: %f", rips.max_distance());

    const AvidaPopulationDetail::OrganismVector& organisms = population.get_organisms();
    rInfo("Number of organisms: %d", organisms.size());
    /* 
    for (int i = 0; i < population.get_organisms().size(); ++i)
        rInfo("%d (%s) %f %d %d", organisms[i].id(),
                                  organisms[i].genome().c_str(),
                                  organisms[i].fitness(),
                                  organisms[i].length(),
                                  organisms[i].genome().size());
   */

    rInfo("Starting to generate rips complex");
    Complex c;
    rips.generate(1, rips.max_distance()/2, make_push_back_functor(c));
    std::sort(c.begin(), c.end(), Smplx::VertexComparison());
    
    rInfo("Generated Rips complex, filling filtration");
    Fltr f(c.begin(), c.end(), RipsGen::Comparison(rips.distances()));

    Persistence p(f);
    p.pair_simplices();

    std::cout << "Outputting histogram of death values" << std::endl;
    typedef std::vector<RealType> DeathVector;
    DeathVector deaths;
    OffsetMap<Persistence::OrderIndex, Fltr::Index> m(p.begin(), f.begin());
    for (Persistence::OrderIndex i = p.begin(); i != p.end(); ++i)
    {
        if (i == i->pair) continue;
        if (i->sign())
        {
            const Smplx& s = f.simplex(m[i]);
            const Smplx& t = f.simplex(m[i->pair]);
            AssertMsg(s.dimension() == 0, "Expecting only 0-dimensional diagram");
            AssertMsg(evaluator(s) == 0, "Expecting only 0 birth values in 0-D diagram ");
            deaths.push_back(evaluator(t));
        }
    }

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
}
