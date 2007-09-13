#include "utilities/counter.h"
#include "utilities/types.h"
#include <algorithm>

#include <boost/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/is_abstract.hpp>

using boost::serialization::make_nvp;

/* Filtration Public */

template<class S, class FS, class V>
Filtration<S, FS, V>::
Filtration(Vineyard* vnrd = 0): paired(false), vineyard_(vnrd)
{}
	
template<class S, class FS, class V>
void 
Filtration<S, FS, V>::
pair_simplices(Index bg, Index end)
{
	int i = 0;
	Dout(dc::filtration, "Entered: compute_pairing");
	for (Index j = bg; j != end; ++j)
	{
		if (i++ % 10000 == 0)
			std::cout << i << " simplices processed" << std::endl;

		Dout(dc::filtration|flush_cf|continued_cf, *j << ": ");
		init_cycle_trail(j); 
		Cycle& bdry = j->cycle();
		Dout(dc::finish, bdry);
		
		CountNum("Boundaries", j->dimension());
		Count("SimplexCount");

		while(!bdry.empty())
		{
			Index i = bdry.top(cycles_cmp);
			Dout(dc::filtration, *i << ": " << *(i->pair()));
			AssertMsg(!cycles_cmp(i, j), "Simplices in the cycle must precede current simplex: " << 
										 "(" << *i << " in cycle of " << *j << ")");

			// i is not paired, so we pair j with i
			if (i->pair() == i)
			{
				Dout(dc::filtration, "Pairing " << *i << " and " << *j << " with cycle " << j->cycle());
				i->set_pair(j);
				j->set_pair(i);
				CountNum("DepositedCycleLength", j->cycle().size());
				break;
			}

			// continue searching --- change the Dout to the continued mode with newlines FIXME
			Dout(dc::filtration, "  Adding: [" << bdry << "] + ");
			Dout(dc::filtration, "          [" << i->pair()->cycle() << "]");
			bdry.add(i->pair()->cycle(), get_consistency_cmp());
			i->pair()->trail().append(j, get_consistency_cmp());
			Dout(dc::filtration, "After addition: " << bdry);
		}
		Dout(dc::filtration, "Finished with " << *j << ": " << *(j->pair()));
	}
	paired = true;
}

template<class S, class FS, class V>
bool							
Filtration<S, FS, V>::
is_paired() const
{ return paired; }

/**
 * Transposes simplices at i and i+1, and records the knee in the vineyard if there is a change in pairing. 
 * Returns true if the pairing changed.
 */
template<class S, class FS, class V>
bool
Filtration<S,FS,V>::
transpose(Index i, bool maintain_lazy)
{
	AssertMsg(vineyard() != 0, "We must have a vineyard for transpositions");
	
	Index i_orig = i++;
	
	AssertMsg(i_orig->pair() != i, "Transposing simplices must not be paired");
	bool result = transpose_simplices(i_orig, maintain_lazy);
	AssertMsg(i_orig == boost::next(i), "Wrong indices after transposition");
	
	if (result) vineyard()->switched(i, i_orig);
	return result;
}

template<class S, class FS, class V>
typename Filtration<S, FS, V>::Index 
Filtration<S, FS, V>::
append(const Simplex& s)
{ 
	Index i = push_back(FiltrationSimplex(s)); 
	return i;
}

template<class S, class FS, class V>
typename Filtration<S, FS, V>::Index 
Filtration<S, FS, V>::
insert(Index prior, const Simplex& s)
{ 
	Index i = Parent::insert(prior, FiltrationSimplex(s)); 
	paired = false;

	return i;
}
		
template<class S, class FS, class V>
typename Filtration<S, FS, V>::const_Index 
Filtration<S, FS, V>::
get_index(const Simplex& s) const
{ 
	typename SimplexMap::const_iterator i = inverse_simplices.find(s); 
	if (i == inverse_simplices.end())
		return end();
	else
		return i->second;
}

template<class S, class FS, class V>
typename Filtration<S, FS, V>::Index 
Filtration<S, FS, V>::
get_index(const Simplex& s)
{ 
	typename SimplexMap::const_iterator i = inverse_simplices.find(s); 
	if (i == inverse_simplices.end())
		return end();
	else
		return i->second;
}

template<class S, class FS, class V>
void
Filtration<S, FS, V>::
fill_simplex_index_map()
{
	for (Index i = begin(); i != end(); ++i)
		inverse_simplices[*i] = i;
}

