/**
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2006
 */

#ifndef __ALPHASHAPES3D_H__
#define __ALPHASHAPES3D_H__

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>

#include <topology/simplex.h>
#include <utilities/types.h>

#include <vector>
#include <set>
#include <iostream>

struct K: CGAL::Exact_predicates_exact_constructions_kernel {};

typedef CGAL::Delaunay_triangulation_3<K>    		Delaunay;
typedef Delaunay::Point                				Point;
typedef Delaunay::Vertex            				Vertex;
typedef Delaunay::Vertex_handle            			Vertex_handle;
typedef Delaunay::Cell_handle						Cell_handle;
typedef K::FT										RealValue;

typedef Delaunay::Finite_vertices_iterator    		Vertex_iterator;
typedef Delaunay::Finite_edges_iterator        		Edge_iterator;
typedef Delaunay::Finite_facets_iterator        	Facet_iterator;
typedef Delaunay::Finite_cells_iterator        		Cell_iterator;
typedef Delaunay::Facet_circulator					Facet_circulator;


class AlphaSimplex3D: public Simplex<Vertex_handle>
{
	public:
		typedef		Simplex<Vertex_handle>					            Parent;
		typedef 	std::set<AlphaSimplex3D, Parent::VertexComparison>  SimplexSet;
		typedef		Parent::VertexContainer								VertexSet;
		typedef		std::list<AlphaSimplex3D>							Boundary;

    public:
									AlphaSimplex3D()					{}
									AlphaSimplex3D(const Parent& p): 
											Parent(p) 					{}
									AlphaSimplex3D(const AlphaSimplex3D& s): 
											Parent(s) 					{ attached_ = s.attached_; alpha_ = s.alpha_; }
	    							AlphaSimplex3D(const ::Vertex& v);
		
								    AlphaSimplex3D(const Delaunay::Edge& e);
								    AlphaSimplex3D(const Delaunay::Edge& e, const SimplexSet& simplices, const Delaunay& Dt, Facet_circulator facet_bg);
		
								    AlphaSimplex3D(const Delaunay::Facet& f);
								    AlphaSimplex3D(const Delaunay::Facet& f, const SimplexSet& simplices, const Delaunay& Dt);
	    
									AlphaSimplex3D(const Delaunay::Cell& c);
	    
		RealType					value() const						{ return CGAL::to_double(alpha_); }
		RealValue					alpha() const						{ return alpha_; }
		bool						attached() const					{ return attached_; }

		// Ordering
		struct AlphaOrder
		{ bool operator()(const AlphaSimplex3D& first, const AlphaSimplex3D& second) const; };
	
        struct AlphaValueEvaluator
        { 
            typedef                 AlphaSimplex3D                                  first_argument_type;
            typedef                 RealType                                        result_type;

            RealType                operator()(const AlphaSimplex3D& s) const       { return s.value(); }
        };

		std::ostream& 				operator<<(std::ostream& out) const;
		
	private:
		RealValue 					alpha_;
		bool 						attached_;
};

typedef 			std::vector<AlphaSimplex3D>								AlphaSimplex3DVector;
void 				fill_complex(const Delaunay& Dt, 
								 AlphaSimplex3DVector& alpha_order);

std::ostream& 		operator<<(std::ostream& out, const AlphaSimplex3D& s)	{ return s.operator<<(out); }

#include "alphashapes3d.hpp"

#endif // __ALPHASHAPES3D_H__
