/**
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2007
 */

#ifndef __AVIDA_POPULATION_DETAIL_H__
#define __AVIDA_POPULATION_DETAIL_H__

#include <string>
#include <vector>

/**
 * Stores organism details stored in a single line of a population detail file in data/ 
 * directory of Avida's output.
 */
class AvidaOrganismDetail
{
	public:
		typedef			int							IDType;
		typedef			unsigned int				DistanceType;
		typedef			unsigned int				CountType;

						AvidaOrganismDetail(std::string line);

		DistanceType 	genome_distance(const AvidaOrganismDetail& other) const;
		
		IDType			id() const									{ return id_; }
		float			fitness() const								{ return fitness_; }
		CountType		length() const								{ return genome_length_; }
		std::string		genome() const								{ return genome_; }

	private:
		IDType			id_, parent_id_;
		int				parent_distance_;
		CountType		num_organisms_alive_, num_organisms_ever_;
		CountType		genome_length_;
		float 			merit_, gestation_time_;
		float 			fitness_;
		int 			update_born_, update_deactivated_, depth_phylogenetic_tree_;
		std::string 	genome_;
};

/**
 * Stores entire population details (all organisms in a given time step), i.e., stores
 * an entire population detail file in data/ directory of Avida's ouptut.
 */
class AvidaPopulationDetail
{
	public:
		typedef			std::vector<AvidaOrganismDetail>				OrganismVector;
						AvidaPopulationDetail(std::string filename);

		const OrganismVector& get_organisms() const						{ return organisms_; }

	private:
		OrganismVector	organisms_;
};


#include "avida-population-detail.hpp"

#endif //__AVIDA_POPULATION_DETAIL_H__
