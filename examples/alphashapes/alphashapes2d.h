/**
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2007
 */

#ifndef __ALPHASHAPES2D_H__
#define __ALPHASHAPES2D_H__

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include <topology/simplex.h>
#include <utilities/types.h>

#include <vector>
#include <set>
#include <iostream>

struct K: CGAL::Exact_predicates_exact_constructions_kernel {};

typedef CGAL::Delaunay_triangulation_2<K>    		Delaunay;
typedef Delaunay::Point                				Point;
typedef Delaunay::Vertex            				Vertex;
typedef Delaunay::Vertex_handle            			Vertex_handle;
typedef Delaunay::Edge								Edge;
typedef Delaunay::Face								Face;
typedef Delaunay::Face_handle						Face_handle;
typedef K::FT										RealValue;

typedef Delaunay::Finite_vertices_iterator    		Vertex_iterator;
typedef Delaunay::Finite_edges_iterator        		Edge_iterator;
typedef Delaunay::Finite_faces_iterator        		Face_iterator;


class AlphaSimplex2D: public SimplexWithVertices<Vertex_handle>
{
	public:
		typedef 	std::set<AlphaSimplex2D>							SimplexSet;
		typedef		SimplexWithVertices<Vertex_handle>					Parent;
		typedef		Parent::VertexContainer								VertexSet;
		typedef		std::list<AlphaSimplex2D>							Cycle;

    public:
									AlphaSimplex2D()					{}
									AlphaSimplex2D(const Parent& p): 
											Parent(p) 					{}
									AlphaSimplex2D(const AlphaSimplex2D& s): 
											Parent(s) 					{ attached_ = s.attached_; alpha_ = s.alpha_; }
	    							AlphaSimplex2D(const ::Vertex& v);
		
								    AlphaSimplex2D(const Edge& e);
								    AlphaSimplex2D(const Edge& e, const SimplexSet& simplices);
		
									AlphaSimplex2D(const Face& c);
	    
		RealType					value() const						{ return CGAL::to_double(alpha_); }
		RealValue					alpha() const						{ return alpha_; }
		bool						attached() const					{ return attached_; }
		Cycle						boundary() const;

		// Ordering
		struct AlphaOrder
		{ bool operator()(const AlphaSimplex2D& first, const AlphaSimplex2D& second) const; };
		
		std::ostream& 				operator<<(std::ostream& out) const;
		
	private:
		RealValue 					alpha_;
		bool 						attached_;
};

typedef 			std::vector<AlphaSimplex2D>								AlphaSimplex2DVector;
void 				fill_alpha_order(const Delaunay& Dt, 
									 AlphaSimplex2DVector& alpha_order);

std::ostream& 		operator<<(std::ostream& out, const AlphaSimplex2D& s)	{ return s.operator<<(out); }

#include "alphashapes2d.hpp"

#endif // __ALPHASHAPES2D_H__
