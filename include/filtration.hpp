#include "counter.h"
#include "types.h"

#include <boost/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/is_abstract.hpp>

using boost::serialization::make_nvp;

/* Filtration Public */

template<class S, class FS>
Filtration<S, FS>::
Filtration(): paired(false)					
{}
	
template<class S, class FS>
template<class OtherFltrSmplx>
Filtration<S, FS>::
Filtration(const Filtration<Simplex,OtherFltrSmplx>& filtration)
{
	for(typename Filtration<Simplex,OtherFltrSmplx>::const_Index cur = filtration.begin(); cur != filtration.end(); ++cur)
		push_back(*cur);

	fill_simplex_index_map();

	// TODO: finish
}

template<class S, class FS>
Filtration<S, FS>::
~Filtration() 								
{}

template<class S, class FS>
void 
Filtration<S, FS>::
pair_simplices(Index bg)
{
	Dout(dc::filtration, "Entered: compute_pairing");
	for (Index j = bg; j != end(); ++j)
	{
		Dout(dc::filtration|flush_cf|continued_cf, *j << ": ");
		init_cycle_trail(j); 
		Cycle& bdry = j->cycle();
		Dout(dc::finish, bdry);
		
		counters.inc("Boundaries", j->dimension());
		counters.inc("SimplexCount");

		while(!bdry.empty())
		{
			Index i = bdry.top(cycles_cmp);
			Dout(dc::filtration, *i << ": " << *(i->pair()));
			AssertMsg(i->value() <= j->value(), "Values in the cycle must be less than the value of the simplex");

			// i is not paired, so we pair j with i
			if (i->pair() == i)
			{
				Dout(dc::filtration, "Pairing " << *i << " and " << *j << " with cycle " << j->cycle());
				i->set_pair(j);
				j->set_pair(i);
				counters.inc("DepositedCycleLength", j->cycle().size());
				break;
			}

			// continue searching --- change the Dout to the continued mode with newlines FIXME
			Dout(dc::filtration, "  Adding: [" << bdry << "] + ");
			Dout(dc::filtration, "          [" << i->pair()->cycle() << "]");
			bdry.add(i->pair()->cycle(), consistency_cmp);
			i->pair()->trail().append(j);
			Dout(dc::filtration, "After addition: " << bdry);
		}
	}
	paired = true;
}

template<class S, class FS>
bool							
Filtration<S, FS>::
is_paired() const
{ return paired; }

/* Filtration Protected */
template<class S, class FS>
typename Filtration<S, FS>::Index 
Filtration<S, FS>::
append(const Simplex& s)
{ 
	Index i = push_back(FiltrationSimplex(s)); 
	i->set_pair(i);
	return i;
}
		
template<class S, class FS>
typename Filtration<S, FS>::const_Index 
Filtration<S, FS>::
get_index(const Simplex& s) const
{ 
	typename SimplexMap::const_iterator i = inverse_simplices.find(s); 
	if (i == inverse_simplices.end())
		return end();
	else
		return i->second;
}

template<class S, class FS>
typename Filtration<S, FS>::Index 
Filtration<S, FS>::
get_index(const Simplex& s)
{ 
	typename SimplexMap::const_iterator i = inverse_simplices.find(s); 
	if (i == inverse_simplices.end())
		return end();
	else
		return i->second;
}

template<class S, class FS>
void
Filtration<S, FS>::
fill_simplex_index_map()
{
	for (Index i = begin(); i != end(); ++i)
		inverse_simplices[*i] = i;
}


/* FIXME
template<class S>
template<class Cmp> 
void Filtration<S>::sort_simplices(const Cmp& cmp)
{
	// Far from efficient, but should work
	typedef std::multiset<Simplex, Cmp> SimplexSet;
	SimplexSet ordered_simplices(cmp);
	for (Index i = begin(); i != end(); ++i)
		ordered_simplices.insert(*i);
	
	simplices.clear();
	inverse_simplices.clear();
	for (typename SimplexSet::const_iterator cur = ordered_simplices.begin();
											 cur != ordered_simplices.end();
											 ++cur)
	{
		append(*cur);
	}
}
*/
	
template<class S, class FS>
std::ostream& 
Filtration<S, FS>::
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

/* Filtration Private */
template<class S, class FS>
void
Filtration<S, FS>::init_cycle_trail(Index j)
{
	typename Simplex::Cycle bdry = j->boundary();

	for (typename Simplex::Cycle::const_iterator cur = bdry.begin(); cur != bdry.end(); ++cur)
	{
		Dout(dc::filtration, "Appending in init_cycle_trail(): " << *cur);
		j->cycle().append(get_index(*cur));
	}
	j->cycle().sort(consistency_cmp);

	j->trail().append(j);
}


/* Serialization */
template<class S, class FS>
template<class Archive> 
void 
Filtration<S, FS>::
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

template<class S, class FS>
template<class Archive>	
void 
Filtration<S, FS>::
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

template<class S, class FS>
std::ostream& 
operator<<(std::ostream& out, const Filtration<S, FS>& f)					
{ return f.operator<<(out); }


