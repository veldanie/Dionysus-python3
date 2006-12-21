/* Implementations */

template<class VI, class Smplx, class FltrSmplx, class Vnrd>
template<class VertexCmp>
LowerStarFiltration<VI,Smplx,FltrSmplx,Vnrd>::
LowerStarFiltration(VertexIndex begin, VertexIndex end, const VertexCmp& cmp, Vineyard* vineyard):
	Parent(vineyard)
{
	// Record VertexIndexes in a temporary list
	typedef std::list<VertexIndex> VertexIndexList;
	VertexIndexList tmp_list;
	while (begin != end)
		tmp_list.push_back(begin++);

	// Sort the temporary list
	tmp_list.sort(cmp);

	// Record vertex order
	for(typename VertexIndexList::const_iterator cur = tmp_list.begin(); cur != tmp_list.end(); ++cur)
		(*cur)->set_order(vertex_order.push_back(VertexDescriptor(*cur, Parent::append(Simplex(*cur)))));
}

template<class VI, class Smplx, class FltrSmplx, class Vnrd>
typename LowerStarFiltration<VI,Smplx,FltrSmplx,Vnrd>::Index 
LowerStarFiltration<VI,Smplx,FltrSmplx,Vnrd>::
append(const Simplex& s)
{
	AssertMsg(s.dimension() != 0, "All vertices must have been inserted in the constructor");
	
	// Find the highest vertex
	typename Simplex::VertexContainer::const_iterator cur = s.vertices().begin(), max = cur++;
	for (; cur != s.vertices().end(); ++cur)
		if (!vertex_cmp((*cur)->get_order(), (*max)->get_order()))
			max = cur;

	Index ms = (*max)->get_order()->simplex_index;	Index prior;
	do { prior = ms++; } while (ms->dimension() <= s.dimension() && ms != Parent::end() && ms->get_attachment() == *max);
	
	Index i = Parent::insert(prior, s);
	i->set_attachment(*max);

	return i;
}

template<class VI, class Smplx, class FltrSmplx, class Vnrd>
bool
LowerStarFiltration<VI,Smplx,FltrSmplx,Vnrd>::SimplexAttachmentComparison::
operator()(const Simplex& first, const Simplex& second) const
{
	int cmp = vertex_cmp.compare(first.get_attachment()->get_order(), second.get_attachment()->get_order());
	if (cmp == 0)
		return first.dimension() < second.dimension();
	else
		return cmp == -1;
}

template<class VI, class Smplx, class FltrSmplx, class Vnrd>
bool 
LowerStarFiltration<VI,Smplx,FltrSmplx,Vnrd>::
transpose_vertices(const VertexOrderIndex& order)
{
	counters.inc("VertexTransposition");

	if ((counters.lookup("VertexTransposition") % 1000000) == 0)
	{
		Dout(dc::lsfiltration, "Vertex transpositions:  " << counters.lookup("VertexTransposition"));
		Dout(dc::lsfiltration, "Simplex transpositions: " << counters.lookup("SimplexTransposition"));
		Dout(dc::lsfiltration, "Attachment changed:     " << counters.lookup("ChangedAttachment"));
		Dout(dc::lsfiltration, "Regular disconnected:   " << counters.lookup("RegularDisconnected"));
		Dout(dc::lsfiltration, "Pairing Changed:        " << counters.lookup("ChangedPairing"));
		Dout(dc::lsfiltration, "------------------------");
	}
	
	Dout(dc::lsfiltration, "Transposing vertices (" << order->vertex_index << ", " 
													<< boost::next(order)->vertex_index << ")");

	Index i = order->simplex_index;
	Index i_prev = boost::prior(i);
	Index i_next = boost::next(order)->simplex_index;
	Index i_next_prev = boost::prior(i_next);			// transpositions are done in terms of the first index in the pair
	Index j = boost::next(i_next);
	
	const VertexIndex& v_i = order->vertex_index;
	const VertexIndex& v_i_next = boost::next(order)->vertex_index;
	bool nbghrs = neighbors(v_i, v_i_next);
	
	bool result = false;
	
	// First, move the vertex --- this can be sped up if we devise special "vertex transpose" operation
	while (i_next_prev != i_prev)						
	{ 
		result |= transpose(i_next_prev);
		i_next_prev = boost::prior(i_next);
	}
	Dout(dc::lsfiltration, "Done moving the vertex");

	// Second, move the simplices attached to it
	Dout(dc::lsfiltration, "Moving attached simplices");
	while (j != Parent::end() && j->get_attachment() == v_i_next)
	{
		Dout(dc::lsfiltration, "  Considering " << *j);
		if (nbghrs && j->contains(v_i))			// short circuit
		{
			counters.inc("ChangedAttachment");
			Dout(dc::lsfiltration, "  Attachment changed for " << *j);
			j->set_attachment(v_i);
			++j;
			continue;
		}	

		Index j_prev = j; ++j;
		while ((--j_prev)->get_attachment() != v_i_next) 		// i.e., until we have reached v_i_next 
															// (and the simplices that follow it) again
		{
			Dout(dc::lsfiltration, "    Moving: " << *j_prev << ", " << *boost::next(j_prev));
			AssertMsg(j_prev->get_attachment() == v_i, "Simplex preceding the one being moved must be attached to v_i");
			result |= transpose(j_prev);
			--j_prev;
		}
	}
	Dout(dc::lsfiltration, "Done moving attached simplices");
	vertex_order.swap(order, boost::next(order));
	
	return result;
}

