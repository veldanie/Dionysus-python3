/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2006
 */

#ifndef __GRID2DVINEYARD_H__
#define __GRID2DVINEYARD_H__

#include "sys.h"
#include "debug.h"

#include "grid2D.h"
#include "lowerstarfiltration.h"

#include <CGAL/Kinetic/Inexact_simulation_traits_1.h>
#include <CGAL/Kinetic/Sort.h>
#include <CGAL/Kinetic/Sort_visitor_base.h>

#include <vector>


class Grid2DVineyard
{
	public:
		typedef					Grid2DVineyard										Self;
			
		class					VertexType;
		typedef					std::vector<VertexType>								VertexVector;
		typedef					VertexVector::iterator								VertexIndex;
		
		typedef					LowerStarFiltration<VertexIndex>					LSFiltration; 
		
		class					StaticEvaluator;
		class					KineticEvaluator;
		class 					VertexComparison;
		
		typedef					Grid2D::CoordinateIndex								CoordinateIndex;
		typedef					Grid2D::ValueType									ValueType;

		typedef					LSFiltration::Index									Index;
		typedef					LSFiltration::Simplex								Simplex;
		typedef					LSFiltration::VertexOrderIndex						VertexOrderIndex;
		typedef					LSFiltration::Vineyard								Vineyard;
		typedef					Vineyard::Evaluator									Evaluator;

		class					SortVisitor;
		typedef 				CGAL::Kinetic::Inexact_simulation_traits_1 			Traits;
		typedef					CGAL::Kinetic::Sort<Traits, SortVisitor>			Sort;
		typedef 				Traits::Simulator 									Simulator;
		typedef 				Traits::Active_points_1_table						ActivePointsTable;
		typedef 				ActivePointsTable::Key								Key;
		typedef					std::map<Key, VertexOrderIndex>						KeyOrderMap;

		typedef					std::vector<Grid2D*>								GridStackContainer;

	public:
								Grid2DVineyard(Grid2D* g);
								~Grid2DVineyard();

		void					compute_pairing();
		void					compute_vineyard(Grid2D* grid);
		
		Grid2D*					grid() const										{ return grid_stack_.back(); }
		Grid2D*					grid(int i) const									{ return grid_stack_[i]; }
		int						num_grids() const									{ return grid_stack_.size(); }
		const LSFiltration*		filtration() const									{ return filtration_; }
		const Vineyard*			vineyard() const									{ return vineyard_; }

	public:
		// For Kinetic Sort
		void 					swap(Key a, Key b);
	
	protected:
		// Do something cleverer
		virtual bool			neighbors(VertexIndex v1, VertexIndex v2) const		{ return true; }
		
	private:
		void 					add_simplices();
		void					change_evaluator(Evaluator* eval);

	private:
		GridStackContainer		grid_stack_;
		VertexVector			vertices_;
		LSFiltration*			filtration_;
		Vineyard*				vineyard_;
		Evaluator*				evaluator_;

		KeyOrderMap				kinetic_map_;
				
#if 0
	private:
		// Serialization
		friend class boost::serialization::access;
		
		Grid2DVineyard() 																	{}

		template<class Archive> 
		void serialize(Archive& ar, version_type )
		{ 
			ar & BOOST_SERIALIZATION_NVP(grid_stack_); 
			ar & BOOST_SERIALIZATION_NVP(vertices_); 
			ar & BOOST_SERIALIZATION_NVP(filtration_); 
		};
#endif
};

//BOOST_CLASS_EXPORT(Grid2DVineyard)
	
class Grid2DVineyard::VertexType
{
	public:
		VertexType(CoordinateIndex ii = 0): i_(ii)											{}
		
		CoordinateIndex			index() const												{ return i_; }
		void					set_index(CoordinateIndex i)								{ i_ = i; }
		VertexOrderIndex		get_order() const											{ return order_; }
		void					set_order(const VertexOrderIndex& o)						{ order_ = o; }
		
		Key						kinetic_key() const											{ return key_; }
		void					set_kinetic_key(Key k)										{ key_ = k; }
		
	private:
		CoordinateIndex			i_;
		VertexOrderIndex		order_;
		Key						key_;
};

std::ostream& operator<<(std::ostream& out, const Grid2DVineyard::VertexIndex& vi)			{ return out << vi->index(); }

class Grid2DVineyard::VertexComparison
{
	public:
		VertexComparison(const Grid2D* g): grid(g)											{}
		bool operator()(VertexIndex i, VertexIndex j) const									{ return (*grid)(i->index()) < 
																									 (*grid)(j->index()); }

	private:
		const Grid2D*			grid;

#if 0
	private:
		// Serialization
		friend class boost::serialization::access;

								VertexComparison()											{}

		template<class Archive>
		void 					serialize(Archive& ar, version_type )						{ ar & BOOST_SERIALIZATION_NVP(grid); }
#endif
};

class Grid2DVineyard::StaticEvaluator: public Evaluator
{
	public:
								StaticEvaluator(Grid2D* grid, RealType time): 
									time_(time), grid_(grid)								{}

		virtual RealType		time()														{ return time_; }
		virtual RealType		value(const Simplex& s)										{ return (*grid_)(s.get_attachment()->index()); }
								
	private:
		RealType				time_;
		Grid2D*					grid_;
};

class Grid2DVineyard::KineticEvaluator: public Evaluator
{
	public:
								KineticEvaluator(Simulator::Handle sp, ActivePointsTable::Handle apt, RealType time_offset): 
									sp_(sp), apt_(apt), time_offset_(time_offset)			{}

		virtual RealType		time()														{ return time_offset_ + CGAL::to_double(get_time()); }
		virtual RealType		value(const Simplex& s)										{ return CGAL::to_double(apt_->at(s.get_attachment()->kinetic_key()).x()(get_time())); }

	private:
		Simulator::Time			get_time()													{ return sp_->current_time(); }
		
		Simulator::Handle			sp_;
		ActivePointsTable::Handle 	apt_;
		RealType					time_offset_;
};


class Grid2DVineyard::SortVisitor: public CGAL::Kinetic::Sort_visitor_base
{
	public:
								SortVisitor(Grid2DVineyard* gv): gv_(gv)					{}

		template<class Vertex_handle>
		void					before_swap(Vertex_handle a, Vertex_handle b) const			{ gv_->swap(*a,*b); }

	private:
		Grid2DVineyard*			gv_;
};

#include "grid2Dvineyard.hpp"

#endif // __GRID2DVINEYARD_H__
