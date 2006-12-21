/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2006
 */

#ifndef __SIMPLEX_H__
#define __SIMPLEX_H__

#include "sys.h"
#include "debug.h"

#include <set>
#include <list>
#include <iostream>

#include "types.h"

#include <boost/serialization/access.hpp>


/**
 * SimplexWithVertices is a basic simplex class. It stores vertices of a given type, 
 * and knows how to compute its own boundary. It should probably be used as a base 
 * class for any explicit simplex representation.
 */
template<class V>
class SimplexWithVertices
{
	public:
		typedef		V																Vertex;
		typedef		SimplexWithVertices<Vertex>										Self;
	
		typedef		std::set<Vertex>												VertexContainer;
		typedef		std::list<Self>													Cycle;
		
		/// \name Constructors 
		/// @{
		SimplexWithVertices()														{}
		SimplexWithVertices(const Self& s):	
			vertices_(s.vertices_)													{}
		template<class Iterator>
		SimplexWithVertices(Iterator bg, Iterator end):
			vertices_(bg, end)														{}
		SimplexWithVertices(const VertexContainer& v):	
			vertices_(v)															{}
		SimplexWithVertices(Vertex v):	
			vertices_()																{ vertices_.insert(v); }
		/// @}
		
		/// \name Core 
		/// @{
		Cycle					boundary() const;
		Dimension				dimension() const									{ return vertices_.size()-1; }
		/// @}
		
		/// \name Vertex manipulation
		/// @{
		bool					contains(const Vertex& v) const						{ return (vertices_.find(v) != vertices_.end()); }
		const VertexContainer&	vertices() const									{ return vertices_; }
		void					add(const Vertex& v) 								{ vertices_.insert(v); }
		/// @}

		/// \name Assignment and comparison
		/// Gives an ordering on simplices (for example, so that simplices can be used as keys for std::map)
		/// @{
		const Self&				operator=(const Self& s)							{ vertices_ = s.vertices_; return *this; }
		bool					operator==(const Self& s) const						{ return vertices_ == s.vertices_; }
		bool 					operator<(const Self& s) const						{ return vertices_ < s.vertices_; }
		/// @}

		std::ostream&			operator<<(std::ostream& out) const;
	
	private:
		VertexContainer			vertices_;

	private:
		/* Serialization */
		friend class boost::serialization::access;
		
		template<class Archive>
		void 					serialize(Archive& ar, version_type );
};

/**
 * SimplexWithValue explicitly adds a RealType value to the SimplexWithVertices.
 */
template<class Vert>
class SimplexWithValue: public SimplexWithVertices<Vert>
{
	public:
		typedef		Vert															Vertex;
		typedef		RealType														Value;
		typedef		SimplexWithValue<Vertex>										Self;
		typedef		SimplexWithVertices<Vertex>										Parent;

		typedef		typename Parent::VertexContainer								VertexContainer;
	
		/// \name Constructors
		/// @{
		SimplexWithValue(Value value = 0): val(value)								{}
		SimplexWithValue(const Self& s):
			Parent(s), val(s.val)													{}
		SimplexWithValue(const Parent& s, Value value = 0): 
			Parent(s), val(value)													{}
		template<class Iterator>
		SimplexWithValue(Iterator bg, Iterator end, Value value = 0):
			Parent(bg, end), val(value)												{}
		SimplexWithValue(const VertexContainer& v, Value value = 0):
			Parent(v), val(value)													{}
		/// @}

		/// \name Core
		/// @{
		void 					set_value(Value value)								{ val = value; }
		Value					get_value() const									{ return val; }
		/// @}
		
		const Self&				operator=(const Self& s);
		std::ostream&			operator<<(std::ostream& out) const;

	private:
		Value					val;

		/* Serialization */
		friend class boost::serialization::access;
		
		template<class Archive>
		void 					serialize(Archive& ar, version_type );
};

/**
 * SimplexWithAttachment stores the vertex to which the simplex is attached (meant for lower-star filtrations)
 */
template<typename V>
class SimplexWithAttachment: public SimplexWithVertices<V>
{
	public:
		typedef 	V																VertexIndex;
		typedef		SimplexWithVertices<VertexIndex>								Parent;
	
		/// \name Constructors 
		/// @{
		SimplexWithAttachment():
			attachment(VertexIndex())												{}
		template<class Iterator>
		SimplexWithAttachment(Iterator bg, Iterator end):
			Parent(bg, end)															{}
		SimplexWithAttachment(const Parent& s):
			Parent(s)																{}
		SimplexWithAttachment(VertexIndex vi):
			Parent(vi), attachment(vi)												{}
		/// @}

		void 					set_attachment(VertexIndex v)						{ attachment = v; }
		VertexIndex				get_attachment() const								{ return attachment; }
		
	private:
		VertexIndex				attachment;
	
	private:
		// Serialization
		friend class boost::serialization::access;

		template<class Archive>
		void 					serialize(Archive& ar, version_type );
};

#include "simplex.hpp"

#endif // __SIMPLEX_H__
