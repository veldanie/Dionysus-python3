/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2006
 */

#ifndef __FILTRATIONCONTAINER_H__
#define __FILTRATIONCONTAINER_H__

#include "utilities/consistencylist.h"
#include "cycle.h"

/**
 * FiltrationContainer class. Serves as a parent of Filtration that 
 * describes the container functionality. Used by FiltrationSimplex 
 * to get Cycle representation.
 */
template<class FltrSmplx>
class FiltrationContainer: public ConsistencyList<FltrSmplx>
{
	public:
		typedef		FltrSmplx														FiltrationSimplex;
		typedef		ConsistencyList<FiltrationSimplex>								ConsistencyLst;
		
		/// \name Cycles and Trails 
		/// @{
		/// Index is and therfore acts like an iterator. The name is preserved for historical reasons.
		typedef		typename ConsistencyLst::iterator								Index;
		/// const_Index is a const_iterator
		typedef		typename ConsistencyLst::const_iterator						    const_Index;
		/// @}

		/// \name Cycles and Trails 
		/// @{
		typedef		typename ConsistencyLst::GreaterThanComparison					CyclesComparator;
		typedef		typename ConsistencyLst::LessThanComparison					    TrailsComparator;
		typedef		typename ConsistencyLst::ConsistencyComparison 				    ConsistencyComparator;
		typedef		::Cycle<Index, CyclesComparator, ConsistencyComparator>			Cycle;
		typedef		::Cycle<Index, TrailsComparator, ConsistencyComparator>			Trail;
		/// @}

		template<class U>
		struct rebind { typedef FiltrationContainer<U> other; };
};

#endif // __FILTRATIONCONTAINER_H__
