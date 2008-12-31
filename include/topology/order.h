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
template<class Chains_      = VectorChains<>,
         class Data_        = Empty>
struct ListOrderDescriptor: 
    public Chains_::template rebding<typename OrderTraits<ListOrderDescriptor<Chains_, Data_> >::Index>::other,
    public Data_
{
    typedef         ListOrderDescriptor<Chains_, Data_>                         Self;

    typedef         typename OrderTraits<Self>::Index                           OrderIndex;
    typedef         typename Chains_::template rebind<OrderIndex>::other        Chains;
    typedef         Data_                                                       Data;

    template<class OtherData_> struct                                           RebindData
    { typedef       ListOrderDescriptor<Chains_, OtherData_>                    other; };
    
    OrderIndex      pair;
};


// Specialization for ListOrderDescriptor
template<class Chains, class Data>
struct OrderTraits<ListOrderDescriptor<Chains, Data> >
{
    typedef         ListOrderDescriptor<Chains, Data>                       Descriptor;

    typedef         std::list<Descriptor>                                   Order;
    typedef         Descriptor                                              Element;
    typedef         Order::iterator                                         Index;

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