template<class S, class FS, class V>
std::ostream& 
Filtration<S, FS, V>::
operator<<(std::ostream& out) const
{
	out << "Pairing: " << std::endl;
	for (const_Index i = begin(); i != end(); ++i)
	{
		out << "(" << *i << ", " << *(i->pair()) << "): ";
		out << i->cycle() << std::endl;
	}
	out << std::endl << std::endl;

	return out;
}


/* Filtration Protected */
/// Transposes simplices at i and i+1. Returns true if the pairing switched.
template<class S, class FS, class V>
bool 
Filtration<S,FS,V>::
transpose_simplices(Index i, bool maintain_lazy)
{
	AssertMsg(is_paired(), "Pairing must be computed before transpositions");
	Count("SimplexTransposition");
	
	Index i_prev = i++;

	if (i_prev->dimension() != i->dimension())
	{
		swap(i_prev, i);
		Dout(dc::transpositions, "Different dimension");
		Count("Case DiffDim");
		return false;
	}
	
	bool si = i_prev->sign(), sii = i->sign();
	if (si && sii)
	{
		Dout(dc::transpositions, "Trail prev: " << i_prev->trail());

		// Case 1
		TrailIterator i_in_i_prev = std::find(i_prev->trail().begin(), i_prev->trail().end(), i);
		if (i_in_i_prev != i_prev->trail().end())
		{
			Dout(dc::transpositions, "Case 1, U[i,i+1] = 1");
			i_prev->trail().erase(i_in_i_prev);
		}

		Index k = i_prev->pair();
		Index l = i->pair();

		// Explicit treatment of unpaired simplex
		if (l == i)
		{
			swap(i_prev, i);
			Dout(dc::transpositions, "Case 1.2 --- unpaired");
			Dout(dc::transpositions, *i_prev);
			Count("Case 1.2");
			return false;
		} else if (k == i_prev)
		{
			if (std::find(l->cycle().begin(), l->cycle().end(), i_prev) == l->cycle().end())
			{
				// Case 1.2
				swap(i_prev, i);
				Dout(dc::transpositions, "Case 1.2 --- unpaired");
				Dout(dc::transpositions, *i_prev);
				Count("Case 1.2");
				return false;
			} else
			{
				// Case 1.1.2 --- special version (plain swap, but pairing switches)
				swap(i_prev, i);
				pairing_switch(i_prev, i);
				Dout(dc::transpositions, "Case 1.1.2 --- unpaired");
				Dout(dc::transpositions, *i_prev);
				Count("Case 1.1.2");
				return true;
			}
		}
		
		Dout(dc::transpositions, "l cycle: " << l->cycle());
		if (std::find(l->cycle().begin(), l->cycle().end(), i_prev) == l->cycle().end())
		{
			// Case 1.2
			if (maintain_lazy)
			{
				TrailIterator k_in_l = std::find(l->trail().begin(), l->trail().end(), k);
				if (k_in_l != l->trail().end())
				{
					l->trail().add(k->trail(), Filtration::get_consistency_cmp());		// Add row k to l
					k->cycle().add(l->cycle(), Filtration::get_consistency_cmp());		// Add column l to k
				}
			}
			swap(i_prev, i);
			Dout(dc::transpositions, "Case 1.2");
			Count("Case 1.2");
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
				Count("Case 1.1.1");
				return false;
			} else
			{
				// Case 1.1.2
				swap(i_prev, i);
				k->cycle().add(l->cycle(), Filtration::get_consistency_cmp());		// Add column l to k
				l->trail().add(k->trail(), Filtration::get_consistency_cmp());		// Add row k to l
				pairing_switch(i_prev, i);
				Dout(dc::transpositions, "Case 1.1.2");
				Count("Case 1.1.2");
				return true;
			}
		}
	} else if (!si && !sii)
	{
		// Case 2
		if (std::find(i_prev->trail().begin(), i_prev->trail().end(), i) == i_prev->trail().end())
		{
			// Case 2.2
			swap(i_prev, i);
			Dout(dc::transpositions, "Case 2.2");
			Count("Case 2.2");
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
				Count("Case 2.1.2");
				return true;
			} 
			
			// Case 2.1.1
			Dout(dc::transpositions, "Case 2.1.1");
			Count("Case 2.1.1");
			return false;
		}
	} else if (!si && sii)
	{
		// Case 3
		if (std::find(i_prev->trail().begin(), i_prev->trail().end(), i) == i_prev->trail().end())
		{
			// Case 3.2
			swap(i_prev, i);
			Dout(dc::transpositions, "Case 3.2");
			Count("Case 3.2");
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
			Count("Case 3.1");
			return true;
		}
	} else if (si && !sii)
	{
		// Case 4
		TrailIterator i_in_i_prev = std::find(i_prev->trail().begin(), i_prev->trail().end(), i);
		if (i_in_i_prev != i_prev->trail().end())
		{
			Dout(dc::transpositions, "Case 4, U[i,i+1] = 1");
			i_prev->trail().erase(i_in_i_prev);
		}
		swap(i_prev, i);
		Dout(dc::transpositions, "Case 4");
		Count("Case 4");
		return false;
	}
	
	return false; // to avoid compiler complaints, should never reach this point
}


