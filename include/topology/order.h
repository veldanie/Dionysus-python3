#ifndef __ORDER_H__
#define __ORDER_H__

#include "utilities/types.h"
#include "utilities/indirect.h"

#include <vector>

//#include <iostream>
#include <sstream>
#include <string>


template<class Element_ = Empty<> >
struct VectorContainer
{
    typedef         Element_                                                    Element;
    typedef         std::vector<Element>                                        Container;

    typedef         typename Container::iterator                                Index;

    class OutputMap
    {
        public:
                                OutputMap(const Container& order):
                                    bg_(order.begin())                          {}

            // returns a string with (i - bg_)                                
            std::string         operator()(Index i) const                       
            { 
                std::stringstream s; s << (i - bg_);
                return  s.str();
            }

        private:
            typename Container::const_iterator          bg_;
    };

    template<class U> struct rebind
    { typedef           VectorContainer<U>              other; };
};

template<class Index = int>
struct GreaterComparison: public std::greater<Index>
{
    template<class U> struct rebind
    { typedef           GreaterComparison<U>            other; };
};

#endif // __ORDER_H__
