/**
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2006
 */

#ifndef __ALPHASHAPES3D_H__
#define __ALPHASHAPES3D_H__

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>

#include <simplex.h>
#include <types.h>

#include <vector>
#include <set>
#include <iostream>

struct K: CGAL::Exact_predicates_exact_constructions_kernel {};

typedef CGAL::Delaunay_triangulation_3<K>    		Delaunay;
typedef Delaunay::Point                				Point;
typedef Delaunay::Vertex            				Vertex;
typedef Delaunay::Vertex_handle            			Vertex_handle;
typedef Delaunay::Edge								Edge;
typedef Delaunay::Facet								Facet;
typedef Delaunay::Cell								Cell;
typedef Delaunay::Cell_handle						Cell_handle;
typedef K::FT										RealValue;

typedef Delaunay::Finite_vertices_iterator    		Vertex_iterator;
typedef Delaunay::Finite_edges_iterator        		Edge_iterator;
typedef Delaunay::Finite_facets_iterator        	Facet_iterator;
typedef Delaunay::Finite_cells_iterator        		Cell_iterator;
typedef Delaunay::Facet_circulator					Facet_circulator;


class AlphaSimplex3D: public SimplexWithVertices<Vertex_handle>
{
	public:
		typedef 	std::set<AlphaSimplex3D>							SimplexSet;
		typedef		SimplexWithVertices<Vertex_handle>					Parent;
		typedef		Parent::VertexContainer								VertexSet;
		typedef		std::list<AlphaSimplex3D>							Cycle;

    public:
									AlphaSimplex3D()					{}
									AlphaSimplex3D(const Parent& p): 
											Parent(p) 					{}
									AlphaSimplex3D(const AlphaSimplex3D& s): 
											Parent(s) 					{ attached_ = s.attached_; alpha_ = s.alpha_; }
	    							AlphaSimplex3D(const ::Vertex& v);
		
								    AlphaSimplex3D(const Edge& e);
								    AlphaSimplex3D(const Edge& e, const SimplexSet& simplices, Facet_circulator facet_bg);
		
								    AlphaSimplex3D(const Facet& f);
								    AlphaSimplex3D(const Facet& f, const SimplexSet& simplices);
	    
									AlphaSimplex3D(const Cell& c);
	    
		RealType					value() const						{ return CGAL::to_double(alpha_); }
		RealValue					alpha() const						{ return alpha_; }
		bool						attached() const					{ return attached_; }
		Cycle						boundary() const;

		// Ordering
		struct AlphaOrder
		{ bool operator()(const AlphaSimplex3D& first, const AlphaSimplex3D& second) const; };
		
		std::ostream& 				operator<<(std::ostream& out) const;
		
	private:
		RealValue 					alpha_;
		bool 						attached_;
};

typedef 			std::vector<AlphaSimplex3D>								AlphaSimplex3DVector;
void 				fill_alpha_order(const Delaunay& Dt, 
									 AlphaSimplex3DVector& alpha_order);

std::ostream& 		operator<<(std::ostream& out, const AlphaSimplex3D& s)	{ return s.operator<<(out); }

#include "alphashapes3d.hpp"

#endif // __ALPHASHAPES3D_H__
