/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2006
 */

#ifndef __FILTRATIONSIMPLEX_H__
#define __FILTRATIONSIMPLEX_H__

#include "sys.h"
#include "debug.h"

#include "filtrationcontainer.h"
#include "vineyard.h"
#include "types.h"

#include <list>

#if 0
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#endif

/**
 * Evaluator is a base class for the structures that are able to return a value
 * given a simplex.
 */
template<class Smplx>
class Evaluator
{
	public:
		typedef					Smplx										Simplex;

		virtual RealType		time() const								{ return 0; }
		virtual RealType		value(const Simplex& s) const				{ return 0; }

		virtual					~Evaluator()								{}
};

/**
 * FiltrationSimplex stores information needed for the RU-decomposition: 
 * cycle (column of R), trail (row of U), and pair.
 */
template<class Smplx>
class FiltrationSimplex: public Smplx
{
	public:
		typedef		Smplx													Simplex;
		typedef		FiltrationSimplex<Simplex>								Self;
		typedef		FiltrationContainer<Self>								Container;
		typedef		Simplex													Parent;
		
		typedef		Vine<Simplex>											Vine;
		typedef		typename Container::Cycle								Cycle;
		typedef		typename Container::Trail								Trail;
		typedef		typename Container::Index								Index;

		typedef		Evaluator<Simplex>										Evaluator;
		
		FiltrationSimplex(const Simplex& s): 
			Simplex(s), vine_(0)											{}
		

		/// \name Core functionality
		/// @{
		void					set_pair(Index pair)						{ pair_ = pair; }
		bool					sign() const								{ return cycles_column.empty(); }
		bool					is_paired() const							{ return pair() != pair()->pair(); }
		void					set_vine(Vine* v)							{ vine_ = v; }
		using 					Parent::dimension;
		/// @}


		/// \name Accessors
		/// @{
		Cycle&					cycle()										{ return cycles_column; }
		Trail&					trail()										{ return trails_row; }
		const Cycle&			cycle()	const								{ return cycles_column; }
		const Trail&			trail()	const								{ return trails_row; }
		Index					pair() const								{ return pair_; }
		Vine*					vine() const								{ return vine_; }
		/// @}

	private:
		Cycle																cycles_column;
		Trail																trails_row; 
		Index																pair_;
		Vine*																vine_;
};

#endif // __FILTRATIONSIMPLEX_H__
