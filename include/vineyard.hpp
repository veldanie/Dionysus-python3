/* Implementations */


template<class S, class F, class VS>
Vineyard<S,F,VS>::
Vineyard()
{
}
	
template<class S, class F, class VS>
Vineyard<S,F,VS>::
Vineyard(const Vineyard& vineyard): 
	Filtration(vineyard)
{
	// TODO: copy vines
}

template<class S, class F, class VS>
template<class OtherFltrSmplx>	
Vineyard<S,F,VS>::
Vineyard(const typename Filtration::template rebind<OtherFltrSmplx>::other& filtration):
	Filtration(filtration)
{
	// TODO (if anything?)
}


template<class S, class F, class VS>
void 
Vineyard<S,F,VS>::
pair_simplices(Index bg)
{
	Filtration::pair_simplices(bg);
	start_vines(bg);	
	record_frame(bg);
}

/** Starts a new frame by recording the current diagram in the vineyard */
template<class S, class F, class VS>
void 
Vineyard<S,F,VS>::
record_frame(Index bg)
{
	Dout(dc::vineyard, "Entered: record_frame()");
	
	AssertMsg(is_paired(), "Pairing must be computed before a vine frame can be recorded");
	for (Index i = begin(); i != end(); ++i)
	{
		if (i->sign())		continue;
		Knee* k = i->new_frame(Knee(i->pair()->value(), i->value(), 0));
		k->set_cycle(resolve_cycle(i));
	}
}

/**
 * Transposes simplices at i and i+1, and records the knee in the vineyard if there is a change in pairing. 
 * Returns true if the pairing changed.
 */
template<class S, class F, class VS>
bool
Vineyard<S,F,VS>::
transpose(Index i)
{
	bool result = transpose_simplices(i);
	if (result)
	{
		// Record pairing: TODO
	}
	return result;
}

/// Transposes simplices at i and i+1. Returns true if the pairing switched.
template<class S, class F, class VS>
bool 
Vineyard<S,F,VS>::
transpose_simplices(Index i)
{
	AssertMsg(is_paired(), "Pairing must be computed before transpositions");
	counters.inc("SimplexTransposition");
	
	Index i_prev = i++;

	if (i_prev->dimension() != i->dimension())
	{
		swap(i_prev, i);
		Dout(dc::transpositions, "Different dimension");
		counters.inc("Case DiffDim");
		return false;
	}
	
	bool si = i_prev->sign(), sii = i->sign();
	if (si && sii)
	{
		Dout(dc::transpositions, "Trail prev: " << i_prev->trail());

		// Case 1
		TrailIterator i_prev_second = i_prev->trail().get_second(Filtration::get_trails_cmp());
		if (*i_prev_second == i)
		{
			Dout(dc::transpositions, "Case 1, U[i,i+1] = 1");
			i_prev->trail().erase(i_prev_second);
		}

		Index k = i_prev->pair();
		Index l = i->pair();

		// Explicit treatment of unpaired simplex
		if (l == i)
		{
			swap(i_prev, i);
			Dout(dc::transpositions, "Case 1.2 --- unpaired");
			Dout(dc::transpositions, *i_prev);
			counters.inc("Case 1.2");
			return false;
		} else if (k == i_prev)
		{
			if (*(l->cycle().get_second(Filtration::get_cycles_cmp())) != i_prev)
			{
				// Case 1.2
				swap(i_prev, i);
				Dout(dc::transpositions, "Case 1.2 --- unpaired");
				Dout(dc::transpositions, *i_prev);
				counters.inc("Case 1.2");
				return false;
			} else
			{
				// Case 1.1.2 --- special version (plain swap, but pairing switches)
				swap(i_prev, i);
				pairing_switch(i_prev, i);
				Dout(dc::transpositions, "Case 1.1.2 --- unpaired");
				Dout(dc::transpositions, *i_prev);
				counters.inc("Case 1.1.2");
				return true;
			}
		}
		
		Dout(dc::transpositions, "l cycle: " << l->cycle());
		if (*(l->cycle().get_second(Filtration::get_cycles_cmp())) != i_prev)
		{
			// Case 1.2
			swap(i_prev, i);
			Dout(dc::transpositions, "Case 1.2");
			counters.inc("Case 1.2");
			return false;
		} else
		{
			// Case 1.1
			if (trails_cmp(k,l))
			{
				// Case 1.1.1
				swap(i_prev, i);
				l->cycle().add(k->cycle(), Filtration::get_consistency_cmp());		// Add column k to l
				k->trail().add(l->trail(), Filtration::get_consistency_cmp());		// Add row l to k
				Dout(dc::transpositions, "Case 1.1.1");
				counters.inc("Case 1.1.1");
				return false;
			} else
			{
				// Case 1.1.2
				swap(i_prev, i);
				k->cycle().add(l->cycle(), Filtration::get_consistency_cmp());		// Add column l to k
				l->trail().add(k->trail(), Filtration::get_consistency_cmp());		// Add row k to l
				pairing_switch(i_prev, i);
				Dout(dc::transpositions, "Case 1.1.2");
				counters.inc("Case 1.1.2");
				return true;
			}
		}
	} else if (!si && !sii)
	{
		// Case 2
		if (*(i_prev->trail().get_second(Filtration::get_trails_cmp())) != i)
		{
			// Case 2.2
			swap(i_prev, i);
			Dout(dc::transpositions, "Case 2.2");
			counters.inc("Case 2.2");
			return false;
		} else
		{
			// Case 2.1
			Index low_i = i_prev->pair();
			Index low_ii = i->pair();
			i_prev->trail().add(i->trail(), Filtration::get_consistency_cmp());			// Add row i to i_prev
			i->cycle().add(i_prev->cycle(), Filtration::get_consistency_cmp());			// Add column i_prev to i
			swap(i_prev, i);	
			if (Filtration::get_trails_cmp()(low_ii, low_i))
			{
				// Case 2.1.2
				i_prev->cycle().add(i->cycle(), Filtration::get_consistency_cmp());		// Add column i to i_prev (after transposition)
				i->trail().add(i_prev->trail(), Filtration::get_consistency_cmp());			// Add row i to i_prev
				pairing_switch(i_prev, i);
				Dout(dc::transpositions, "Case 2.1.2");
				counters.inc("Case 2.1.2");
				return true;
			} 
			
			// Case 2.1.1
			Dout(dc::transpositions, "Case 2.1.1");
			counters.inc("Case 2.1.1");
			return false;
		}
	} else if (!si && sii)
	{
		// Case 3
		if (*(i_prev->trail().get_second(Filtration::get_trails_cmp())) != i)
		{
			//AssertMsg(pair(i)->cycle_get_second(cycles_cmp) != i, dc::transpositions, "Problem in Case 3");
			// Case 3.2
			swap(i_prev, i);
			Dout(dc::transpositions, "Case 3.2");
			counters.inc("Case 3.2");
			return false;
		} else
		{
			// Case 3.1
			i_prev->trail().add(i->trail(), Filtration::get_consistency_cmp());			// Add row i to i_prev
			i->cycle().add(i_prev->cycle(), Filtration::get_consistency_cmp());			// Add column i_prev to i
			swap(i_prev, i);
			i_prev->cycle().add(i->cycle(), Filtration::get_consistency_cmp());			// Add column i_prev to i (after transposition)
			i->trail().add(i_prev->trail(), Filtration::get_consistency_cmp());			// Add row i to i_prev
			pairing_switch(i_prev, i);
			Dout(dc::transpositions, "Case 3.1");
			counters.inc("Case 3.1");
			return true;
		}
	} else if (si && !sii)
	{
		// Case 4
		TrailIterator i_prev_second = i_prev->trail().get_second(Filtration::get_trails_cmp());
		if (*i_prev_second == i)
		{
			Dout(dc::transpositions, "Case 4, U[i,i+1] = 1");
			i_prev->trail().erase(i_prev_second);
		}
		swap(i_prev, i);
		Dout(dc::transpositions, "Case 4");
		counters.inc("Case 4");
		return false;
	}
	
	return false; // to avoid compiler complaints, should never reach this point
}

