/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2006
 */

#ifndef __VINEYARD_H__
#define __VINEYARD_H__

#include "utilities/types.h"
#include <list>
#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/is_abstract.hpp>
	

template<class Smplx>	class Knee;
template<class Smplx>	class Vine;

/**
 * Vineyard class. Keeps track of vines and knees. switched() is the key function called
 * by filtration when pairing switches after a Filtration::transpose().
 */
template<class FltrSmplx>
class Vineyard
{
	public:
		typedef							FltrSmplx									FiltrationSimplex;
		typedef							typename FiltrationSimplex::Simplex			Simplex;
		typedef							Vine<Simplex>								Vine;
		typedef							Knee<Simplex>								Knee;
		typedef							std::list<Vine>								VineList;
		typedef							std::vector<VineList>						VineListVector;
		typedef							typename FiltrationSimplex::Cycle			Cycle;

		typedef							typename FiltrationSimplex::Index			Index;
		typedef							typename FiltrationSimplex::Evaluator		Evaluator;
										
	public:
										Vineyard(Evaluator* eval = 0): 
											evaluator(eval)							{}

		void							start_vines(Index bg, Index end);
		void							switched(Index i, Index j);
		void							record_diagram(Index bg, Index end);

		void							set_evaluator(Evaluator* eval)				{ evaluator = eval; }

		void							save_edges(const std::string& filename) const;

	protected:
		typename Knee::SimplexList  	resolve_cycle(Index i) const;

	private:
		void							record_knee(Index i);
		void							start_vine(Index i);

	private:
		VineListVector					vines;
		Evaluator*						evaluator;
};

/**
 * Knee class stores the knee in R^3 as well as the cycle that is associated with the Simplex starting from the Knee.
 */
template<class S>
class Knee
{
	public:
		typedef					S												Simplex;
		typedef					std::list<Simplex>								SimplexList;
	
		RealType				birth;
		RealType				death;
		RealType				time;
		SimplexList				cycle;
			
								// Default parameters for serialization
								Knee(RealType b = 0, RealType d = 0, RealType t = 0):
									birth(b), death(d), time(t)
								{}
								Knee(const Knee& other): 
									birth(other.birth), death(other.death), time(other.time)
								{}

		bool 					is_diagonal() const								{ return birth == death; }
		bool					is_infinite() const								{ return (death == Infinity) || (birth == Infinity); }
		void 					set_cycle(const SimplexList& lst)				{ cycle = lst; }

		std::ostream&			operator<<(std::ostream& out) const				{ return out << "(" << birth << ", " 
																									<< death << ", " 
																									<< time  << ")"; }
	
	private:
		friend class boost::serialization::access;

		template<class Archive>
		void 					serialize(Archive& ar, version_type );
};

template<class S>
std::ostream& operator<<(std::ostream& out, const Knee<S>& k) 					{ return k.operator<<(out); }

/**
 * Vine is a list of Knees
 */
template<class S>
class Vine: public std::list<Knee<S> >
{	
	public:
		typedef					S												Simplex;
		typedef					Knee<Simplex>									Knee;
		typedef					std::list<Knee>									VineRepresentation;
		typedef					typename VineRepresentation::const_iterator		const_knee_iterator;
		
								Vine()											{}
								Vine(const Knee& k)								{ add(k); }
		
		void 					add(RealType b, RealType d, RealType t)			{ push_back(Knee(b,d,t)); }
		void 					add(const Knee& k)								{ push_back(k); }

		using VineRepresentation::begin;
		using VineRepresentation::end;
		using VineRepresentation::front;
		using VineRepresentation::back;
		using VineRepresentation::size;
		using VineRepresentation::empty;

	protected:
		using VineRepresentation::push_back;

	private:
		friend class boost::serialization::access;

		template<class Archive>
		void 					serialize(Archive& ar, version_type );
};


#include "vineyard.hpp"

#endif // __VINEYARD_H__