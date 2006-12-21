/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2006
 */

#ifndef __FILTRATION_H__
#define __FILTRATION_H__

#include "sys.h"
#include "debug.h"

#include "filtrationcontainer.h"
#include "filtrationsimplex.h"
#include "vineyard.h"

#include <map>
#include <vector>

#include <boost/serialization/access.hpp>

/**
 * Filtration class. Serves as an (ordered) container for the simplices, 
 * and provides pair_simplices() method that computes the RU-decomposition
 * for the simplex order stored in the filtration. Iterators remain valid 
 * through all the operations.
 */
template<class Smplx, class FltrSmplx = FiltrationSimplex<Smplx>, class Vnrd = Vineyard<FltrSmplx> >
class Filtration: public FltrSmplx::Container
{
	public:
		typedef 	Smplx															Simplex;
		typedef		FltrSmplx														FiltrationSimplex;
		typedef		Vnrd															Vineyard;
		
		/// \name Container Types
		/// @{
		/** The actual container type (which is the parent of the Filtration) */
		typedef		typename FiltrationSimplex::Container							FiltrationContainer;
		typedef		typename FiltrationContainer::Index								Index;
		typedef		typename FiltrationContainer::const_Index						const_Index;
		/// @}
		
		/// \name Cycles and Trails 
		/// @{
		typedef		typename FiltrationContainer::GreaterThanComparison				CyclesComparator;
		typedef		typename FiltrationContainer::LessThanComparison				TrailsComparator;
		typedef		typename FiltrationContainer::ConsistencyComparison 			ConsistencyComparator;
		typedef		typename FiltrationContainer::Cycle								Cycle;
		typedef		typename FiltrationContainer::Trail								Trail;
		typedef		typename Cycle::iterator										CycleIterator;
		typedef		typename Trail::iterator										TrailIterator;
		/// @}
		
		typedef		Filtration<Simplex, FiltrationSimplex, Vineyard>				Self;
		typedef		FiltrationContainer												Parent;

	public:
										Filtration(Vineyard* vineyard);
	
		/// \name Core Functionality
		/// @{
		/// Computes RU decomposition of the simplices in [bg, end) range, assuming that everything before bg has been paired 
		void 							pair_simplices(Index bg, Index end);
		bool							transpose(Index i);
		bool							is_paired() const;
		Index							append(const Simplex& s);					///< Appends s to the filtration
		Index							insert(Index prior, const Simplex& s);		///< Inserts s after prior
		const_Index						get_index(const Simplex& s) const;			/**< Returns the iterator pointing to s 
																						 (end() if s not in filtration) */
		Index							get_index(const Simplex& s);				///< \overload
		void							fill_simplex_index_map();					///< Fills the mapping for get_index()
		/// @}
		
		/// \name Accessors
		/// @{
		Vineyard*						vineyard()									{ return vineyard_; }
		const Vineyard*					vineyard() const							{ return vineyard_; }
		/// @}
	
	protected:
		using 							Parent::swap;
		bool 							transpose_simplices(Index i);				

	public:
		/// \name Container Operations
		/// @{
		using Parent::size;
		using Parent::begin;
		using Parent::end;
		/// @}
		
		std::ostream& 					operator<<(std::ostream& out) const;

	protected:
		/// \name Comparator accessors (protected)
		/// @{
		const ConsistencyComparator& 	get_consistency_cmp() const					{ return consistency_cmp; }
		const CyclesComparator& 		get_cycles_cmp() const						{ return cycles_cmp; }
		const TrailsComparator& 		get_trails_cmp() const						{ return trails_cmp; }
		/// @}

	private:
		typedef							std::map<Simplex, Index>					SimplexMap;

		/// Initializes the cycle  with the indices of the simplices in the boundary, and the trail with the index of this simplex
		void							init_cycle_trail(Index j);
		void							pairing_switch(Index i, Index j);
		
		bool 							paired;
		SimplexMap						inverse_simplices;

		Vineyard*						vineyard_;

		CyclesComparator				cycles_cmp;
		TrailsComparator				trails_cmp;
		ConsistencyComparator			consistency_cmp;

	private:
		/* Serialization */
		friend class boost::serialization::access;
										
		typedef		std::map<const_Index, SizeType, ConsistencyComparator>			IndexIntMap;
		typedef		std::vector<Index>												IndexVector;
		
		template<class Archive> void 	save(Archive& ar, version_type ) const;
		template<class Archive>	void 	load(Archive& ar, version_type );
		BOOST_SERIALIZATION_SPLIT_MEMBER()
};

#include "filtration.hpp"

#endif	// __FILTRATION_H__
