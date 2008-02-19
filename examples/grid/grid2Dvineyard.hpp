#include <utilities/log.h>

/* Implementation */
	
Grid2DVineyard::
Grid2DVineyard(Grid2D* g): vertices_(g->size())
{
	grid_stack_.push_back(g); 
	for (CoordinateIndex i = 0; i < g->size(); ++i)
		vertices_[i].set_index(i);

	evaluator_ = new StaticEvaluator(grid(), 0);
	vineyard_ = new Vineyard(evaluator_);

	filtration_ = new LSFiltration(vertices_.begin(), vertices_.end(), VertexComparison(grid()), vineyard_);
	add_simplices();
}

Grid2DVineyard::
~Grid2DVineyard()
{
	delete filtration_;
	delete vineyard_;
	delete evaluator_;
}

void
Grid2DVineyard::
compute_pairing()
{
	filtration_->fill_simplex_index_map();
	filtration_->pair_simplices(filtration_->begin(), filtration_->end());
	vineyard_->start_vines(filtration_->begin(), filtration_->end());
}

void					
Grid2DVineyard::
compute_vineyard(Grid2D* g, bool explicit_events)
{
	AssertMsg(filtration_->is_paired(), "Simplices must be paired for a vineyard to be computed");
	
	typedef Traits::Kinetic_kernel::Point_1 								Point;
	typedef Traits::Kinetic_kernel::Function_kernel::Construct_function 	CF; 
	typedef Traits::Kinetic_kernel::Motion_function 						F; 
	
	Traits tr(0,1);
	Simulator::Handle sp = tr.simulator_handle();
	ActivePointsTable::Handle apt = tr.active_points_1_table_handle();
	Sort sort(tr, SortVisitor(this));
	
	// Setup the (linear) trajectories
	std::cout << "Setting up trajectories" << std::endl;
	CF cf; 
	kinetic_map_.clear();
	for (VertexIndex cur = vertices_.begin(); cur != vertices_.end(); ++cur)
	{
		ValueType val0 = (*grid())(cur->index());
		ValueType val1 = (*g)(cur->index());
		F x = cf(F::NT(val0), F::NT(val1 - val0));			// x = val0 + (val1 - val0)*t
		Point p(x);
		cur->set_kinetic_key(apt->insert(p));
		kinetic_map_[cur->kinetic_key()] = cur->get_order();
		if (cur->index() % 10000 == 0)
			std::cout << "Added trajectory: " << cur->index() << " " << val0 << " " << val1 << std::endl;
	}
	
	// Process all the events (compute the vineyard in the process)
	change_evaluator(new KineticEvaluator(sp, apt, num_grids() - 1));
	if (explicit_events)
	{
		while (sp->next_event_time() < 1)
		{
			std::cout << "Next event time: " << sp->next_event_time() << std::endl;
			sp->set_current_event_number(sp->current_event_number() + 1);
			std::cout << "Processed event" << std::endl;
		}
	} else
		sp->set_current_time(1.0);
	std::cout << "Processed " << sp->current_event_number() << " events" << std::endl;
	
	// Add the grid to the stack
	grid_stack_.push_back(g); 
	change_evaluator(new StaticEvaluator(grid(), num_grids() - 1));
	vineyard_->record_diagram(filtration_->begin(), filtration_->end());
}
		
void 					
Grid2DVineyard::
swap(Key a, Key b)
{
	VertexOrderIndex ao = kinetic_map_[a], bo = kinetic_map_[b];
	AssertMsg(filtration_->get_vertex_cmp()(ao, bo), "In swap(a,b), a must precede b");
	filtration_->transpose_vertices(ao);
	AssertMsg(filtration_->get_vertex_cmp()(bo, ao), "In swap(a,b), b must precede a after the transposition");
}

void 
Grid2DVineyard::
add_simplices()
{
	// Takes advantage of LowerStarFiltration's smart append (which allows faces
	// to be inserted after cofaces, since everything is rearranged in the
	// proper lower star order anyway). Also note that vertices were added by
	// LowerStarFiltration's constructor
	for (CoordinateIndex x = 0; x < grid()->xsize() - 1; ++x)
		for (CoordinateIndex y = 0; y < grid()->ysize() - 1; ++y)
		{
			VertexIndex v(&vertices_[grid()->seq(x,y)]);
			VertexIndex vh(&vertices_[grid()->seq(x+1,y)]);
			VertexIndex vv(&vertices_[grid()->seq(x,y+1)]);
			VertexIndex vd(&vertices_[grid()->seq(x+1,y+1)]);

			Simplex sh(2, v);
			sh.add(vh);	filtration_->append(sh);		// Horizontal edge
			sh.add(vd);	filtration_->append(sh);		// "Horizontal" triangle
			
			Simplex sv(2, v);
			sv.add(vv);	filtration_->append(sv);		// Vertical edge
			sv.add(vd);	filtration_->append(sv);		// "Vertical" triangle
			
			Simplex sd(2, v);
			sd.add(vd); filtration_->append(sd);		// Diagonal edge

			if (y == grid()->ysize() - 2)
			{
				Simplex s(1, vv); 
				s.add(vd); filtration_->append(s);		// Top edge
			}
			if (x == grid()->xsize() - 2)
			{
				Simplex s(1, vh); 
				s.add(vd); filtration_->append(s);		// Right edge
			}
		}
}

void
Grid2DVineyard::
change_evaluator(Evaluator* eval)
{
	AssertMsg(evaluator_ != 0, "change_evaluator() assumes that existing evaluator is not null");
		
	delete evaluator_;
	evaluator_ = eval;
	vineyard_->set_evaluator(evaluator_);
}

