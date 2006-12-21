/**
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005-2006
 */

#ifndef __CYCLE_H__
#define __CYCLE_H__

#include "sys.h"
#include "debug.h"

#include "types.h"
#include "circular_list.h"
#include <list>
#include <boost/serialization/access.hpp>

/**
 * Class storing a cycle of simplices. Stores items in the order defined by ConsistencyCmp. 
 * The actual order of the elements is defined by OrderCmp. Instances of those 
 * classes are not stored in Cycle for efficiency, and are passed as arguments to those methods 
 * that require them.
 */
template <class Itm, class OrderCmp, class ConsistencyCmp = OrderCmp>
class Cycle: public List<Itm>
{
	public:
		/// \name Template Parameters
		/// @{
		typedef			Itm												Item;
		typedef			OrderCmp										OrderComparison;
		typedef			ConsistencyCmp									ConsistencyComparison;
		/// @}
		
		typedef 		Cycle<Item, OrderComparison, ConsistencyCmp>	Self;
		typedef			List<Item> 										CycleRepresentation; 

		/// \name Accessor typedefs
		/// @{
		typedef			typename CycleRepresentation::iterator			iterator; 
		typedef			typename CycleRepresentation::const_iterator	const_iterator; 
		typedef			typename CycleRepresentation::const_reference	const_reference; 
		typedef			typename CycleRepresentation::reference			reference; 
		typedef			typename CycleRepresentation::pointer			pointer; 
		typedef			typename CycleRepresentation::const_pointer		const_pointer; 
		/// @}
		
	public:		
						Cycle();
						Cycle(const Cycle& c);
		
		/// \name Whole Cycle operations
		/// @{
		/** Add c to *this assuming both Cycles are sorted in increasing order according to cmp. */
		Self&			add(const Self& c, const ConsistencyComparison& cmp);
		void			swap(Cycle& c); 								///< Swaps the contents of c and *this (like STL's swap destroys c)
		//void 			insertion_sort(const Comparison& cmp); 			///< Sort list[i] using insertion sort
		void			sort(const ConsistencyComparison& cmp);			///< Sort elements to enforce consistency
		using 			CycleRepresentation::empty;
		using 			CycleRepresentation::clear;
		using			CycleRepresentation::size;
		/// @}
		
		/// \name Modifiers
		/// @{
		using 			CycleRepresentation::erase;
		void			append(const_reference x, const ConsistencyComparison& cmp);
		/// @}
		
		/// \name Accessors
		/// @{
		using 			CycleRepresentation::begin;
		using 			CycleRepresentation::end;
		const_reference	top(const OrderComparison& cmp) const;			///< First element in cmp order
		iterator		get_second(const OrderComparison& cmp) const;	///< Second element in cmp order
		/// @}

		/// \name Block access optimizations
		// Between operations used in optimization of transpositions for regular vertices. Maybe we don't need these? TODO
		/// @{
		/** Return first after i, but before or equal j; return i if no such element found */
		const_reference	first_between(const_reference i, const_reference j, const OrderComparison& cmp);
		/// Add lists and remove all elements after i and before or equal to j
		const_reference	add_and_first_between(const Self& c, const ConsistencyComparison& consistency_cmp,
											  const_reference i, const_reference j, const OrderComparison& order_cmp);
		/// Erase everything after i, but before or equal to j
		void 			erase_between(const_reference i, const_reference j, const OrderComparison& cmp);
		/// @}
	
		/// \name Debugging
		/// @{
		const_reference get_first(const OrderComparison& cmp) const;	///< First element in cmp order
		std::ostream&	operator<<(std::ostream& out) const;
		/// @}
		
	private:
		typedef			std::list<Item>									TemporaryCycleRepresenation;
		
		using 			CycleRepresentation::push_back;
		using 			CycleRepresentation::insert;

	private:
		size_t sz;
		
	private:
		// Serialization
		typedef			List<Item>										Parent;
		friend class 	boost::serialization::access;
		template<class Archive> 
		void			serialize(Archive& ar, version_type );
};

#include "cycle.hpp"

#endif // __CYCLE_H__

