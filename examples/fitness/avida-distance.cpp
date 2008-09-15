#include <iostream>
#include <vector>
#include <algorithm>
#include "avida-population-detail.h"

#include <topology/filtration.h>
#include <topology/simplex.h>


typedef			SimplexWithValue<AvidaOrganismDetail::IDType>		Simplex;
typedef			std::vector<Simplex>								SimplexVector;
typedef			Filtration<Simplex>									SimplexFiltration;

int main(int argc, char** argv)
{
#ifdef LOGGING
	rlog::RLogInit(argc, argv);
	//stdoutLog.subscribeTo(RLOG_CHANNEL("info"));
#endif

	if (argc < 2)
	{
		std::cout << "USAGE: avida FILENAME" << std::endl;
		return 0;
	}

	AvidaPopulationDetail population(argv[1]);
	const AvidaPopulationDetail::OrganismVector& organisms = population.get_organisms();

	rInfo("Number of organisms: %d", organisms.size());
	for (int i = 0; i < population.get_organisms().size(); ++i)
		rInfo("%d (%s) %f %d %d", organisms[i].id(),
								  organisms[i].genome().c_str(),
								  organisms[i].fitness(),
								  organisms[i].length(),
								  organisms[i].genome().size());

	// Distance function filtration
	SimplexVector simplices;

	// Insert edges
	AvidaOrganismDetail::DistanceType avg_distance = 0;
	for (AvidaOrganismDetail::CountType i = 0; i < organisms.size(); ++i)
	{
		simplices.push_back(0);
		simplices.back().add(organisms[i].id());

		for (AvidaOrganismDetail::CountType j = i+1; j < organisms.size(); ++j)
		{
			avg_distance += organisms[i].genome_distance(organisms[j]);
			simplices.push_back(Simplex(organisms[i].genome_distance(organisms[j])));
			simplices.back().add(organisms[i].id());
			simplices.back().add(organisms[j].id());
		}
	}
	std::sort(simplices.begin(), simplices.end(), DimensionValueComparison<Simplex>());
	rInfo("Average distance: %f", float(avg_distance)/
								  ((organisms.size()*organisms.size() - organisms.size())/2));

	SimplexFiltration filtration;
	for (SimplexVector::const_iterator cur = simplices.begin(); cur != simplices.end(); ++cur)
	{
		rInfo("Simplex: %s", tostring(*cur).c_str());
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
				AssertMsg(i->get_value() == 0, "Expecting only 0 birth values in 0-D diagram ");
				deaths.push_back(i->pair()->get_value());
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
