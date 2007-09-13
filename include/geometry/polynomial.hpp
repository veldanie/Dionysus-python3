template<class T>
void
UPolynomial<T>::
solve(const RationalFunction& rf, RootStack& stack)
{
	typedef SYNAPS::Seq<RootType>		RootSeq;

	RootSeq seq_num = SYNAPS::solve(rf.numerator(), Solver());
	RootSeq seq_den = SYNAPS::solve(rf.denominator(), Solver());

	// TODO: assert that all roots in seq_den have positive multiplicity
	// TODO: deal with multiplicities for the numerator
	for (typename RootSeq::const_reverse_iterator cur = seq_num.rbegin(); 
												  cur != seq_num.rend(); 
												  ++cur)
	{
		if (SynapsTraits<T>::multiplicity(*cur) % 2 != 0)
		{		
			if (!stack.empty() && stack.top() == *cur)			// get rid of even multiplicities
				// TODO: add logging information for this
				stack.pop();
			else
				stack.push(*cur);
		}
	}
}

template<class T>
int
UPolynomial<T>::
sign_at(const RationalFunction& rf, const RootType& r)
{
	return SynapsTraits<T>::sign_at(rf.numerator(), r) * SynapsTraits<T>::sign_at(rf.denominator(), r);
}
