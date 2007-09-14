/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2007
 */

#ifndef __COUNTER_H__
#define __COUNTER_H__


#ifndef COUNTERS
	#define 	GetCounter(path) 		0
	#define 	Count(x)
	#define		CountNum(x,y)
	#define		SetFrequency(x, freq)
	#define		SetTrigger(x, y)
#else // COUNTERS
	#define 	GetCounter(path) 			get_counter(path)
	#define 	Count(x) 					do { x->count++; if ((x->count % x->frequency == 0)) x->trigger->print(); } while (0)
	#define 	CountNum(x,y) 				do { x->subcount[y]++; } while (0)
	#define		SetFrequency(x, freq)		do { x->frequency = freq; } while (0)
	#define		SetTrigger(x, y)			do { x->trigger = y; } while(0)
#endif // COUNTERS


#include <map>
#include <string>
#include <iostream>
#include <limits>

class Counter
{
	public:
		typedef 				unsigned long 							CounterType;
		typedef					std::map<std::string, Counter*>			SubCounterMap;
		typedef					std::map<int, CounterType>				SubCountMap;

	public:
		CounterType				count;
		CounterType				frequency;
		SubCountMap				subcount;
		Counter*				trigger;

	public:
								Counter(const std::string& full_name = "",
										CounterType freq = std::numeric_limits<CounterType>::max()):
										count(0), frequency(freq), trigger(this), full_name_(full_name)
								{}
								~Counter();

		Counter*				get_child(const std::string& path, std::string::size_type pos);
		void					print();

	private:	
		SubCounterMap			subcounters_;
		std::string				full_name_;
};

static		Counter				rootCounter;

Counter*	get_counter(const char* path)
{
	return rootCounter.get_child(path, 0);
}

#include "counter.hpp"


#endif // __COUNTER_H__
