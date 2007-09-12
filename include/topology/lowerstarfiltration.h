/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2006
 */

#ifndef __LOWERSTARFILTRATION_H__
#define __LOWERSTARFILTRATION_H__

#include "utilities/sys.h"
#include "utilities/debug.h"

#include "filtration.h"
#include "simplex.h"
#include "utilities/consistencylist.h"
#include <boost/utility.hpp>
#include <list>
#include "utilities/types.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>


template<class VI, 
		 class Smplx = SimplexWithAttachment<VI>, 
		 class FltrSmplx = FiltrationSimplex<Smplx>,
		 class Vnrd = Vineyard<FltrSmplx> >
class LowerStarFiltration: public Filtration<Smplx, FltrSmplx, Vnrd>
{
	public:
		// Treat VertexIndex as an iterator
		typedef					VI													VertexIndex;		
		typedef					Smplx												Simplex;
		typedef					Filtration<Simplex>									Parent;
		typedef					typename Parent::Vineyard							Vineyard;

		typedef					typename Parent::Index								Index;
		typedef					typename Parent::const_Index						const_Index;
		typedef					typename Parent::Cycle								Cycle;
		typedef					typename Parent::Trail								Trail;
		typedef					typename Simplex::Cycle 							SimplexBoundaryCycle;

		struct 					VertexDescriptor;
		typedef					ConsistencyList<VertexDescriptor>					VertexOrder;
		typedef					typename VertexOrder::iterator						VertexOrderIndex;
		typedef					typename VertexOrder::const_iterator				const_VertexOrderIndex;
		typedef 				typename VertexOrder::LessThanComparison			VertexOrderComparison;
		struct					SimplexAttachmentComparison;

	public:
								template<class VertexCmp>							
								LowerStarFiltration(VertexIndex begin, VertexIndex end, const VertexCmp& cmp, Vineyard* vineyard);

		using 					Parent::size;
		using 					Parent::begin;
		using 					Parent::end;
		VertexIndex				num_vertices() const								{ return vertex_order.size(); }
		const VertexOrderComparison& 
								get_vertex_cmp() const								{ return vertex_cmp; }
		
		Index 					append(const Simplex& s);
		bool					transpose_vertices(const VertexOrderIndex& voi);

	protected:
		/// Hint function: if unsure, should return true
		virtual bool			neighbors(VertexIndex v1, VertexIndex v2) const		{ return true; }

	private:
		bool 					transpose(Index i);
		void					assert_pairing(Index i);
		
	private:
		VertexOrder				vertex_order;	
		VertexOrderComparison	vertex_cmp;
	
	/* Serialization */
	protected:
		LowerStarFiltration()														{}
		
	private:
		friend class boost::serialization::access;
			
		template<class Archive>
		void save(Archive& ar, version_type ) const									{ ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Parent); }

		template<class Archive>
		void load(Archive& ar, version_type );

		BOOST_SERIALIZATION_SPLIT_MEMBER()
};

template<class VI, class Smplx, class FltrSmplx, class Vnrd>
struct LowerStarFiltration<VI,Smplx,FltrSmplx,Vnrd>::VertexDescriptor
{
	VertexDescriptor(VertexIndex vi, Index si): 
		vertex_index(vi), simplex_index(si)		
	{}
	
	VertexIndex			vertex_index;
	Index				simplex_index;
};

template<class VI, class Smplx, class FltrSmplx, class Vnrd>
struct LowerStarFiltration<VI,Smplx,FltrSmplx,Vnrd>::SimplexAttachmentComparison
{
	bool operator()(const Simplex& first, const Simplex& second) const;
	VertexOrderComparison	vertex_cmp;
};

#include "lowerstarfiltration.hpp"

#endif // __LOWERSTARFILTRATION_H__
