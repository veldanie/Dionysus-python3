#include <utilities/log.h>

/* Implementation */
	
void
ARConeSimplex::
swap_thresholds(SimplexSort* sort, ThresholdList::iterator i, Simulator* simulator)
{
	typename ThresholdList::iterator n = boost::next(i);
	tl->splice(i, *tl, n);
	if (n == tl->begin())
		sort->update_trajectory(kinetic_key(), simulator);
}

void
ARConeSimplex::
schedule_thresholds(SimplexSort* sort, Simulator* simulator)
{
	thresholds_sort_.initialize(thresholds_.begin(), thresholds_.end(), 
								boost::bind(&ARConeSimplex::swap_thresholds, this, sort, _1, _2), simulator);
}


ARVineyard::
ARVineyard(const PointList& points, const Point& z): simplex_sort_(0), z_(z)
{
	for (PointList::const_iterator cur = points.begin(); cur != points.end(); ++cur)
		dt_.insert(*cur);
	std::cout << "Delaunay triangulation computed" << std::endl;

	ARSimplex3DVector alpha_ordering;
	fill_alpha_order(dt_, z_, alpha_ordering);
	std::cout << "Delaunay simplices: " << alpha_ordering.size() << std::endl;
		
	evaluator_ = new StaticEvaluator(0);
	vineyard_ = new Vineyard(evaluator_);

	filtration_ = new ARFiltration(vineyard_);
	for (ARSimplex3DVector::const_iterator cur = alpha_ordering.begin(); cur != alpha_ordering.end(); ++cur)
	{
		filtration_->append(ARConeSimplex(*cur));						// Delaunay simplex
		filtration_->append(ARConeSimplex(*cur, true));	// Coned off delaunay simplex
	}
}

ARVineyard::
~ARVineyard()
{
	delete filtration_;
	delete vineyard_;
	delete evaluator_;
}

void
ARVineyard::
compute_pairing()
{
	filtration_->fill_simplex_index_map();
	filtration_->pair_simplices(filtration_->begin(), filtration_->end());
	vineyard_->start_vines(filtration_->begin(), filtration_->end());
	std::cout << "Simplices paired" << std::endl;
}

void					
ARVineyard::
compute_vineyard(bool explicit_events)
{
	AssertMsg(filtration_->is_paired(), "Simplices must be paired for a vineyard to be computed");
	
	Simulator simulator;
	SimplexSort	simplex_sort;
	
	// Set thresholds
	for (Index cur = filtration_->begin(); cur != filtration_->end(); ++cur)
	{
		cur->thresholds.push_back(ARConeSimplex::Polynomial(CGAL::to_double(cur->alpha())));
			
		if (!cur->coned()) continue;						// non-coned simplices stay put, so we are done

		Time lambda_alpha = CGAL::to_double((cur->alpha() - cur->rho()));	// when lambda becomes greater than alpha
		lambda_alpha += 2*CGAL::sqrt(CGAL::to_double(cur->s()*lambda_alpha));
		lambda_alpha += CGAL::to_double(cur->s() + cur->v());

		Time phi_alpha = CGAL::to_double(cur->alpha() - cur->phi_const());

		Time phi_lambda = CGAL::to_double(cur->rho() + cur->s() - cur->v() - cur->phi_const());
		phi_lambda *= phi_lambda;
		phi_lambda /= CGAL::to_double(4*cur->s());
		phi_lambda += CGAL::to_double(cur->v());

		Time sv = CGAL::to_double(cur->s() + cur->v());
		
		if (true || phi_lambda < sv || phi_lambda < phi_alpha)		// FIXME: remove true
		{
			sp->new_event(Time(phi_alpha), 
						  MembershipFunctionChangeEvent(cur->kinetic_key(),
														cf(F::NT(CGAL::to_double(cur->phi_const())), 1),
														apt));		// \phi^2 = r^2 + \phi_c^2
			std::cout << "Scheduled" << std::endl;
		} else
			std::cout << "Not scheduled" << std::endl;
		

		//sp->new_event(Time(...), MembershipFunctionChangeEvent(cur->kinetic_key()));
		
		std::cout << *cur << std::endl;
		std::cout << "lambda_alpha: " 		<< lambda_alpha << std::endl;
		std::cout << "phi_alpha: " 			<< phi_alpha << std::endl;
		std::cout << "phi_lambda: " 		<< phi_lambda << std::endl;
		std::cout << "s^2 + v^2: " 			<< sv << std::endl;
		std::cout << std::endl;
		
		cur->set_kinetic_key();
	}


	// Once thresholds are set (and sorted), we can initialize the simplex_sort
	simplex_sort.initialize(filtration_.begin(), filtration_.end(), 
							boost::bind(&ARVineyard::swap, this, _1, _2), &simulator);


	
	// Process all the events (compute the vineyard in the process)
	// FIXME: the time should not be 1, but something like twice the radius of
	// the pointset as seen from z
	change_evaluator(new KineticEvaluator(sp, apt, 0));
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
	
	//change_evaluator(new StaticEvaluator(1));
	vineyard_->record_diagram(filtration_->begin(), filtration_->end());
}
		
void 					
ARVineyard::
swap(Key a, Key b)
{
	Index ao = kinetic_map_[a], bo = kinetic_map_[b];
	AssertMsg(filtration_->get_trails_cmp()(ao, bo), "In swap(a,b), a must precede b");
	filtration_->transpose(ao);
	AssertMsg(filtration_->get_trails_cmp()(bo, ao), "In swap(a,b), b must precede a after the transposition");
}

void
ARVineyard::
change_evaluator(Evaluator* eval)
{
	AssertMsg(evaluator_ != 0, "change_evaluator() assumes that existing evaluator is not null");
		
	delete evaluator_;
	evaluator_ = eval;
	vineyard_->set_evaluator(evaluator_);
}

void 
ARVineyardBase::MembershipFunctionChangeEvent::
process() const
{
	apt_->set(key_, function_);
	std::cout << "Updated for phi's dominance" << std::endl;
}


template<class Vertex_handle>
void
ARVineyardBase::SortVisitor::
before_swap(Vertex_handle a, Vertex_handle b) const
{ 
	std::cout << "Swapping elements" << *a << " and " << *b << std::endl;
	arv_->swap(*a,*b); 
}


std::ostream&
ARVineyardBase::MembershipFunctionChangeEvent::
operator<<(std::ostream& out) const
{
	return out << "Membership change" << std::endl;
}

