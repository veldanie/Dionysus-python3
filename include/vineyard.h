/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2006
 */

#ifndef __VINEYARD_H__
#define __VINEYARD_H__

#include "filtration.h"
#include "vineyardsimplex.h"
#include "types.h"
#include <list>

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/is_abstract.hpp>
	

template<class S> class VineyardSimplex;

/**
 * Vineyard class. Provides transpose() function, and keeps track of vines and knees in the process.
 * Derives from Filtration, which represents the current state of the filtration (after the performed 
 * transpositions).
 */
template<class Smplx,
		 class Fltr = Filtration<Smplx>, 
		 class VnrdSmplx = VineyardSimplex<Smplx> >
class Vineyard: public Fltr::template rebind<VnrdSmplx>::other
{
	public:
		typedef							Smplx											Simplex;
		typedef							VnrdSmplx										VineyardSimplex;
		typedef				   typename Fltr::template rebind<VineyardSimplex>::other	Filtration;
		
		/// \name Vineyard types
		/// @{
		typedef				   typename VineyardSimplex::Knee							Knee;
		typedef				   typename VineyardSimplex::Vine							Vine;
		/// @}

		/// \name Container  Types
		/// @{
		typedef				   typename Filtration::Index								Index;
		typedef				   typename Filtration::const_Index							const_Index;
		/// @}
		
		/// \name Cycles and Trails 
		/// @{
		typedef				   typename Filtration::CyclesComparator					CyclesComparator;
		typedef				   typename Filtration::Cycle								Cycle;
		typedef				   typename Filtration::TrailIterator						TrailIterator;
		/// @}

										
	public:
										Vineyard();
										Vineyard(const Vineyard& vineyard); 				///< Copy-constructor
		template<class OtherFltrSmplx>	Vineyard(const typename Filtration::template rebind<OtherFltrSmplx>::other& filtration);
		///< Pseudo from-base constructor
		
		/// \name Core functionality
		/// @{
		void							pair_simplices(Index bg);
		void							record_frame(Index bg);
		bool							transpose(Index i);
		/// @}

		using Filtration::is_paired;
		using Filtration::begin;
		using Filtration::end;
		using Filtration::size;
		
	protected:
		bool 							transpose_simplices(Index i);				
		typename Knee::SimplexList  	resolve_cycle(Index i) const;
							
	private:
		void							start_vines(Index bg);
		void							pairing_switch(Index i, Index j);
};

#include "vineyard.hpp"

#endif // __VINEYARD_H__
