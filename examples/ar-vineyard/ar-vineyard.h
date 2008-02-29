/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2007
 */

#ifndef __AR_VINEYARD_H__
#define __AR_VINEYARD_H__

#include <boost/signals.hpp>
#include <boost/bind.hpp>
#include <list>
#include <vector>

#include "topology/conesimplex.h"
#include "topology/filtration.h"
#include "geometry/kinetic-sort.h"
#include "geometry/simulator.h"

#include "ar-simplex3d.h"
#include "ar-function-kernel.h"


template <class Simulator_>
class ARConeSimplex3D: public ConeSimplex<ARSimplex3D>
{
	public:
		typedef						ConeSimplex<ARSimplex3D>									Parent;
		typedef						ARSimplex3D													ARSimplex3D;
		
		/// \name Simulator types
		/// @{
        typedef                     Simulator_                                                  Simulator;
        typedef                     typename Simulator::FunctionKernel                          FunctionKernel;
        typedef                     typename FunctionKernel::Function                           Function;
        /// @}
		
		/// \name ThresholdSort types
		/// @{
		typedef 					std::list<Function>										    ThresholdList;
        typedef                     typename ThresholdList::iterator                            ThresholdListIterator;

		struct 						ThresholdTrajectoryExtractor
		{	Function                operator()(ThresholdListIterator i) const		            { return *i; } };

		typedef						KineticSort<ThresholdListIterator, 
                                                ThresholdTrajectoryExtractor, Simulator>		ThresholdSort;
		/// @}

        typedef                     boost::signal<void (Simulator*)>                            NewMaxSignal;
    
    public:
									ARConeSimplex3D(const ARSimplex3D& s, bool coned = false);
									ARConeSimplex3D(const Parent& p): Parent(p)                 {}      // crucial for boundary() to work correctly
									ARConeSimplex3D(const ARConeSimplex3D& other):              // need explicit copy-constructor because of the signal
                                        Parent(other, other.coned()), 
                                        thresholds_(other.thresholds_)                          {}

		const ThresholdList&        thresholds() const											{ return thresholds_; }

        NewMaxSignal&               new_max_signal()                                            { return new_max_signal_; }
        const Function&             max_threshold() const                                       { return thresholds_.back(); }
		void						schedule_thresholds(Simulator* simulator);

        // need explicit operator= because of the signal
        ARConeSimplex3D&            operator=(const ARConeSimplex3D& other)                     { Parent::operator=(other); thresholds_ = other.thresholds_; return *this; }

								
	private:
		ThresholdList				thresholds_;
		ThresholdSort				thresholds_sort_;
        NewMaxSignal                new_max_signal_;

		void						swap_thresholds(ThresholdListIterator i, Simulator* simulator);
};

/**
 * Encapsulated filtration, and provides compute_vineyard() functionality.
 */
class ARVineyard
{
	public:
		typedef						ARVineyard													Self;
	
        /// \name FunctionKernel and Simulator types
        /// @{
        typedef                     ARFunctionKernel                                            FunctionKernel;
        typedef                     FunctionKernel::Function                                    Function;
        typedef                     Simulator<FunctionKernel>                                   Simulator;
        /// @}

        /// \name Filtration types
        /// @{    
        typedef                     ARConeSimplex3D<Simulator>                                  ARConeSimplex3D;
		typedef						Filtration<ARConeSimplex3D>									Filtration;
		typedef						Filtration::Simplex										    Simplex;
		typedef						Filtration::Index											Index;
		typedef						Filtration::Vineyard										Vineyard;
		typedef						Vineyard::Evaluator											Evaluator;
        /// @}
		
        /// \name SimplexSort types
        /// @{
        struct 						SimplexTrajectoryExtractor
		{	Function				operator()(Index i) const									{ return i->max_threshold(); } };

		typedef						KineticSort<Index, SimplexTrajectoryExtractor, Simulator>   SimplexSort;
		typedef						SimplexSort::iterator										SimplexSortIterator;
		
        class                       ThresholdChangeSlot;              // used to notify of change in max threshold
		/// @}

		typedef						std::list<Point>											PointList;

		class						StaticEvaluator;
		class						KineticEvaluator;

	public:
									ARVineyard(const PointList& points, const Point& z);
									~ARVineyard();

		void						compute_pairing();
		void						compute_vineyard();
		
		const Filtration*			filtration() const											{ return filtration_; }
		const Vineyard*				vineyard() const											{ return vineyard_; }

	public:
		void 						swap(Index i, Simulator* simulator);						///< For kinetic sort
	
	private:
		void 						add_simplices();
		void						change_evaluator(Evaluator* eval);

	private:
		Filtration*				    filtration_;
		Vineyard*					vineyard_;
		Evaluator*					evaluator_;

		Point						z_;
		Delaunay					dt_;
				
#if 0
	private:
		// Serialization
		friend class boost::serialization::access;
		
		ARVineyard() 																	{}

		template<class Archive> 
		void serialize(Archive& ar, version_type )
		{ 
			// FIXME
		};
#endif
};

//BOOST_CLASS_EXPORT(ARVineyard)

#ifdef COUNTERS
static Counter*  cARVineyardTrajectoryKnee =		 GetCounter("ar/vineyard/trajectoryknee");
#endif

class ARVineyard::ThresholdChangeSlot
{   
    public:
                                ThresholdChangeSlot(SimplexSortIterator iter, SimplexSort* sort, 
                                                    Vineyard* vineyard, Simulator* sort_simulator):
                                    iter_(iter), sort_(sort), vineyard_(vineyard), sort_simulator_(sort_simulator)      { iter_->element->new_max_signal().connect(*this); }
        void                    operator()(Simulator* simulator)                                
        { 
            Count(cARVineyardTrajectoryKnee); 
            sort_->update_trajectory(iter_, sort_simulator_); 
            if (iter_->element->sign()) 
                vineyard_->record_knee(iter_->element);
            else
                vineyard_->record_knee(iter_->element->pair());
        }
    
    private:
        SimplexSortIterator     iter_;
        SimplexSort*            sort_;              // could make both of these static
        Vineyard*               vineyard_;          // currently inefficient since there is
                                                    // only one SimplexSort and one Vineyard, 
                                                    // but each is stored in every slot
        Simulator*              sort_simulator_;
};

class ARVineyard::StaticEvaluator: public Evaluator
{
	public:
									StaticEvaluator()                   						{}

		virtual RealType			time() const												{ return 0; }
		virtual RealType			value(const Simplex& s) const								{ return s.value(); }
};

class ARVineyard::KineticEvaluator: public Evaluator
{
	public:
									KineticEvaluator(Simulator* simulator): 
                                        simulator_(simulator)                                   {}

		virtual RealType			time() const												{ return simulator_->current_time(); }
		virtual RealType			value(const Simplex& s)	const								{ return FunctionKernel::value_at(s.max_threshold(), time()); }

	private:
		Simulator*                  simulator_;
};


#include "ar-vineyard.hpp"

#endif // __AR_VINEYARD_H__