template<class VI, class Smplx, class FltrSmplx, class Vnrd>
bool 
LowerStarFiltration<VI,Smplx,FltrSmplx,Vnrd>::
transpose(Index i)
{
	Index j = boost::next(i);
	
	Dout(dc::lsfiltration, "    Transposing (" << *i << ", " << *(i->pair()) << ") and (" 
											   << *j << ", " << *(j->pair()) << ")");

	assert_pairing(i);
	assert_pairing(j);

	bool res = Parent::transpose(i);
	Dout(dc::lsfiltration, "    " << *j << ": " << *(j->pair()) << ", " << *i << ": " << *(i->pair()));

	assert_pairing(j);
	assert_pairing(i);

	return res;
}

template<class VI, class Smplx, class FltrSmplx, class Vnrd>
void 
LowerStarFiltration<VI,Smplx,FltrSmplx,Vnrd>::
assert_pairing(Index i)
{
#ifndef NDEBUG
	AssertMsg(i != Parent::end(), "Cannot assert pairing of end()");
	if (!i->sign())
	{
		if (i->pair() != i->cycle().top(Parent::get_cycles_cmp()))
		{
			Dout(dc::lsfiltration, "i (negative): " << *i);
			Dout(dc::lsfiltration, "pair(i): " << *(i->pair()));
			Dout(dc::lsfiltration, "i->cycle().top(): " << *(i->cycle().top(Parent::get_cycles_cmp())));
			DoutFatal(dc::fatal, "Pairing not matching the matrix at " << *i);
		}
	} else
	{
		if (i->pair() != i)
		{
			if (i->pair()->cycle().top(Parent::get_cycles_cmp()) != i)
			{
				Dout(dc::lsfiltration, "i (positive): " << *i);
				Dout(dc::lsfiltration, "pair(i): " << *(i->pair()));
				Dout(dc::lsfiltration, "pair(i)->cycle(): " << i->pair()->cycle());
				Dout(dc::lsfiltration, "pair->cycle().top(): " << *(i->pair()->cycle().top(Parent::get_cycles_cmp())));
				DoutFatal(dc::fatal, "Pairing not matching the matrix at " << *(i->pair()));
			}
		}
	}
#endif
}


template<class VI, class Smplx, class FltrSmplx, class Vnrd>
template<class Archive>
void 
LowerStarFiltration<VI,Smplx,FltrSmplx,Vnrd>::
load(Archive& ar, version_type )
{
/*
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Parent);
	
	// Count the number of vertices
	VertexIndex num_vertices = 0;
	for (Index cur = begin(); cur != end(); ++cur)
		if (dimension(cur) == 0)	
			num_vertices++;

	// Second pass to record vertex_order
	vertex_order.resize(num_vertices);
	inverse_vertex_order.resize(num_vertices);
	num_vertices = 0;
	for (Index cur = begin(); cur != end(); ++cur)
	{
		if (dimension(cur) == 0)
		{
			vertex_order[num_vertices].index = cur;
			vertex_order[num_vertices].vertex_index = *(cur->get_vertices().begin());
			inverse_vertex_order[vertex_order[num_vertices].vertex_index] = num_vertices;
			++num_vertices;
		}
	}
*/
}


