/* Implementations */

template<typename T>
Rational<T>::
Rational(T v): 
	numerator(v), denominator(1)					
{}

template<typename T>
Rational<T>::
Rational(const Self& other): 
	numerator(other.numerator), denominator(other.denominator)
{}

template<typename T>
typename Rational<T>::Self& 
Rational<T>::
operator/=(const Self& rhs)
{ 
	numerator *= rhs.denominator; 
	denominator *= rhs.numerator; 

	if (denominator < 0)
	{
		numerator = -numerator;
		denominator = -denominator;
	}
	
	return *this; 
}

template<typename T>
bool 
Rational<T>::
operator<(const Self& rhs) const
{ return (numerator * rhs.denominator < denominator * rhs.numerator); }

template<typename T>
bool 
Rational<T>::
operator<=(const Self& rhs) const
{ return (numerator * rhs.denominator <= denominator * rhs.numerator); }

template<typename T>
bool 
Rational<T>::
operator>(const Self& rhs) const
{ return rhs < (*this); }

template<typename T>
bool 
Rational<T>::
operator>=(const Self& rhs) const
{ return rhs <= (*this); }

template<typename T>
typename Rational<T>::Self& 
Rational<T>::
operator=(const Self& rhs)
{ numerator = rhs.numerator; denominator = rhs.denominator; return *this; }

template<typename T>
std::ostream& 
Rational<T>::
operator<<(std::ostream& out) const
{ out << numerator << " / " << denominator << " = " << (numerator/denominator); return out; }

template<typename T>
RealType 
Rational<T>::
to_real() const
{ return numerator/denominator; }

		
template<typename T>
std::ostream& operator<<(std::ostream& out, const Rational<T>& r)
{
	return r.operator<<(out);
}
