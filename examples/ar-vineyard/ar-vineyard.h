/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2007
 */

#ifndef __AR_VINEYARD_H__
#define __AR_VINEYARD_H__

#include "sys.h"
#include "debug.h"

#include "conesimplex.h"
#include "filtration.h"

#include <CGAL/Kinetic/Inexact_simulation_traits_1.h>
#include <CGAL/Kinetic/Sort.h>
#include <CGAL/Kinetic/Sort_visitor_base.h>

#include <list>
#include "ar-simplex3d.h"

#include <vector>


class ARVineyardBase
{
	public:
		/// \name CGAL Kinetic Sort types
		/// @{
		class						SortVisitor;
		typedef 					CGAL::Kinetic::Inexact_simulation_traits_1 					Traits;
		typedef						CGAL::Kinetic::Sort<Traits, SortVisitor>					Sort;
		typedef 					Traits::Simulator 											Simulator;
		typedef 					Traits::Active_points_1_table								ActivePointsTable;
		typedef 					ActivePointsTable::Key										Key;
		
		typedef 					Traits::Kinetic_kernel::
											Function_kernel::Construct_function 				CF; 
		typedef 					Traits::Kinetic_kernel::Motion_function 					F; 
		/// @}
		
		class						ARConeSimplex;
		class						MembershipFunctionChangeEvent;
};

class ARVineyardBase::ARConeSimplex: public ConeSimplex<ARSimplex3D>
{
	public:
		typedef						ConeSimplex<ARSimplex3D>									Parent;
		typedef						ARSimplex3D													ARSimplex3D;

									ARConeSimplex(const ARSimplex3D& s, bool coned = false): 
										Parent(s, coned)										{}

		Key							kinetic_key() const											{ return key_; }
		void						set_kinetic_key(Key k)										{ key_ = k; }
								
	private:
		Key							key_;
};


class ARVineyard: public ARVineyardBase
{
	public:
		typedef						ARVineyard													Self;
		
		typedef						Filtration<ARConeSimplex>									ARFiltration;	
		typedef						ARFiltration::Simplex										Simplex;
		typedef						ARFiltration::Index											Index;
		typedef						ARFiltration::Vineyard										Vineyard;
		typedef						Vineyard::Evaluator											Evaluator;
		typedef						std::map<Key, Index>										KeyIndexMap;
		
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
		void 						swap(Key a, Key b);
	
	private:
		void 						add_simplices();
		void						change_evaluator(Evaluator* eval);

	private:
		ARFiltration*				filtration_;
		Vineyard*					vineyard_;
		Evaluator*					evaluator_;

		KeyIndexMap					kinetic_map_;

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


class ARVineyardBase::MembershipFunctionChangeEvent
{
	public:
									MembershipFunctionChangeEvent(Key k, F function, 
																  ActivePointsTable::Handle apt):
										key_(k), function_(function), apt_(apt)					{}
		
		void						process(Simulator::Time t) const;
		std::ostream&				operator<<(std::ostream& out) const;

	private:
		Key							key_;
		F							function_;
		ActivePointsTable::Handle	apt_;
};

std::ostream& operator<<(std::ostream& out, const ARVineyardBase::MembershipFunctionChangeEvent& e)
{ return e.operator<<(out); }

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


class ARVineyardBase::SortVisitor: public CGAL::Kinetic::Sort_visitor_base
{
	public:
									SortVisitor(ARVineyard* arv): arv_(arv)						{}

		template<class Vertex_handle>
		void						before_swap(Vertex_handle a, Vertex_handle b) const;

	private:
		ARVineyard*					arv_;
};


#include "ar-vineyard.hpp"

#endif // __AR_VINEYARD_H__
