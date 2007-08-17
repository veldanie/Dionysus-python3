/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2007
 */

#ifndef __AR_VINEYARD_H__
#define __AR_VINEYARD_H__

#include "utilities/sys.h"
#include "utilities/debug.h"

#include "topology/conesimplex.h"
#include "topology/filtration.h"
#include "geometry/kinetic-sort.h"

#include <list>
#include "ar-simplex3d.h"


class ARConeSimplex: public ConeSimplex<ARSimplex3D>
{
	public:
		typedef						ConeSimplex<ARSimplex3D>									Parent;
		typedef						ARSimplex3D													ARSimplex3D;
		typedef						Filtration<ARConeSimplex>									Filtration;
		
		/// \name Polynomial Kernel types
		/// @{
		typedef						double														FieldType;
		typedef						UPolynomial<FieldType>										PolyKernel;
		typedef						PolyKernel::Polynomial										Polynomial;
		typedef						Simulator<PolyKernel>										Simulator;
		
		typedef						KineticSort<ARFiltration, SimplexTrajectoryExtractor, Simulator>
																								SimplexSort;
		typedef						SimplexSort::iterator										SimplexSortIterator;
		typedef						SimplexSortIterator											Key;
		/// @}

		/// \name Kinetic Sort types
		/// @{
		typedef 					std::list<Polynomial>										ThresholdList;

		struct 						ThresholdTrajectoryExtractor
		{	Polynomial				operator()(ThresholdList::iterator i) const				{ return *i; } }
		struct 						SimplexTrajectoryExtractor
		{	Polynomial				operator()(ARFiltration::iterator i) const					{ i->thresholds().front(); }

		typedef						KineticSort<ThresholdList, ThresholdTrajectoryExtractor, Simulator>	
																								ThresholdSort;
		/// @}

									ARConeSimplex(const ARSimplex3D& s, bool coned = false): 
										Parent(s, coned), 
										thresholds_sort_(&thresholds_)							{}

		Key							kinetic_key() const											{ return key_; }
		void						set_kinetic_key(Key k)										{ key_ = k; }
		const ThresholdList&		thresholds() const											{ return thresholds_; }

		void						schedule_thresholds(Simulator* simulator);

								
	private:
		Key							key_;
		ThresholdList				thresholds_;
		ThresholdSort				thresholds_sort_;

		void						swap_thresholds(ThresholdList* tl, typename ThresholdList::iterator i);
};


class ARVineyard
{
	public:
		typedef						ARVineyard													Self;
		
		typedef						ARConeSimplex::Filtration									ARFiltration;	
		typedef						ARFiltration::Simplex										Simplex;
		typedef						ARFiltration::Index											Index;
		typedef						ARFiltration::Vineyard										Vineyard;
		typedef						Vineyard::Evaluator											Evaluator;
		
		typedef						ARConeSimplex::Simulator									Simulator;	
		typedef						ARConeSimplex::SimplexSort									SimplexSort;	


		typedef						std::list<Point>											PointList;

		class						StaticEvaluator;
		class						KineticEvaluator;

	public:
									ARVineyard(const PointList& points, const Point& z);
									~ARVineyard();

		void						compute_pairing();
		void						compute_vineyard(bool explicit_events = false);
		
		const ARFiltration*			filtration() const											{ return filtration_; }
		const Vineyard*				vineyard() const											{ return vineyard_; }

	public:
		// For Kinetic Sort
		static void 				swap(ARFiltration* filtration, Index i);
	
	private:
		void 						add_simplices();
		void						change_evaluator(Evaluator* eval);

	private:
		ARFiltration*				filtration_;
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


class ARVineyard::StaticEvaluator: public Evaluator
{
	public:
									StaticEvaluator(RealType t): time_(t)						{}

		virtual RealType			time() const												{ return time_; }
		virtual RealType			value(const Simplex& s) const								{ return s.value(); }

	private:
		RealType					time_;
};

class ARVineyard::KineticEvaluator: public Evaluator
{
	public:
									KineticEvaluator(Simulator::Handle sp, 
													 ActivePointsTable::Handle apt, 
													 RealType time_offset): 
										sp_(sp), apt_(apt)										{}

		virtual RealType			time() const												{ return CGAL::to_double(get_time()); }
		virtual RealType			value(const Simplex& s)	const								{ return CGAL::to_double(apt_->at(s.kinetic_key()).x()(get_time())); }

	private:
		Simulator::Time				get_time() const											{ return sp_->current_time(); }
		
		Simulator::Handle			sp_;
		ActivePointsTable::Handle 	apt_;
};


#include "ar-vineyard.hpp"

#endif // __AR_VINEYARD_H__
