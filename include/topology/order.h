#ifndef __ORDER_H__
#define __ORDER_H__

#include "cycles.h"
#include "utilities/types.h"
#include "utilities/indirect.h"

#include <vector>
#include <list>

//#include <iostream>
#include <sstream>
#include <string>

template<class OrderDescriptor>
struct OrderTraits {};

/** 
 * Class: VectorOrderDescriptor
 * Class that stores a dense order of simplices in a vector. 
 * That prevents it from performing efficient insertions.
 */
template<class Chains_ =    VectorChains<>,
         class Data_ =      Empty<> >
struct VectorOrderDescriptor: 
    public Chains_::template rebind<typename OrderTraits<VectorOrderDescriptor<Chains_, Data_> >::Index>::other,
    public Data_
{
    typedef         VectorOrderDescriptor<Chains_, Data_>                       Self;
    
    typedef         typename OrderTraits<Self>::Index                           OrderIndex;
    typedef         typename Chains_::template rebind<OrderIndex>::other        Chains;
    typedef         Data_                                                       Data;

    template<class OtherData_> struct                                           RebindData
    { typedef       VectorOrderDescriptor<Chains_, OtherData_>                  other; };

  
    // TODO: Do we need any of these constructors?
    VectorOrderDescriptor(const Data& d = Data()):
        Data(d)                                                                 {}
    VectorOrderDescriptor(typename Chains::Cycle z, const Data& d = Data()):
        Chains(z), Data(d)                                                      {}
    VectorOrderDescriptor(OrderIndex i, const Data& d = Data()):
        Data(d), pair(i)                                                        {}
    VectorOrderDescriptor(OrderIndex i, typename Chains::Cycle z, const Data& d = Data()):
        Chains(z), Data(d), pair(i)                                             {}

    OrderIndex      pair;
};

// Specialization for VectorOrderDescriptor
template<class Chains, class Data>
struct OrderTraits<VectorOrderDescriptor<Chains, Data> >
{
    typedef         VectorOrderDescriptor<Chains, Data>                         Descriptor;

    typedef         std::vector<Descriptor>                                     Order;
    typedef         Descriptor                                                  Element;
    typedef         typename Order::iterator                                    Index;
    typedef         ThreeOutcomeCompare<std::greater<Index> >                   Comparison;

    class OutputMap
    {
        public:
                                OutputMap(const Order& order):
                                    bg_(order.begin())                          {}

            // returns a string with (i - bg_)                                
            std::string         operator()(Index i) const                       
            { 
                std::stringstream s; s << (i - bg_);
                return  s.str();
            }

        private:
            typename Order::const_iterator          bg_;
    };
};

#if 0
template<class StoragePolicy_ = ListRV<> >
struct ListOrderDescriptor: public StoragePolicy_::template RebindOrder<typename std::list<ListOrderDescriptor<StoragePolicy_> >::iterator>::other
{
    typedef         ListOrderDescriptor<StoragePolicy_>                         Self;
    typedef         typename std::list<Self>::iterator                          OrderIndex;
    OrderIndex      pair;
    
    typedef         typename StoragePolicy_::template RebindOrder<OrderIndex>::other StoragePolicy;
    typedef         typename StoragePolicy::ComplexIndex                        ComplexIndex;

                    ListOrderDescriptor(ComplexIndex i): 
                        StoragePolicy(i)                                        {}

    // Acts as a rebind
    template<class OtherStoragePolicy_> struct                                  Order
    { typedef       std::list<ListOrderDescriptor<OtherStoragePolicy_> >        type; };
};


template<class T>
struct ConsistencyCmp
{
    int             compare(T a, T b) const                                 { if (a < b) return -1; if (a == b) return 0; return 1; }
    bool            operator()(T a, T b) const                              { return compare(a,b) == -1; }
};

// Traits
template<class Order_>
struct OrderTraits
{
    typedef         Order_                                                  Order;
    typedef         void                                                    StoragePolicy;
    typedef         void                                                    Index;
    typedef         void                                                    Element;
};

// Specialization
template<class T>
struct OrderTraits<std::vector<T> >
{
    typedef         std::vector<T>                                          Order;
    typedef         typename T::StoragePolicy                               StoragePolicy;
    typedef         typename T::OrderIndex                                  Index;
    typedef         T                                                       Element;

    typedef         std::less<Index>                                        OrderComparison;
    typedef         ConsistencyCmp<Index>                                   ConsistencyComparison;      

    static Index    begin(Order& order)                                     { return order.begin(); }
    static Index    end(Order& order)                                       { return order.end(); }

    template<class Comparison>
    static void     sort(Order& order, const Comparison& cmp)               { std::sort(order.begin(), order.end(), cmp); }
};

template<class T>
struct OrderTraits<std::list<T> >
{
    typedef         std::list<T>                                            Order;
    typedef         typename T::StoragePolicy                               StoragePolicy;
    typedef         typename T::OrderIndex                                  Index;
    typedef         T                                                       Element;

    // FIXME
    typedef         std::less<Index>                                        OrderComparison;
    typedef         ConsistencyCmp<Index>                                   ConsistencyComparison;      

    static Index    begin(Order& order)                                     { return order.begin(); }
    static Index    end(Order& order)                                       { return order.end(); }
    
    template<class Comparison>
    static void     sort(Order& order, const Comparison& cmp)               { order.sort(cmp); }
};
#endif

#endif // __ORDER_H__
