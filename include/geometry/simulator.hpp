template<class PolyKernel_, template<class Event> class EventComparison_>
template<class Event_>
typename Simulator<PolyKernel_, EventComparison_>::Key
Simulator<PolyKernel_, EventComparison_>::
add(const Event_& e)
{
	Event* ee = new Event_(e);
	return queue_.push(ee);
}

template<class PolyKernel_, template<class Event> class EventComparison_>
template<class Event_>
typename Simulator<PolyKernel_, EventComparison_>::Key
Simulator<PolyKernel_, EventComparison_>::
add(const RationalFunction& f, const Event_& e)
{
	Event* ee = new Event_(e);
	//std::cout << "Solving: " << f << std::endl;
	PolynomialKernel::solve(f, ee->root_stack());
	bool sign = PolynomialKernel::sign_at_negative_infinity(f);
	while (!ee->root_stack().empty() && ee->root_stack().top() < current_time())
	{
		ee->root_stack().pop();
		sign = !sign;
	}
	if (sign) ee->root_stack().pop();			// TODO: double-check the logic
	//std::cout << "Pushing: " << ee->root_stack().top() << std::endl;
	return queue_.push(ee);
}
		
template<class PolyKernel_, template<class Event> class EventComparison_>
void
Simulator<PolyKernel_, EventComparison_>::
update(Key k, const RationalFunction& f)
{
	Event* ee = *k;
	ee->root_stack() = RootStack();								// no clear() in std::stack
	PolynomialKernel::solve(f, ee->root_stack());
	while (!ee->root_stack().empty() && ee->root_stack().top() < current_time())
		ee->root_stack().pop();
	update(k);
}

template<class PolyKernel_, template<class Event> class EventComparison_>
void
Simulator<PolyKernel_, EventComparison_>::
process()
{
	std::cout << "Queue size: " << queue_.size() << std::endl;
	Key top = queue_.top();
	Event* e = *top;
	
	if (e->root_stack().empty()) 		{ reached_infinity_ = true; return; }
	else 								{ current_ = e->root_stack().top(); e->root_stack().pop();  }
	
	if (e->process(this))				update(top);
	else								{ queue_.pop(); delete e; }
}

template<class PolyKernel_, template<class Event> class EventComparison_>
void
Simulator<PolyKernel_, EventComparison_>::
update(Key i)
{
	queue_.update(i);
}
		
template<class PolyKernel_, template<class Event> class EventComparison_>
typename Simulator<PolyKernel_, EventComparison_>::Time
Simulator<PolyKernel_, EventComparison_>::
audit_time() const
{
	const_Key top = queue_.top();
	Event* e = *top;

	if (e->root_stack().empty()) return current_ + 1;
	else return PolynomialKernel::between(e->root_stack().top(), current_);
}
		
template<class PolyKernel_, template<class Event> class EventComparison_>
std::ostream&
Simulator<PolyKernel_, EventComparison_>::
print(std::ostream& out) const
{
	out << "Simulator: " << std::endl;
	return queue_.print(out, "  ");
}
