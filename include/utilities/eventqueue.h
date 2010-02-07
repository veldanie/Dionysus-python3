#ifndef __EVENTQUEUE_H__
#define __EVENTQUEUE_H__

#include <list>
#include <functional>
#include <boost/utility.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
namespace bmi = boost::multi_index;

#include <utilities/log.h>
#ifdef LOGGING
static rlog::RLogChannel* rlEventQueue =             DEF_CHANNEL("utilities/eventqueue", rlog::Log_Debug);
#endif // LOGGING

#include <iostream>
#include <string>
#include <algorithm>

template<class Event_, class EventComparison_>
class EventQueue
{

	public:
		typedef					Event_											Event;
		typedef					EventComparison_								EventComparison;

        // Not actually a heap, but will work for now
        typedef                 boost::multi_index_container<Event, 
                                                             bmi::indexed_by<
                                                                bmi::sequenced<>,
                                                                bmi::ordered_non_unique<bmi::identity<Event>, EventComparison >
                                                             > >
                                                                                QueueRepresentation;
		typedef					typename QueueRepresentation::iterator			iterator;
		typedef					typename QueueRepresentation::const_iterator	const_iterator;
		
		EventQueue()			{}
		
		const_iterator 			top() const					{ AssertMsg(!empty(), "Queue must not be empty"); return queue_.begin(); }
		iterator 				top()						{ AssertMsg(!empty(), "Queue must not be empty"); return queue_.begin(); }
		// iterator 				push(Event e)				{ return queue_.insert(insert_position(e), e).first; }
		iterator 				push(Event e)				{ iterator i = queue_.push_front(e).first; update(i); return i; }
		void 					pop()						{ AssertMsg(!empty(), "Queue must not be empty"); queue_.erase(queue_.begin()); }
		void					remove(iterator i)			{ queue_.erase(i); }
        void                    replace(iterator i,
                                        Event e)            { queue_.replace(i, e); update(i); }

        iterator                begin()                     { return queue_.begin(); }
        const_iterator          begin() const               { return queue_.begin(); }
		iterator 				end()						{ return queue_.end(); }
		const_iterator 			end() const					{ return queue_.end(); }
		bool					empty() const				{ return queue_.empty(); }
		size_t					size() const				{ return queue_.size(); }

		std::ostream&			print(std::ostream& out, const std::string& prefix) const;

        iterator                update(iterator i)
        { 
            iterator bg = bmi::project<0>(queue_, bmi::get<1>(queue_).lower_bound(*i)); 
            iterator pos = std::find_if(bg, queue_.end(), std::bind1st(EventComparison(), *i));
            queue_.replace(i, *i);
            queue_.relocate(pos, i);
            return pos; 
        }

	private:
		QueueRepresentation		queue_;
};


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
