/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2006
 */

#ifndef __EVENTQUEUE_H__
#define __EVENTQUEUE_H__

#include <map>

template<class _Key, class _Event>
class EventQueue
{

	public:
		typedef					_Key											Key;
		typedef					_Event											Event;

		// multimap just in case
		typedef					std::multimap<Key,Event>						QueueRepresentation;
		typedef					typename QueueRepresentation::iterator			iterator;
		typedef					typename QueueRepresentation::const_iterator	const_iterator;
		typedef					std::pair<Key, Event>							KeyValuePair;
		
		EventQueue()															{}
		
		const_iterator 			top() 	const									{ assert(!empty()); return queue.begin(); }
		iterator 				push(Key k, Event e)							{ return queue.insert(std::make_pair(k,e)); }
		void 					pop()											{ assert(!empty()); queue.erase(queue.begin()); }
		void					remove(iterator i)								{ queue.erase(i); }

		iterator 				end()											{ return queue.end(); }
		const_iterator 			end() const										{ return queue.end(); }
		bool					empty() const									{ return queue.empty(); }
		size_t					size() const									{ return queue.size(); }

	private:
		QueueRepresentation		queue;
};

#endif // __EVENTQUEUE_H__
