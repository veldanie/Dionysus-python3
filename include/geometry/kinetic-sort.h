#ifndef __KINETIC_SORT_H__
#define __KINETIC_SORT_H__

#include <list>
#include <boost/function.hpp>
#include <boost/utility.hpp>
#include <iostream>

/**
 * Maintains elements of the given data structure in the sorted order assuming the elements follow 
 * trajectories given by TrajectoryExtractor_.
 *
 *  \arg ElementIterator_     iterator over the underlying data structure that's kept in sorted order
 *  \arg TrajectoryExtractor_ applied to the iterator into SortDS_ should return a rational 
 *                            function describing the 
 *  \arg Simulator_           the Simulator type, e.g. Simulator. Note that KineticSort does not store 
 *                            a pointer to the Simulator (so a pointer is passed in each relevant operation)
 *  \arg Swap_                is called with an ElementIterator_ when a swap needs to be performed
 */
template<class ElementIterator_, class TrajectoryExtractor_, 
		 class Simulator_, class Swap_ = boost::function<void(ElementIterator_ pos)>>
class KineticSort
{
	public:
		typedef						Simulator_									Simulator;
		typedef						typename Simulator::PolynomialKernel		PolynomialKernel;
		typedef						ElementIterator_							ElementIterator;
		typedef						Swap_										Swap;
		typedef						TrajectoryExtractor_						TrajectoryExtractor;
		
		typedef						typename Simulator::Key						SimulatorKey;

									
	private:
		/* Implementation */
		struct Node
		{
			ElementIterator			element;
			SimulatorKey			swap_event_key;		

									Node(ElementIterator e, SimulatorKey k): 
										element(e), swap_event_key(k)			{}
		};

		typedef						std::list<Node>								NodeList;

	public:
		typedef						typename NodeList::iterator					iterator;


		/// \name Core Functionality
		/// @{
									KineticSort(Swap swap);
									KineticSort(ElementIterator b, ElementIterator e, Swap swap, Simulator* simulator);

		void						insert(iterator pos, ElementIterator f, ElementIterator l, Simulator* simulator);
		void						erase(iterator pos, Simulator* simulator);
		void						update_trajectory(iterator pos, Simulator* simulator);

		void						swap(iterator pos, Simulator* simulator);

		bool						audit(Simulator* simulator) const;

		iterator					begin() 									{ return list_.begin(); }
		iterator					end() 										{ return list_.end(); }

		void						initialize(ElementIterator b, ElementIterator e, Simulator* simulator);
		/// @}

	private:
		class SwapEvent;
		void						schedule_swaps(iterator b, iterator e, Simulator* s);
		void						schedule_swaps(iterator i, Simulator* s);

	private:
		NodeList					list_;
		Swap						swap_;	
};

#include "kinetic-sort.hpp"

#endif // __KINETIC_SORT_H__
