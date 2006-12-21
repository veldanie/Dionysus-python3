/* Implementations */

#include <fstream>
#include <sstream>

template<class FS>
void
Vineyard<FS>::
start_vines(Index bg, Index end)
{
	AssertMsg(evaluator != 0, "Cannot start vines with a null evaluator");
	for (Index cur = bg; cur != end; ++cur)
	{
		if (!cur->sign()) continue;
		Dimension dim = cur->dimension();
		
		if (dim >= vines.size())
		{
			AssertMsg(dim == vines.size(), "New dimension has to be contiguous");
			vines.push_back(std::list<Vine>());
		}

		start_vine(cur);
		record_knee(cur);
	}
}

template<class FS>
void
Vineyard<FS>::
switched(Index i, Index j)
{
	Vine* i_vine = i->vine();
	Vine* j_vine = j->vine();
	i->set_vine(j_vine);
	j->set_vine(i_vine);

	// Since the pairing has already been updated, the following assertions should be true
	AssertMsg(i->vine() == i->pair()->vine(), "i's vine must match the vine of its pair");
	AssertMsg(j->vine() == j->pair()->vine(), "j's vine must match the vine of its pair");

	if (!i->sign()) i = i->pair();
	if (!j->sign()) j = j->pair();
	record_knee(i);
	record_knee(j);
}

template<class FS>
void
Vineyard<FS>::
start_vine(Index i)
{
	Dout(dc::vineyard, "Starting new vine");
	AssertMsg(i->sign(), "Can only start vines for positive simplices");
		
	Dimension dim = i->dimension();
	vines[dim].push_back(Vine());
	i->set_vine(&vines[dim].back());
	i->pair()->set_vine(i->vine());
}
	
/// Records the current diagram in the vineyard
template<class FS>
void 
Vineyard<FS>::
record_diagram(Index bg, Index end)
{
	Dout(dc::vineyard, "Entered: record_diagram()");
	AssertMsg(evaluator != 0, "Cannot record diagram with a null evaluator");
	
	for (Index i = bg; i != end; ++i)
	{
		AssertMsg(i->vine() != 0, "Cannot process a null vine in record_diagram");
		if (!i->sign())		continue;
		record_knee(i);
	}
}


template<class FS>
void			
Vineyard<FS>::
save_edges(const std::string& filename) const
{
	for (int i = 0; i < vines.size(); ++i)
	{
		std::ostringstream os; os << i;
		std::string fn = filename + os.str() + ".edg";
		std::ofstream out(fn.c_str());
		for (typename VineList::const_iterator vi = vines[i].begin(); vi != vines[i].end(); ++vi)
			for (typename Vine::const_iterator ki = vi->begin(), kiprev = ki++; ki != vi->end(); kiprev = ki++)
			{
				out << kiprev->birth << ' ' << kiprev->death << ' ' << kiprev->time << std::endl;
				out << ki->birth << ' ' << ki->death << ' ' << ki->time << std::endl;
			}
		out.close();
	}
}

/// Records a knee for the given simplex
template<class FS>
void
Vineyard<FS>::
record_knee(Index i)
{
	Dout(dc::vineyard, "Entered record_knee()");
	AssertMsg(evaluator != 0, "Cannot record knee with a null evaluator");
	AssertMsg(i->vine() != 0, "Cannot add a knee to a null vine");
	AssertMsg(i->sign(), "record_knee() must be called on a positive simplex");
	
	if (!i->is_paired())
		i->vine()->add(evaluator->value(*i), Infinity, evaluator->time());
	else
	{
		Dout(dc::vineyard, "Creating knee");
		Knee k(evaluator->value(*i), evaluator->value(*(i->pair())), evaluator->time());
		Dout(dc::vineyard, "Knee created: " << k);

		if (!k.is_diagonal() || i->vine()->empty())			// non-diagonal k, or empty vine
		{
			Dout(dc::vineyard, "Extending a vine");
			i->vine()->add(k);
		}
		else if (i->vine()->back().is_diagonal())			// last knee is diagonal
		{
			AssertMsg(i->vine()->size() == 1, "Only first knee may be diagonal for a live vine");
			Dout(dc::vineyard, "Overwriting first diagonal knee");
			i->vine()->back() = k;
		} else												// finish this vine
		{
			Dout(dc::vineyard, "Finishing a vine");
			i->vine()->add(k);
			start_vine(i);
			i->vine()->add(k);
		}
	}
	
	i->vine()->back().set_cycle(resolve_cycle(i));
	Dout(dc::vineyard, "Leaving record_knee()");
}

template<class FS>
typename Vineyard<FS>::Knee::SimplexList  
Vineyard<FS>::
resolve_cycle(Index i) const
{
	Dout(dc::vineyard, "Entered resolve_cycle");
	const Cycle& cycle = i->cycle();
	
	// Resolve simplices
	typename Knee::SimplexList lst;
	for (typename Cycle::const_iterator cur = cycle.begin(); cur != cycle.end(); ++cur)
		lst.push_back(**cur);

	return lst;
}
