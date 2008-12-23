#ifndef __CONTAINERS_H__
#define __CONTAINERS_H__

#include <vector>
#include "circular_list.h"

// TODO: write documentation

template<class Container_, class Comparison_ = std::less<typename Container_::value_type> >
struct ContainerTraits
{
    typedef     Container_                                                                  Container;
    typedef     typename Container::value_type                                              Item;
    typedef     Comparison_                                                                 Comparison;

    static void reserve(Container& c, size_t sz)                                            {}
    static void sort(Container& c, const Comparison& cmp = Comparison())                    { c.sort(cmp); }
};

/**
 * Class: SizeStorage
 *
 * This class expresses how size should be stored for various containers to be able 
 * to deduce it in constant time. By default it is stored explicitly, so that if the 
 * Container is std::list everything works. However, specialization is available for 
 * std::vector which uses its builtin size() function.
 */
template<class Container_>
class SizeStorage
{
    public:
        typedef         Container_                                                          Container;

                        SizeStorage(): size_(0)                                             {}

        void            increment(size_t inc = 1)                                           { size_ += inc; }
        void            decrement(size_t dec = 1)                                           { size_ -= dec; }
        size_t          size(const Container& c) const                                      { return size_; }
        void            swap(SizeStorage& other)                                            { std::swap(size_, other.size_); }

    private:
        size_t          size_;
};


/* Specializations */

template<class T, class Comparison_>
struct ContainerTraits<std::vector<T>, Comparison_>
{
    typedef     T                                                                           Item;
    typedef     std::vector<T>                                                              Container;
    typedef     Comparison_                                                                 Comparison;

    static void reserve(Container& c, size_t sz)                                            { c.reserve(sz); }
    static void sort(Container& c, const Comparison& cmp = Comparison())                    { std::sort(c.begin(), c.end(), cmp); }
};

template<class T, class Comparison_>
struct ContainerTraits<List<T>, Comparison_>
{
    typedef     T                                                                           Item;
    typedef     List<T>                                                                     Container;
    typedef     Comparison_                                                                 Comparison;

    static void reserve(Container& c, size_t sz)                                            { }
    static void sort(Container& c, const Comparison& cmp = Comparison())                    
    { 
    	std::vector<Item> tmp(c.begin(), c.end());
	    std::sort(tmp.begin(), tmp.end(), cmp);
    	std::copy(tmp.begin(), tmp.end(), c.begin());
    }
};

// TODO: specialize for List (singly-linked list)

template<class T>
class SizeStorage<std::vector<T> >
{
    public:
        typedef         std::vector<T>                                                      Container;

        void            increment(size_t inc = 1)                                           {}
        void            decrement(size_t dec = 1)                                           {}
        size_t          size(const Container& c) const                                      { return c.size(); }
        void            swap(SizeStorage& other)                                            {}
};

#endif // __CONTAINERS_H__