template<class S, class F, class VS>
typename Vineyard<S,F,VS>::Knee::SimplexList  
Vineyard<S,F,VS>::
resolve_cycle(Index i) const
{
	Dout(dc::filtration, "Entered: resolve_cycle");
	const Cycle& cycle = i->cycle();
		
	const CyclesComparator& cmp = Filtration::get_cycles_cmp();
	
#if 0
	// Make into canonical cycle
	bool done = false;
	while (!done)
	{
		done = true;
		for (typename Cycle::const_iterator cur = cycle.begin(); cur != cycle.end(); ++cur)
		{
			if (!((*cur)->sign()))
				continue;

			if (cmp(i,(*cur)->pair()))
			{
				done = false;
				cycle.add((*cur)->pair()->cycle(), get_consistency_cmp());
				break;
			}
		}
	}
	std::cout << "Canonical cycle computed" << std::endl;
#endif

	// Resolve simplices
	typename Knee::SimplexList lst;
	for (typename Cycle::const_iterator cur = cycle.begin(); cur != cycle.end(); ++cur)
		lst.push_back(**cur);

	return lst;
}


/* Vineyard Private */
/** Initializes vines in VineyardSimplices; should be called after pair_simplices() */
template<class S, class F, class VS>
void 
Vineyard<S,F,VS>::
start_vines(Index bg)
{
	for (Index i = bg; i != end(); ++i)
	{
		if (!i->sign())		continue;
		Vine* v = new Vine;
		i->set_vine(v);
		i->pair()->set_vine(v);				// if i is unpaired i->pair() == i
	}
}

/// Update the pairing, so that whoever was paired with i is now paired with j and vice versa.
template<class S, class F, class VS>
void 
Vineyard<S,F,VS>::
pairing_switch(Index i, Index j)
{
	Index i_pair = i->pair();
	Index j_pair = j->pair();

	if (i_pair == i)
		j->set_pair(j);
	else
	{
		j->set_pair(i_pair);
		i_pair->set_pair(j);
	}

	if (j_pair == j)
		i->set_pair(i);
	else
	{
		i->set_pair(j_pair);
		j_pair->set_pair(i);
	}
}