/* Filtration Private */
template<class S, class FS, class V>
void
Filtration<S, FS, V>::
init_cycle_trail(Index j)
{
	typename Simplex::Cycle bdry = j->boundary();

	for (typename Simplex::Cycle::const_iterator cur = bdry.begin(); cur != bdry.end(); ++cur)
	{
		Dout(dc::filtration, "Appending in init_cycle_trail(): " << *cur);
		AssertMsg(get_index(*cur) != end(), "Non-existent simplex in the cycle");
		j->cycle().append(get_index(*cur), get_consistency_cmp());
	}
	j->trail().append(j, get_consistency_cmp());
	j->set_pair(j);
}

/// Update the pairing, so that whoever was paired with i is now paired with j and vice versa.
template<class S, class FS, class V>
void 
Filtration<S,FS,V>::
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

/* Serialization */
template<class S, class FS, class V>
template<class Archive> 
void 
Filtration<S, FS, V>::
save(Archive& ar, version_type ) const
{
	ar << BOOST_SERIALIZATION_NVP(paired);
	ar << BOOST_SERIALIZATION_NVP(cycles_cmp);
	ar << BOOST_SERIALIZATION_NVP(trails_cmp);
	ar << BOOST_SERIALIZATION_NVP(consistency_cmp);

	SizeType sz = size();
	ar << make_nvp("size", sz);
	Dout(dc::filtration, "Size: " << sz);

	/* Record integer indices */
	IndexIntMap index_map; SizeType i = 0;
	for (const_Index cur = begin(); cur != end(); ++cur)
	{ index_map[cur] = i++; }
	
	/* Save the simplices */
	int count = 0;
	for (const_Index cur = begin(); cur != end(); ++cur)
	{ 
		count++;
		// FIXME
		//FiltrationSimplexSerialization simplex = FiltrationSimplexSerialization(*cur, index_map);
		//ar << make_nvp("FiltrationSimplex", simplex);	
	}
	Dout(dc::filtration, count << " simplices serialized");
}

template<class S, class FS, class V>
template<class Archive>	
void 
Filtration<S, FS, V>::
load(Archive& ar, version_type )
{
	Dout(dc::filtration, "Starting to read filtration");
	ar >> BOOST_SERIALIZATION_NVP(paired);
	ar >> BOOST_SERIALIZATION_NVP(cycles_cmp);
	ar >> BOOST_SERIALIZATION_NVP(trails_cmp);
	ar >> BOOST_SERIALIZATION_NVP(consistency_cmp);
	Dout(dc::filtration, "In Filtration: first block read");

	SizeType sz;
	ar >> make_nvp("size", sz);
	Dout(dc::filtration, "In Filtration: size read " << sz);
	
	IndexVector index_vector(sz);
	for (SizeType i = 0; i < sz; ++i)
	{
		index_vector[i] = append(Simplex());
	}
		
	int count = 0;
	for (SizeType i = 0; i < sz; ++i)
	{
		// FIXME
		//FiltrationSimplexSerialization simplex;
		//ar >> make_nvp("FiltrationSimplex", simplex);
		count++;
		Dout(dc::filtration, "In Filtration: simplex read (" << count << ")");
		//simplex.set_filtration_simplex(*index_vector[i], index_vector);
	}
	Dout(dc::filtration, "In Filtration: simplices read");
}

template<class S, class FS, class V>
std::ostream& 
operator<<(std::ostream& out, const Filtration<S, FS, V>& f)					
{ return f.operator<<(out); }


