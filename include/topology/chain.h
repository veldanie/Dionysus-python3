/**
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005-2008
 */

#ifndef __CHAIN_H__
#define __CHAIN_H__

//#include "utilities/types.h"
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/optional.hpp>
#include <iterator>
#include <string>

#include <utilities/containers.h>

/**
 * Class: ChainWrapper
 * Class storing a chain of simplices. Stores items in the order defined by ConsistencyCmp. 
 * The actual order of the elements is defined by OrderCmp. Instances of those 
 * classes are not stored in Chain for efficiency, and are passed as arguments to those methods 
 * that require them.
 *
 * \ingroup topology
 *
 * TODO: add field (defaulting to Z_2)
 */
template <class Container_>
class ChainWrapper: public Container_, 
                    public SizeStorage<Container_>
{
    public:
        /// \name Template Parameters
        /// @{
        typedef         Container_                                                                  Container;
        typedef         typename boost::iterator_value<typename Container::iterator>::type          Item;
        /// @}
        
        typedef         ChainWrapper                                                                Self;
        typedef         Container                                                                   ChainRepresentation; 

        /// \name Accessor typedefs
        /// @{
        typedef         typename ChainRepresentation::iterator                                      iterator; 
        typedef         typename ChainRepresentation::const_iterator                                const_iterator; 
        typedef         typename ChainRepresentation::const_reference                               const_reference; 
        typedef         typename ChainRepresentation::reference                                     reference; 
        typedef         typename ChainRepresentation::pointer                                       pointer; 
        typedef         typename ChainRepresentation::const_pointer                                 const_pointer; 
        typedef         Item                                                                        value_type;
        /// @}
        
    public:     
                                                ChainWrapper();
                                                ChainWrapper(const ChainWrapper& c);
        
        /// \name Whole Chain operations
        /// @{
        /** Add c to *this assuming both Chains are sorted in increasing order according to cmp. */
        template<class ConsistencyComparison>                        
        Self&                                   add(const Self& c, const ConsistencyComparison& cmp);
        
        void                                    swap(ChainWrapper& c);                          ///< Swaps the contents of c and *this (like STL's swap destroys c)
        
        template<class ConsistencyComparison>
        void                                    sort(const ConsistencyComparison& cmp);         ///< Sort elements to enforce consistency

        size_t                                  size() const                                        { return SizeStorage<Container>::size(*this); }

        using                                   ChainRepresentation::empty;
        using                                   ChainRepresentation::clear;
        /// @}
        
        /// \name Modifiers
        /// @{
        using                                   ChainRepresentation::erase;
        
        template<class ConsistencyComparison>
        void                                    append(const_reference x, const ConsistencyComparison& cmp);
        /// @}
        
        /// \name Accessors
        /// @{
        using                                   ChainRepresentation::begin;
        using                                   ChainRepresentation::end;
        
        template<class OrderComparison>
        const_reference                         top(const OrderComparison& cmp) const;          ///< First element in cmp order

        boost::optional<const_iterator>         contains(const_reference x) const;              ///< tests whether chain contains x
        boost::optional<iterator>               contains(reference x);                          ///< tests whether chain contains x
        /// @}
    
        /// \name Debugging
        /// @{
        template<class OrderComparison>
        const_reference                         get_first(const OrderComparison& cmp) const;    ///< First element in cmp order

        template<class OutputMap>
        std::string                             tostring(const OutputMap& outmap = OutputMap()) const;
        /// @}
        
    private:
        using                                   ChainRepresentation::push_back;
        using                                   ChainRepresentation::insert;
        
    private:
        // Serialization
        typedef                                 Container                                           Parent;
        friend class                            boost::serialization::access;
        template<class Archive> 
        void                                    serialize(Archive& ar, boost::serialization::version_type );
};

#include "chain.hpp"

#endif // __CHAIN_H__

