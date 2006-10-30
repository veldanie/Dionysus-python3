/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2006
 */

#ifndef __FILTRATIONSIMPLEX_H__
#define __FILTRATIONSIMPLEX_H__

#include "sys.h"
#include "debug.h"

#include "filtrationcontainer.h"

#include <list>

#if 0
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#endif

/**
 * FiltrationSimplex stores information needed for the RU-decomposition: 
 * cycle (column of R), trail (row of U), and pair.
 */
template<class Smplx, class ContainerSimplex = void>
class FiltrationSimplex: public Smplx
{
	public:
		typedef		Smplx													Simplex;
		typedef		FiltrationSimplex<Simplex>								Self;
		typedef		FiltrationContainer<ContainerSimplex>					Container;
		typedef		Simplex													Parent;
		
		typedef		typename Container::Cycle								Cycle;
		typedef		typename Container::Trail								Trail;
		typedef		typename Container::Index								Index;

		
		FiltrationSimplex(const Simplex& s): 
			Simplex(s)														{}
		

		/// \name Core functionality
		/// @{
		void					set_pair(Index pair)						{ pair_ = pair; }
		bool					sign() const								{ return cycles_column.empty(); }
		using Parent::dimension;
		/// @}


		/// \name Accessors
		/// @{
		Cycle&					cycle()										{ return cycles_column; }
		Trail&					trail()										{ return trails_row; }
		const Cycle&			cycle()	const								{ return cycles_column; }
		const Trail&			trail()	const								{ return trails_row; }
		Index					pair() const								{ return pair_; }
		/// @}

	private:
		Cycle																cycles_column;
		Trail																trails_row; 
		Index																pair_;
};

/** Specialization for ContainerSimplex = void */
template<class Smplx>
class FiltrationSimplex<Smplx, void> : public Smplx
{
	public:
		typedef		Smplx													Simplex;
		typedef		FiltrationSimplex<Simplex>								Self;
		typedef		FiltrationContainer<Self>								Container;
		typedef		Simplex													Parent;
		
		typedef		typename Container::Cycle								Cycle;
		typedef		typename Container::Trail								Trail;
		typedef		typename Container::Index								Index;

		
		FiltrationSimplex(const Simplex& s): 
			Simplex(s)														{}
		

		/// \name Core functionality
		/// @{
		void					set_pair(Index pair)						{ pair_ = pair; }
		bool					sign() const								{ return cycles_column.empty(); }
		using Parent::dimension;
		/// @}


		/// \name Accessors
		/// @{
		Cycle&					cycle()										{ return cycles_column; }
		Trail&					trail()										{ return trails_row; }
		const Cycle&			cycle()	const								{ return cycles_column; }
		const Trail&			trail()	const								{ return trails_row; }
		Index					pair() const								{ return pair_; }
		/// @}

	private:
		Cycle																cycles_column;
		Trail																trails_row; 
		Index																pair_;
};

#if 0			// FIXME
template<class S>
class Filtration<S>::FiltrationSimplexSerialization: public Simplex
{
	public:
		typedef		std::list<IntegerIndex>									IntegerIndexList;
		
		// Default constructor for serialization
		FiltrationSimplexSerialization()										{}
		
		FiltrationSimplexSerialization(const FiltrationSimplex& fs, const IndexIntMap& im):
			Simplex(fs)
		{
			pairing = im.find(fs.pair())->second;
			for (typename FiltrationCycle::const_iterator 	cur = fs.cycle().begin(); 
															cur != fs.cycle().end();
															++cur)
			{ 	cycle.push_back(im.find(*cur)->second);	}
			
			for (typename FiltrationTrail::const_iterator 	cur = fs.trail().begin(); 
															cur != fs.trail().end();
															++cur)
			{	trail.push_back(im.find(*cur)->second);	}			

			vine = fs.get_vine();
		}

		void set_filtration_simplex(FiltrationSimplex& s, const IndexVector& index_vector) const
		{
			s = *this;
			
			s.pair_with(index_vector[pairing]);

			// Just in case
			s.cycles_column.clear();
			s.trails_row.clear();

			for (IntegerIndexList::const_iterator cur = cycle.begin(); cur != cycle.end(); ++cur)
			{	s.cycles_column.append(index_vector[*cur]);	}
			for (IntegerIndexList::const_iterator cur = trail.begin(); cur != trail.end(); ++cur)
			{	s.trails_row.append(index_vector[*cur]);	}

			s.set_vine(vine);
		}

	private:
		IntegerIndexList													cycle;
		IntegerIndexList													trail;
		IntegerIndex														pairing;
		Vine* 																vine;
	
	private:
		// Serialization
		friend class boost::serialization::access;
		
		template<class Archive>
		void serialize(Archive& ar, version_type )
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Simplex);
			
			ar & BOOST_SERIALIZATION_NVP(cycle);
			ar & BOOST_SERIALIZATION_NVP(trail);
			ar & BOOST_SERIALIZATION_NVP(pairing);
			ar & BOOST_SERIALIZATION_NVP(vine);
		}
};
#endif

#endif // __FILTRATIONSIMPLEX_H__
