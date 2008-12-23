#include <algorithm>
#include <vector>
#include "utilities/containers.h"

#include <boost/serialization/split_member.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/utility.hpp>

#include "utilities/log.h"
#include "utilities/counter.h"

using boost::serialization::make_nvp;
using boost::serialization::make_binary_object;

#ifdef LOGGING
static rlog::RLogChannel* rlChain = 				DEF_CHANNEL( "topology/chain", rlog::Log_Debug);
#endif // LOGGING

#ifdef COUNTERS
static Counter*  cChainAddBasic =		 			GetCounter("chain/add/basic");
static Counter*  cChainAddComparison =		 		GetCounter("chain/add/comparison");
#endif // COUNTERS

template<class C>
ChainWrapper<C>::
ChainWrapper()
{}

template<class C>
ChainWrapper<C>::
ChainWrapper(const ChainWrapper& c): ChainRepresentation(c)
{}

template<class C>
template<class ConsistencyCmp>
void
ChainWrapper<C>::
append(const_reference x, const ConsistencyCmp& cmp)						
{ 
    SizeStorage<Container>::increment();

	// First try the special cases that x goes at the end
	const_reference last = ChainRepresentation::back();
	if (empty() || cmp(last, x))
	{
		push_back(x); 
		return;
	}

	for (iterator cur = begin(); cur != end(); ++cur)
		if (cmp(x, *cur))
		{
			insert(cur, x);
			return;
		}
}
		
template<class C>
template<class OrderComparison>
typename ChainWrapper<C>::const_reference				
ChainWrapper<C>::
top(const OrderComparison& cmp) const
{ 
	AssertMsg(!empty(), "Chain must not be empty for low()");
	const_iterator min = begin();
	for (const_iterator cur = ++begin(); cur != end(); ++cur)
		if (cmp(*cur, *min))
			min = cur;
	return *min; 
}

template<class C>
void 
ChainWrapper<C>::
swap(ChainWrapper& c)
{
	ChainRepresentation::swap(c);
    SizeStorage<Container>::swap(c);
}

template<class C>
template<class ConsistencyComparison>
void 
ChainWrapper<C>::
sort(const ConsistencyComparison& cmp)
{ 
    ContainerTraits<C,ConsistencyComparison>::sort(*this, cmp);
}

template<class C>
boost::optional<typename ChainWrapper<C>::const_iterator>
ChainWrapper<C>::
contains(const_reference x) const
{
#if 0
    for (const_iterator cur = begin(); cur != end(); ++cur)
        if (!cmp(*cur, x) && !cmp(x, *cur))
            return make_optional(cur);

    return make_optional(false);
#endif
    const_iterator res = std::find(begin(), end(), x);
    return make_optional(res != end(), res);
}

template<class C>
boost::optional<typename ChainWrapper<C>::iterator>
ChainWrapper<C>::
contains(reference x)
{
    iterator res = std::find(begin(), end(), x);
    return boost::make_optional(res != end(), res);
}

template<class C>
template<class OutputMap>
std::string
ChainWrapper<C>::
tostring(const OutputMap& outmap) const
{
    std::string str;
	for (const_iterator cur = begin(); cur != end(); ++cur)
	{
        if (cur != begin()) str += ", ";
		str += outmap(*cur);
	}
	// str += "(last: " + *last + ")";  // For debugging only
	return str;
}

template<class C>
template<class ConsistencyCmp>
typename ChainWrapper<C>::Self& 
ChainWrapper<C>::
add(const Self& c, const ConsistencyCmp& cmp)
{
    // TODO: tmp-based addition is necessary and useful for Containers that are vectors, 
    //       however, I believe it creates costly overhead for Containers that are lists.
    //       Need to put some thought into this.
    ChainRepresentation     tmp;
    SizeStorage<Container>  size;

    // FIXME: need to do something about the output
	rLog(rlChain, "Adding chains"); //: ": %s + %s",  tostring(*this).c_str(), tostring(c).c_str());
	
	iterator 			cur1 = begin();
	const_iterator 		cur2 = c.begin();

	while (cur2 != c.end())
	{
		if (cur1 == end())
		{
			while (cur2 != c.end())
			{
				tmp.push_back(*cur2++);
                size.increment();
				Count(cChainAddBasic);
			}
			rLog(rlChain, "After addition"); //: %s", tostring(*this).c_str());
            
            static_cast<ChainRepresentation*>(this)->swap(tmp);
            static_cast<SizeStorage<Container>*>(this)->swap(size);
			return *this;
		}

		// mod 2
		int res = cmp.compare(*cur1, *cur2);
		Count(cChainAddComparison);
		rLog(rlChain, "Comparison result: %i", res);
		if (res == 0)		// *cur1 == *cur2
		{
			rLog(rlChain, "Equality");
			//cur1 = erase(cur1);		// erase cur1 --- as a result cur1 will be pointing at old_cur1++
            ++cur1;
			++cur2;
		} else if (res < 0)	// *cur1 < *cur2
		{
			rLog(rlChain, "Less than");
			//cur1++;
            tmp.push_back(*cur1++);
            size.increment();
		} else if (res > 0) // *cur1 > *cur2
		{
			rLog(rlChain, "Greater than");
			//insert(cur1, *cur2);
			//++cur2;
            tmp.push_back(*cur2++);
            size.increment();
		}
		Count(cChainAddBasic);
	}
	while (cur1 != end())
	{
		tmp.push_back(*cur1++);
        size.increment();
		Count(cChainAddBasic);
	}

	rLog(rlChain, "After addition"); //: %s", tostring(*this).c_str());
    
    static_cast<ChainRepresentation*>(this)->swap(tmp);
    static_cast<SizeStorage<Container>*>(this)->swap(size);
	return *this;
}

template<class C>
template<class OrderComparison>
typename ChainWrapper<C>::const_reference 
ChainWrapper<C>::
get_first(const OrderComparison& cmp) const
{ return top(cmp); }

		
template<class C>
template<class Archive> 
void						
ChainWrapper<C>::
serialize(Archive& ar, boost::serialization::version_type )
{
	ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Parent);
}
