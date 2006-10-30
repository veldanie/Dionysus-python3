#include <algorithm>
#include <vector>

#include <boost/serialization/split_member.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/utility.hpp>

using boost::serialization::make_nvp;
using boost::serialization::make_binary_object;

template<class I, class OrderCmp, class ConsistencyCmp>
Cycle<I,OrderCmp,ConsistencyCmp>::
Cycle(): sz(0)
{}

template<class I, class OrderCmp, class ConsistencyCmp>
Cycle<I,OrderCmp,ConsistencyCmp>::
Cycle(const Cycle& c): CycleRepresentation(c), sz(c.sz)					
{}

template<class I, class OrderCmp, class ConsistencyCmp>
void
Cycle<I,OrderCmp,ConsistencyCmp>::
append(const_reference x)						
{ push_back(x); }
		
template<class I, class OrderCmp, class ConsistencyCmp>
typename Cycle<I,OrderCmp,ConsistencyCmp>::const_reference				
Cycle<I,OrderCmp,ConsistencyCmp>::
top(const OrderComparison& cmp) const
{ 
	AssertMsg(!empty(), "Cycle must not be empty for top()");
	const_iterator min = begin();
	for (const_iterator cur = ++begin(); cur != end(); ++cur)
		if (cmp(*cur, *min))
			min = cur;
	return *min; 
}

template<class I, class OrderCmp, class ConsistencyCmp>
void 
Cycle<I,OrderCmp,ConsistencyCmp>::
swap(Cycle& c)
{
	CycleRepresentation::swap(c);
	std::swap(sz, c.sz);
}

template<class I, class OrderCmp, class ConsistencyCmp>
void 
Cycle<I,OrderCmp,ConsistencyCmp>::
sort(const ConsistencyComparison& cmp)
{ 
	std::vector<Item> tmp(begin(), end());
	std::sort(tmp.begin(), tmp.end(), cmp);
	std::copy(tmp.begin(), tmp.end(), begin());
}

template<class I, class OrderCmp, class ConsistencyCmp>
typename Cycle<I,OrderCmp,ConsistencyCmp>::iterator 
Cycle<I,OrderCmp,ConsistencyCmp>::
get_second(const OrderComparison& cmp) const
{
	AssertMsg(!empty(), "Cycle must not be empty for get_second()");
	if (size() < 2)			return begin();					// Indicates that there is no second.

	Dout(dc::cycle, "Looking for second");
	AssertMsg(size() >= 2, "Cycle must have at least two elements for get_second()");
	iterator min = begin();
	iterator second = ++begin();
	if (cmp(*second, *min))
		std::swap(min, second);
	for (iterator cur = boost::next(begin(),2); cur != end(); ++cur)
	{
		if (cmp(*cur, *min))
		{
			second = min;
			min = cur;
		} else if (cmp(*cur, *second))
		{
			second = cur;
		}
	}
	
	Dout(dc::cycle, "Looked up: " << **second);
	return second;
}

template<typename I, class OrderCmp, class ConsistencyCmp>
typename Cycle<I,OrderCmp,ConsistencyCmp>::const_reference				
Cycle<I,OrderCmp,ConsistencyCmp>::
first_between(const_reference i, const_reference j, const OrderComparison& cmp)
{
	// Find the first element in ConsistencyComparison order (> i and <= j)
	const_pointer first = &i;
	iterator cur = begin();
	for (; cur != end(); ++cur)
	{
		if ((*cur == j) || (cmp(*cur, j) && cmp(i, *cur)))
		{
			first = &(*cur);
			break;
		}
	}
	
	// If no such element found, then we are done
	if (cur == end())
		return i;

	// Find first element in OrderComparison order (> i and <= j)
	for (++cur; cur != end(); ++cur)
	{
		if ((*cur == j) || (cmp(*cur, j) && cmp(i, *cur)))
		{
			if (cmp(*cur, *first))
				first = &(*cur);
		}
	}
	return *first;
}

