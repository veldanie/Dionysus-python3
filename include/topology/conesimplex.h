/**
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2007
 */

#ifndef __CONESIMPLEX_H__
#define __CONESIMPLEX_H__

#include <list>
#include <iostream>

template<class S>
class ConeSimplex: public S
{
	public:
		typedef		S													Parent;
		typedef		ConeSimplex<S>										Self;
		typedef		std::list<Self>										Cycle;

    public:
								ConeSimplex(const Parent& parent, 
											bool coned = false):
									Parent(parent), coned_(coned)		{}
	    
		Cycle					boundary() const;
		bool					coned() const							{ return coned_; }

		std::ostream& 			operator<<(std::ostream& out) const;
		
	private:
		bool					coned_;
};

template<class S>
std::ostream& 		operator<<(std::ostream& out, const ConeSimplex<S>& s)	{ return s.operator<<(out); }

#include "conesimplex.hpp"

#endif // __CONESIMPLEX_H__
