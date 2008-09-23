#include <iostream>
#include <vector>
#include <algorithm>
#include "avida-population-detail.h"

#include <topology/filtration.h>
#include <topology/simplex.h>
#include <topology/rips.h>


typedef         ExplicitDistances<AvidaPopulationDetail>            ExplicitDist;
typedef         Rips<ExplicitDist>                                  RipsComplex; 
typedef         RipsComplex::Simplex                                Simplex;
typedef         RipsComplex::SimplexVector                          SimplexVector;
typedef			Filtration<Simplex>									SimplexFiltration;

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

    RipsComplex rips(distances);
    RipsComplex::Evaluator evaluator(rips.distances());
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
    rips.generate(1, rips.max_distance()/2);
    
    rInfo("Generated Rips complex, filling filtration");
	SimplexFiltration filtration;
	for (SimplexVector::const_iterator cur = rips.simplices().begin(); cur != rips.simplices().end(); ++cur)
	{
		//rInfo("Simplex: %s %f", tostring(*cur).c_str(), evaluator.value(*cur));
		filtration.append(*cur);
	}

	filtration.fill_simplex_index_map();
	filtration.pair_simplices(false);			// pair simplices without storing trails

	std::cout << "Outputting histogram of death values" << std::endl;
	typedef std::vector<RealType> DeathVector;
	DeathVector deaths;
	for (SimplexFiltration::Index i = filtration.begin(); i != filtration.end(); ++i)
	{
		if (i->is_paired())
			if (i->sign())
			{
				AssertMsg(i->dimension() == 0, "Expecting only 0-dimensional diagram");
				AssertMsg(evaluator.value(*i) == 0, "Expecting only 0 birth values in 0-D diagram ");
				deaths.push_back(evaluator.value(*(i->pair())));
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
	std::cout << "Total: " << deaths.size() + 1;		// +1 for the unpaired
}
