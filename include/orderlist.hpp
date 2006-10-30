/* Implementations */

template<class T>
void OrderList<T>::swap(iterator i, iterator j)
{
	typename Parent::iterator i_base = i.get_base();
	typename Parent::iterator j_base = j.get_base();
	std::swap(i_base->current, j_base->current);

	// Exchange the actual elements in the list --- so that iterators behave as expected
	typename Parent::iterator after_j = boost::next(j_base);	
	Parent::splice(i_base, *this, j_base);
	Parent::splice(after_j, *this, i_base);
}

template<class T>
typename OrderList<T>::iterator OrderList<T>::push_back(const_reference x)
{
	OrderType index = size();
	Parent::push_back(NodeType(x, index, index));
	return last();
}


/* OrderComparison */
template<class T>
int OrderList<T>::OrderComparison::compare(ComparableType a, ComparableType b) const
{
	if (a.get_base()->current == b.get_base()->current)			return 0;
	if (a.get_base()->current < b.get_base()->current)			return -1;
	return 1;
}


/* LessThanComparison */
template<class T>
int OrderList<T>::LessThanComparison::compare(ComparableType a, ComparableType b) const
{ return Parent::compare(a,b); }

template<class T>
bool OrderList<T>::LessThanComparison::operator()(ComparableType a, ComparableType b) const
{ return compare(a,b) == -1; }


/* GreaterThanComparison */
template<class T>
int OrderList<T>::GreaterThanComparison::compare(ComparableType a, ComparableType b) const
{ return -Parent::compare(a,b); }

template<class T>
bool OrderList<T>::GreaterThanComparison::operator()(ComparableType a, ComparableType b) const
{ return compare(a,b) == -1; }


/* ConsistencyComparison */
template<class T>
int OrderList<T>::ConsistencyComparison::compare(ComparableType a, ComparableType b) const
{ 
	if (a.get_base()->original < b.get_base()->original) 			return -1;
	else if (a.get_base()->original == b.get_base()->original)		return 0;
	else															return 1;
}

template<class T>
bool OrderList<T>::ConsistencyComparison::operator()(ComparableType a, ComparableType b) const
{ return compare(a,b) == -1; }

