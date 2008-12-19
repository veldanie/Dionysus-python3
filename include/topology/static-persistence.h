#ifndef __STATIC_PERSISTENCE_H__
#define __STATIC_PERSISTENCE_H__

#include "order.h"
#include "filtration.h"

/**
 * Class: StaticPersistence
 * The class that encapsulates order and pairing information as well as 
 * implements methods to compute and maintain the pairing. Its most 
 * significant method is <pair_simplices()>.
 *
 * Template parameters:
 *   Data_ -                auxilliary contents to store with each OrderElement
 *   OrderDescriptor_ -     class describing how the order is stored; it defaults to <VectorOrderDescriptor> 
 *                          which serves as a prototypical class
 */
template<class Data_ = Empty,
         class OrderDescriptor_ =   VectorOrderDescriptor<> >
class StaticPersistence
{
    public:
        // Typedef: Data
        // The data type stored in each order element
        typedef                         Data_                                                   Data;

        /* Typedefs: Order
         * Types associated with the order
         *
         * OrderDescriptor -        template parameter rebound with `Data` carrying all the necessary information
         * Order -                  the container class itself
         * OrderIndex -             iterator into the Order
         * OrderComparison -        a class for comparing OrderIndices
         */
        typedef                         typename OrderDescriptor_::
                                                 template RebindData<Data>::
                                                 other                                          OrderDescriptor;
        typedef                         OrderTraits<OrderDescriptor>                            Traits;
        typedef                         typename Traits::Order                                  Order;
        typedef                         typename Traits::Index                                  OrderIndex;
        typedef                         typename Traits::Element                                OrderElement;
        typedef                         typename Traits::Comparison                             OrderComparison;

        /* Constructor: StaticPersistence()
         * TODO: write a description
         *
         * Template parameters:
         *   Filtration -           filtration of the complex whose persistence we are computing
         */
        template<class Filtration>      StaticPersistence(const Filtration& f, 
                                                          const OrderComparison& ocmp = OrderComparison());
        
        // Function: pair_simplices()                                        
        // Compute persistence of the filtration
        void                            pair_simplices()                                        { pair_simplices<PairVisitor>(begin(), end()); }

        // Functions: Accessors
        //   begin() -              returns OrderIndex of the first element
        //   end() -                returns OrderIndex of one past the last element
        //   size() -               number of elements in the StaticPersistence
        OrderIndex                      begin()                                                 { return order_.begin(); }
        OrderIndex                      end()                                                   { return order_.end(); }
        size_t                          size() const                                            { return order_.size(); }
        const OrderComparison&          order_comparison() const                                { return ocmp_; }

    protected:
        // Function: pair_simplices(bg, end)
        // Compute persistence of the simplices in filtration between bg and end
        template<class Visitor>
        void                            pair_simplices(OrderIndex bg, OrderIndex end, const Visitor& visitor = Visitor());

        // Struct: PairVisitor
        // Acts as an archetype and if necessary a base class for visitors passed to <pair_simplices(bg, end, visitor)>.
        struct                          PairVisitor
        {
            // Function: init(i)
            // Called after OrderElement pointed to by `i` has been initialized 
            // (its cycle is set to be its boundary, and pair is set to self, i.e. `i`)
            void                        init(OrderIndex i) const                                {}
            
            // Function: update(j, i)
            // Called after the cycle of `i` has been added to the cycle of `j`, 
            // this allows the derived class to perform the necessary updates 
            // (e.g., add `i`'s chain to `j`'s chain)
            void                        update(OrderIndex j, OrderIndex i) const                {}

            // Function: finished(j)
            // Called after the processing of `j` is finished.
            void                        finished(OrderIndex j) const                            {}
        };

        const Order&                    order() const                                           { return order_; }

    private:
        Order                           order_;
        OrderComparison                 ocmp_;
};

#include "static-persistence.hpp"

#endif // __STATIC_PERSISTENCE_H__
