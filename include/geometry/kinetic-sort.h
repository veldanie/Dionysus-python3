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
 *  \arg SortDS_              should be forward and backward iterable, swaps are handles via SwapCallback
 *  \arg TrajectoryExtractor_ applied to the iterator into SortDS_ should return a rational 
 *                            function describing the 
 *  \arg Simulator_           the Simulator type, e.g. Simulator. Note that KineticSort does not store 
 *                            a pointer to the Simulator (so a pointer is passed in each relevant operation)
 */
template<class SortDS_, class TrajectoryExtractor_, class Simulator_>
class KineticSort
{
	public:
		typedef						Simulator_									Simulator;
		typedef						typename Simulator::PolynomialKernel		PolynomialKernel;
		typedef						SortDS_										SortDS;
		typedef						TrajectoryExtractor_						TrajectoryExtractor;
		
		typedef						typename Simulator::Key						SimulatorKey;
		typedef						typename SortDS::iterator					SortDSIterator;

									
	private:
		/* Implementation */
		struct Node
		{
			SortDSIterator			element;
			SimulatorKey			swap_event_key;		

									Node(SortDSIterator e, SimulatorKey k): 
										element(e), swap_event_key(k)			{}
		};

		typedef						std::list<Node>								NodeList;

	public:
		typedef						typename NodeList::iterator					iterator;
		typedef						boost::function<void(SortDS*, SortDSIterator pos)>	
																				SwapCallback;


		/// \name Core Functionality
		/// @{
									KineticSort(SortDS* sort, Simulator* simulator, SwapCallback swap_callback);

		template<class InputIterator>
		void						insert(iterator pos, InputIterator f, InputIterator l, Simulator* simulator);
		void						erase(iterator pos, Simulator* simulator);
		void						update_trajectory(iterator pos, Simulator* simulator);

		void						swap(iterator pos, Simulator* simulator);

		bool						audit(Simulator* simulator) const;
		/// @}

	private:
		class SwapEvent;
		void						schedule_swaps(iterator b, iterator e, Simulator* s);
		void						schedule_swaps(iterator i, Simulator* s);

	private:
		NodeList					list_;
		SortDS*						sort_;			
		SwapCallback				swap_callback_;	
};

#include "kinetic-sort.hpp"

#endif // __KINETIC_SORT_H__