template<typename I, class OrderCmp, class ConsistencyCmp>
void 
Cycle<I,OrderCmp,ConsistencyCmp>::
erase_between(const_reference i, const_reference j, const OrderComparison& cmp)
{
	for (iterator cur = begin(); cur != end(); ++cur)
		while ((cur != end()) && ((*cur == j) || (cmp(*cur, j) && cmp(i, *cur))))
		{
			Dout(dc::cycle, "Iteration of the erase while loop");
			cur = erase(cur);
		}
}

template<typename I, class OrderCmp, class ConsistencyCmp>
std::ostream& 
Cycle<I,OrderCmp,ConsistencyCmp>::
operator<<(std::ostream& out) const
{
	for (const_iterator cur = begin(); cur != end(); ++cur)
	{
		out << **cur << " ";
	}
	// out << "(last: " << *last << ")";  // For debugging only
	return out;
}

template<typename I, class OrderCmp, class ConsistencyCmp>
std::ostream& 
operator<<(std::ostream& out, const Cycle<I, OrderCmp, ConsistencyCmp>& c)	
{
	return c.operator<<(out);
}

template<typename I, class OrderCmp, class ConsistencyCmp>
typename Cycle<I, OrderCmp, ConsistencyCmp>::Self& 
Cycle<I, OrderCmp, ConsistencyCmp>::
add(const Self& c, const ConsistencyCmp& cmp)
{
	Dout(dc::cycle, "Adding cycles");
	
	iterator 			cur1 = begin();
	const_iterator 		cur2 = c.begin();

	while (cur2 != c.end())
	{
		if (cur1 == end())
		{
			while (cur2 != c.end())
				append(*cur2++);
			return *this;
		}

		// mod 2
		int res = cmp.compare(*cur1, *cur2);
		if (res == 0)		// *cur1 == *cur2
		{
			Dout(dc::cycle, "Equality");
			cur1 = erase(cur1);		// erase cur1 --- as a result cur1 will be pointing at old_cur1++
			--sz;
			++cur2;
		} else if (res < 0)	// *cur1 < *cur2
		{
			Dout(dc::cycle, "Less than");
			cur1++;
		} else if (res > 0) // *cur1 > *cur2
		{
			Dout(dc::cycle, "Greater than");
			insert(cur1, *cur2);
			++cur2;
			++sz;
		}
	}

	Dout(dc::cycle, "After addition: " << *this);
	return *this;
}

template<typename I, class OrderCmp, class ConsistencyCmp>
typename Cycle<I,OrderCmp,ConsistencyCmp>::const_reference 
Cycle<I,OrderCmp,ConsistencyCmp>::
add_and_first_between(const Self& c, const ConsistencyComparison& consistency_cmp,
					  const_reference i, const_reference j, const OrderComparison& order_cmp)
{
	add(c, consistency_cmp);
	return first_between(i,j, order_cmp);
}

template<typename I, class OrderCmp, class ConsistencyCmp>
typename Cycle<I,OrderCmp,ConsistencyCmp>::const_reference 
Cycle<I,OrderCmp,ConsistencyCmp>::
get_first(const OrderComparison& cmp) const
{ return top(cmp); }

		
template<typename I, class OrderCmp, class ConsistencyCmp>
template<class Archive> 
void						
Cycle<I,OrderCmp,ConsistencyCmp>::
serialize(Archive& ar, version_type )
{
	ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Parent);
	ar & make_nvp("size", sz);;
}


/*
template<typename I, class Cmp>
void 
Cycle<I, Cmp>::
insertion_sort(const Comparison& cmp)
{
	TemporaryCycleRepresenation tmp;

	// Insertion sort into the temporary list
	for (const_iterator cur = begin(); cur != end(); ++cur)
	{
		typename TemporaryCycleRepresenation::iterator tmp_cur = tmp.end();
		typename TemporaryCycleRepresenation::iterator tmp_next = tmp_cur--;

		while (tmp_next != tmp.begin())
		{
			if (cmp(*cur, *tmp_cur))
				tmp_next = tmp_cur--;
			else 
				break;
		}
		tmp.insert(tmp_next, *cur);
	}

	// Copy temporary list back into ourselves
	iterator cur = begin();
	typename TemporaryCycleRepresenation::const_iterator tmp_cur = tmp.begin();
	while(tmp_cur != tmp.end())
	{
		*cur = *tmp_cur;
		++cur; ++tmp_cur;
	}
}
*/


