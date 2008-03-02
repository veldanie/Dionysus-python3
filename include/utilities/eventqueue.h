#ifndef __EVENTQUEUE_H__
#define __EVENTQUEUE_H__

#include <list>
#include <functional>
#include <boost/utility.hpp>

#include <iostream>
#include <cassert>					// TODO: switch to internal debugging system
#include <string>

// TODO: change inefficient list-based implementation to something heap-based
// Need a queue that supports deleting arbitrary items (given by iterator), 
// and maintaining correctness of iterators when different operations (notably 
// remove and update are performed)

template<class Event_, class EventComparison_>
class EventQueue
{

	public:
		typedef					Event_											Event;
		typedef					EventComparison_								EventComparison;

		typedef					std::list<Event>								QueueRepresentation;
		typedef					typename QueueRepresentation::iterator			iterator;
		typedef					typename QueueRepresentation::const_iterator	const_iterator;
		
		EventQueue()			{}
		
		const_iterator 			top() const					{ assert(!empty()); return queue_.begin(); }
		iterator 				top()						{ assert(!empty()); return queue_.begin(); }
		iterator 				push(Event e)				{ queue_.push_front(e); iterator i = top(); update(i); return i; }
		void 					pop()						{ assert(!empty()); queue_.erase(queue_.begin()); }
		void					remove(iterator i)			{ queue_.erase(i); }
		void					update(iterator i);
        void                    prepend(iterator i, 
                                        EventQueue& other)  { queue_.splice(queue_.begin(), other.queue_, i); }
        ///< intended for temporary storage of elements from other queues

		iterator 				end()						{ return queue_.end(); }
		const_iterator 			end() const					{ return queue_.end(); }
		bool					empty() const				{ return queue_.empty(); }
		size_t					size() const				{ return queue_.size(); }

		std::ostream&			print(std::ostream& out, const std::string& prefix) const;

	private:
		QueueRepresentation		queue_;
};


template<class Event_, class EventComparison_>
void
EventQueue<Event_, EventComparison_>::
update(iterator i)
{
	QueueRepresentation tmp;
	tmp.splice(tmp.end(), queue_, i);
	//iterator pos = std::find_if(queue_.begin(), queue_.end(), std::not1(std::bind2nd(EventComparison(), *i)));
	iterator pos = std::find_if(queue_.begin(), queue_.end(), std::bind1st(EventComparison(), *i));
	queue_.splice(pos, tmp);
}

template<class Event_, class EventComparison_>
std::ostream&
EventQueue<Event_, EventComparison_>::
print(std::ostream& out, const std::string& prefix) const
{
	for (typename QueueRepresentation::const_iterator cur = queue_.begin(); cur != queue_.end(); ++cur)
		(*cur)->operator<<(out << prefix) << std::endl;
	return out;
}

#endif // __EVENTQUEUE_H__
