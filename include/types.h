#ifndef __TYPES_H__
#define __TYPES_H__

#include <iostream>

/* Types */
typedef 	bool					Sign;
typedef		short int				Dimension;
const 		Sign	 				POS = true;
const 		Sign					NEG = false;
typedef		double					RealType;
typedef		unsigned int			SizeType;

typedef 	const unsigned int&		version_type;


/**
 * Rational number type
 */
template<typename T>
class Rational
{
	public:
		typedef			Rational<T>			Self;
	
		Rational(T v);
		Rational(const Self& other);

		Self& operator/=(const Self& rhs);

		/// \name Comparisons
		/// Assume denominator is positive.
		/// @{
		bool operator<(const Self& rhs) const;
		bool operator<=(const Self& rhs) const;
		bool operator>(const Self& rhs) const;
		bool operator>=(const Self& rhs) const;
		/// @}
		
		Self& operator=(const Self& rhs);
		RealType to_real() const;

		std::ostream& operator<<(std::ostream& out) const;

	private:
		T numerator, denominator;
};

#include "types.hpp"

#endif // __TYPES_H__
