/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2006
 */

#ifndef __VINEYARDSIMPLEX_H__
#define __VINEYARDSIMPLEX_H__

#include "types.h"

#include <boost/serialization/access.hpp>

template<class S> class Knee;
template<class S> class Vine;

/**
 * VineyardSimplex class stores a Vine as a list of VineFrames, each of which is a list of Knees.
 */
template<class Smplx>
class VineyardSimplex: public FiltrationSimplex<Smplx, VineyardSimplex<Smplx> >
{
	public:
		typedef					Smplx											Simplex;
		typedef					VineyardSimplex									Self;
		typedef 				FiltrationSimplex<Simplex, Self>				FiltrationSimplex;

		typedef					typename FiltrationSimplex::Container			Container;
		typedef					typename FiltrationSimplex::Index				Index;
			
		typedef					Knee<Simplex>									Knee;
		typedef					Vine<Simplex>									Vine;

								VineyardSimplex(const FiltrationSimplex& fs): 
									FiltrationSimplex(fs)						{}
		
		Vine*					vine() const									{ return vine_; }
		void					set_vine(Vine* v)								{ vine_ = v; }
		Knee*					add_knee(const Knee& k)							{ assert(vine_); return vine_->add(k); } 
		Knee*					new_frame(const Knee& k)						{ assert(vine_); return vine_->new_frame(k); }
		void					swap_vine(Index i)								{ std::swap(vine_, i->vine_); }

		using FiltrationSimplex::set_pair;
	
	private:
		Vine*																	vine_;
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

		bool 					is_diagonal() const								{ return birth == death; }
		void 					set_cycle(const SimplexList& lst)				{ cycle = lst; }
	
	private:
		friend class boost::serialization::access;

		template<class Archive>
		void 					serialize(Archive& ar, version_type );
};

/**
 * VineFrame is a list of Knees.
 */
template<class S>
class VineFrame: public std::list<Knee<S> >
{};


/**
 * Vine is a list of VineFrames.
 */
template<class S>
class Vine: public std::list<VineFrame<S> >
{	
	public:
		typedef					S												Simplex;
		typedef					Knee<Simplex>									Knee;
		typedef					VineFrame<Simplex>								VineFrame;
		typedef					std::list<VineFrame>							VineRepresentation;
		typedef					typename VineRepresentation::const_iterator		const_frame_iterator;
		
								Vine()											{}
		
		Knee* 					add(RealType b, RealType d, RealType t)			{ back().push_back(Knee(b,d,t)); return &(back().back()); }
		Knee* 					add(const Knee& k)								{ back().push_back(k); return &(back().back()); }
		Knee*					new_frame(const Knee& k)						{ push_back(VineFrame()); return add(k); }
		unsigned int			num_frames()	const							{ return size(); }

		// TODO: Do a proper frame_iterator here, and knee_iterator
		using VineRepresentation::begin;
		using VineRepresentation::end;
		using VineRepresentation::front;
		using VineRepresentation::back;
		using VineRepresentation::size;

	protected:
		using VineRepresentation::push_back;

	private:
		friend class boost::serialization::access;

		template<class Archive>
		void 					serialize(Archive& ar, version_type );
};

#include "vineyardsimplex.hpp"

#endif // __VINEYARDSIMPLEX_H__
