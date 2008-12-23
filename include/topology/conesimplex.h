/**
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2007
 */

#ifndef __CONESIMPLEX_H__
#define __CONESIMPLEX_H__

#include <list>
#include <iostream>

#include "utilities/types.h"


template<class S>
class ConeSimplex: public S
{
	public:
		typedef		S													Parent;
		typedef		ConeSimplex<S>										Self;
		typedef		std::list<Self>										Cycle;

    public:
								ConeSimplex(const Self& s): 
                                    Parent(s), coned_(s.coned_)         {}
								ConeSimplex(const Parent& parent, 
											bool coned = false):
									Parent(parent), coned_(coned)		{}
	    
		Cycle					boundary() const;
		bool					coned() const							{ return coned_; }
        Dimension               dimension() const                       { return coned_ ? (Parent::dimension() + 1) : Parent::dimension(); }
        
        bool                    operator<(const Self& other) const      { if (coned_ ^ other.coned_) return !coned_; else return Parent::operator<(other); }
        bool                    operator==(const Self& other) const     { return !(coned_ ^ other.coned_) && Parent::operator==(other); }

		std::ostream& 			operator<<(std::ostream& out) const;
		
	private:
		bool					coned_;
};

template<class S>
std::ostream& 		operator<<(std::ostream& out, const ConeSimplex<S>& s)	{ return s.operator<<(out); }

#include "conesimplex.hpp"

#endif // __CONESIMPLEX_H__
