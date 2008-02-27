#include <utilities/log.h>

/* Implementation */

#ifdef LOGGING
static rlog::RLogChannel* rlARVineyard =                        DEF_CHANNEL("ar/vineyard", rlog::Log_Debug);
static rlog::RLogChannel* rlARVineyardComputing =               DEF_CHANNEL("ar/vineyard/computing", rlog::Log_Debug);
static rlog::RLogChannel* rlARVineyardSwap =                    DEF_CHANNEL("ar/vineyard/swap", rlog::Log_Debug);
static rlog::RLogChannel* rlARVineyardConeSimplexSwap =         DEF_CHANNEL("ar/vineyard/conesimplex/swap", rlog::Log_Debug);
#endif


template <class Simulator_>
ARConeSimplex3D<Simulator_>::
ARConeSimplex3D(const ARSimplex3D& s, bool coned): Parent(s, coned)
{}
	
template <class Simulator_>
void
ARConeSimplex3D<Simulator_>::
swap_thresholds(ThresholdListIterator i, Simulator* simulator)
{
    rLog(rlARVineyardConeSimplexSwap, "Transposing %s and %s", tostring(*i).c_str(),
                                                               tostring(*boost::next(i)).c_str());
	typename ThresholdList::iterator n = boost::next(i);
	thresholds_.splice(i, thresholds_, n);
	if (boost::next(i) == thresholds_.end())
        new_max_signal_(simulator);
}

template <class Simulator_>
void
ARConeSimplex3D<Simulator_>::
schedule_thresholds(Simulator* simulator)
{
    if (!coned()) thresholds_.push_back(Function(Function::rho, this));
    else        
    { 
        thresholds_.push_back(Function(Function::phi, this)); 
        thresholds_.push_back(Function(Function::rho, this)); 
    }

	thresholds_sort_.initialize(thresholds_.begin(), thresholds_.end(), 
								boost::bind(&ARConeSimplex3D::swap_thresholds, this, _1, _2), simulator);
}


ARVineyard::
ARVineyard(const PointList& points, const Point& z): z_(z)
{
	for (PointList::const_iterator cur = points.begin(); cur != points.end(); ++cur)
		dt_.insert(*cur);
	rLog(rlARVineyard, "Delaunay triangulation computed");

	ARSimplex3DVector alpha_ordering;
	fill_alpha_order(dt_, z_, alpha_ordering);
	rLog(rlARVineyard, "Delaunay simplices: %i", alpha_ordering.size());
		
	evaluator_ = new StaticEvaluator;
	vineyard_ = new Vineyard(evaluator_);

	filtration_ = new Filtration(vineyard_);
	for (ARSimplex3DVector::const_iterator cur = alpha_ordering.begin(); cur != alpha_ordering.end(); ++cur)
	{
		filtration_->append(ARConeSimplex3D(*cur));         // Delaunay simplex
		filtration_->append(ARConeSimplex3D(*cur, true));   // Coned off delaunay simplex
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
	rLog(rlARVineyard, "Simplices paired");
}

void					
ARVineyard::
compute_vineyard()
{
	AssertMsg(filtration_->is_paired(), "Simplices must be paired for a vineyard to be computed");
	
	Simulator simulator;
	SimplexSort	simplex_sort;
	
	// Schedule thresholds
	for (Index cur = filtration_->begin(); cur != filtration_->end(); ++cur)
        cur->schedule_thresholds(&simulator);

	// Once thresholds are scheduled, we can initialize the simplex_sort
	simplex_sort.initialize(filtration_->begin(), filtration_->end(), 
							boost::bind(&ARVineyard::swap, this, _1, _2), &simulator);
    rLog(rlARVineyardComputing, "SimplexSort initialized");

    // Connect signals and slots
    std::vector<ThresholdChangeSlot> slots; 
    slots.reserve(filtration_->size());
    for (SimplexSortIterator cur = simplex_sort.begin(); cur != simplex_sort.end(); ++cur)
        slots.push_back(ThresholdChangeSlot(cur, &simplex_sort));
    rLog(rlARVineyardComputing, "Signals and slots connected");
	
    // Simulate
	change_evaluator(new KineticEvaluator(&simulator));
    while(!simulator.reached_infinity())
    {
        rLog(rlARVineyardComputing, "Current time before: %lf", simulator.current_time());
        simulator.process();
    }
	
	vineyard_->record_diagram(filtration_->begin(), filtration_->end());
}
		
void 					
ARVineyard::
swap(Index i, Simulator* simulator)
{
    rLog(rlARVineyardSwap, "Transposing %p and %p: %s and %s", 
                            &(*i), &(*boost::next(i)),
                            tostring(*i).c_str(), tostring(*boost::next(i)).c_str());
	filtration_->transpose(i);
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
