#ifndef __POLYNOMIAL_H__
#define __POLYNOMIAL_H__

#include <synaps/upol.h>
#include <synaps/upol/gcd.h>
#include <synaps/usolve/Algebraic.h>
#include <synaps/usolve/algebraic/sign_fct.h>
#include <synaps/usolve/bezier/SlvBzStd.h>
//#include <synaps/usolve/Sturm.h>
//#include <synaps/arithm/Infinity.h>

#include <stack>

#include "rational-function.h"

template<class T> class SynapsTraits;

template<class T>
class UPolynomial
{
	public:
		typedef						typename SynapsTraits<T>::Polynomial				Polynomial;
		typedef						RationalFunction<Polynomial>						RationalFunction;

		typedef						typename SynapsTraits<T>::Solver					Solver;
		typedef						typename SynapsTraits<T>::RootType					RootType;
		typedef						std::stack<RootType>								RootStack;

		static void					solve(const RationalFunction& rf, RootStack& stack);
		static RootType				root(const T& r)									{ return SynapsTraits<T>::root(r); }
		static int					sign_at(const RationalFunction& rf, const RootType& r);
		static RootType				between(const RootType& r1, const RootType& r2)		{ return SynapsTraits<T>::between(r1,r2); }
};

template<class T>
struct SynapsTraits				///< Suitable for double
{
		typedef						T													CoefficientType;
		typedef						SYNAPS::UPolDse<CoefficientType>					Polynomial;
		typedef						SYNAPS::SlvBzStd<CoefficientType>					Solver;
		typedef						T													RootType;

		static RootType				root(CoefficientType r)								{ return r; }
		static unsigned int			multiplicity(RootType r)							{ return 1; }
		static int					sign_at(const Polynomial& p, RootType r)			{ return SYNAPS::UPOLDAR::sign_at(p, r); }
		static RootType				between(RootType r1, RootType r2)					{ return (r1 + r2)/2; }
};

template<>
struct SynapsTraits<ZZ>
{
		typedef						ZZ													CoefficientType;
		typedef						SYNAPS::UPolDse<CoefficientType>					Polynomial;
		typedef						SYNAPS::Algebraic<CoefficientType>					Solver;
		typedef						Solver::root_t										RootType;

		static RootType				root(const CoefficientType& r)						{ CoefficientType p[2] = {-r, 1}; return SYNAPS::solve(Polynomial(2, p), Solver(), 0);}
		static unsigned int			multiplicity(const RootType& r)						{ return r.multiplicity(); }
		static int					sign_at(const Polynomial& p, 
											const RootType& r)							{ return SYNAPS::ALGEBRAIC::sign_at(p, r); }
		//static RootType				between(const RootType& r1, const RootType& r2) 	{ RootType r = r1; r += r2; r /= root(2); return r; }
};

#include "polynomial.hpp"

#endif
