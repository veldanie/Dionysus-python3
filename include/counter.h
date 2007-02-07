/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2006
 */

#ifndef __COUNTER_H__
#define __COUNTER_H__

/* This should be integrated with the logging facilities. Written in the same framework. */

#include <map>
#include <string>
#include <iostream>

#ifndef COUNTERS
#define Count(x)
#define CountNum(x,y)
#else // COUNTERS
#define Count(x) counters.inc(x)
#define CountNum(x,y) counters.inc(x,y)
#endif

typedef 		unsigned long 		CounterType;
typedef			std::string			KeyType;

class CounterFactory
{
	private:
		typedef				std::map<int, CounterType> 			CountersMap;
		typedef				std::map<KeyType, CountersMap>		KeyMap;
		KeyMap				ctrs;
		
	public:
		~CounterFactory()
		{
#ifdef COUNTERS
			std::cout << "Counters:" << std::endl;
			for (KeyMap::const_iterator cur = ctrs.begin(); cur != ctrs.end(); ++cur)
			{
				std::cout << cur->first << ": ";
				if (cur->second.size() == 1)
				{
					std::cout << cur->second.begin()->second << std::endl;
					continue;
				}
				std::cout << std::endl;
				for (CountersMap::const_iterator ccur = cur->second.begin();
														  ccur != cur->second.end();
														  ++ccur)
					std::cout << "  " << ccur->first << ": " << ccur->second << std::endl;
			}
#endif // COUNTERS
		}

		void inc(const KeyType& key, int num = 0)
		{
			ctrs[key][num]++;
		}

		CounterType lookup(const KeyType& key, int num = 0) const
		{
			return const_cast<KeyMap&>(ctrs)[key][num];
		}
};

static CounterFactory counters;

#endif // __COUNTER_H__
