#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include "utilities/eventqueue.h"
#include "polynomial.h"

template<class Comparison>  						class IndirectComparison;
template<class PolyKernel_, class Simulator_>		class Event;

template<class PolyKernel_, template<class Event> class EventComparison_ = std::less>
class Simulator
{
	public:
		typedef						PolyKernel_									PolynomialKernel;
		typedef						typename PolynomialKernel::Polynomial		Polynomial;
		typedef						typename PolynomialKernel::RationalFunction	RationalFunction;
		typedef						typename PolynomialKernel::RootStack		RootStack;
		typedef						typename PolynomialKernel::RootType			RootType;
		typedef						RootType									Time;

		class Event;
		typedef						EventComparison_<Event>						EventComparison;
		typedef						EventQueue<Event*, IndirectComparison<EventComparison> >			
																				EventQueue;
		typedef						typename EventQueue::iterator				Key;
		typedef						typename EventQueue::const_iterator			const_Key;


									Simulator(Time start = PolynomialKernel::root(0)):
										current_(start), 
										reached_infinity_(false)				{}


		template<class Event_> 
		Key							add(const Event_& e);
		template<class Event_> 
		Key							add(const RationalFunction& f, const Event_& e);
		void						process();
		void						update(Key k, const RationalFunction& f);
		
		void						remove(Key k)								{ queue_.remove(k); }
		Key							null_key() 									{ return queue_.end(); }

		Time						current_time() const						{ return current_; }
		Time						audit_time() const;
		bool						reached_infinity() const					{ return reached_infinity_; }

		std::ostream&				print(std::ostream& out) const;

	private:
		void						update(Key i);

	private:
		Time						current_;
		EventQueue					queue_;
		bool						reached_infinity_;
};

template<class PolyKernel_, template<class Event> class EventComparison_>
class Simulator<PolyKernel_, EventComparison_>::Event
{
	public:
		typedef						PolyKernel_									PolynomialKernel;
		typedef						typename PolynomialKernel::RootStack		RootStack;

		/// Returns true if the event needs to remain in the Simulator 
		/// (top of the root_stack() will be used for new time)
		virtual	bool				process(Simulator* s) const					=0;
		
		RootStack&					root_stack()								{ return root_stack_; }
		const RootStack&			root_stack() const							{ return root_stack_; }

		bool						operator<(const Event& e) const				
		{ 
			if (root_stack().empty())
				return false;
			else if (e.root_stack().empty())
				return true;
			else
				return root_stack().top() < e.root_stack().top();
		}

		int							sign_before() const							{ return root_stack().top().sign_low(); }
		int							sign_after() const							{ return root_stack().top().sign_high(); }

		virtual std::ostream&		print(std::ostream& out) const				{ return out << "Event with " << root_stack_.size() << " roots"; }

	private:
		RootStack					root_stack_;
};

template<class Comparison_>
class IndirectComparison: public std::binary_function<const typename Comparison_::first_argument_type*, 
													  const typename Comparison_::second_argument_type*, 
													  bool>
{
	public:
		typedef						Comparison_											Comparison;
		typedef						const typename Comparison::first_argument_type*		first_argument_type;
		typedef						const typename Comparison::second_argument_type*	second_argument_type;

		bool						operator()(first_argument_type e1, second_argument_type e2) const
		{ return Comparison()(*e1, *e2); }
};

#include "simulator.hpp"

#endif // __SIMULATOR_H__
