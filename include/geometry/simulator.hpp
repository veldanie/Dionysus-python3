#include "utilities/log.h"
#include "utilities/counter.h"

#ifdef LOGGING
static rlog::RLogChannel* rlSimulator =             DEF_CHANNEL("geometry/simulator", rlog::Log_Debug);

#endif // LOGGING

#ifdef COUNTERS
static Counter*  cSimulatorProcess =                GetCounter("simulator/process");
#endif // COUNTERS


template<class FuncKernel_, template<class Event> class EventComparison_>
template<class Event_>
typename Simulator<FuncKernel_, EventComparison_>::Key
Simulator<FuncKernel_, EventComparison_>::
add(const Event_& e)
{
	Event* ee = new Event_(e);
	return queue_.push(ee);
}

template<class FuncKernel_, template<class Event> class EventComparison_>
template<class Event_>
typename Simulator<FuncKernel_, EventComparison_>::Key
Simulator<FuncKernel_, EventComparison_>::
add(const Function& f, const Event_& e)
{
	Event* ee = new Event_(e);
	rLog(rlSimulator, "Solving: %s", tostring(f).c_str());
	int sign = FunctionKernel::sign_at_negative_infinity(f);
    rLog(rlSimulator, "Sign at -infinity: %i", sign);
    if (sign != 0)
    {
    	FunctionKernel::solve(f, ee->root_stack());
        rLog(rlSimulator, "Got solution with root stack size: %i", ee->root_stack().size());
    }

	while (!ee->root_stack().empty() && ee->root_stack().top() < current_time())
	{
		ee->root_stack().pop();
		sign *= -1;
	}
    if (sign == -1)
    {
        AssertMsg(ee->root_stack().top() == current_time(), 
                  "If sign is negative, we must be in the degenerate case");
        ee->root_stack().pop();
    }

	if (ee->root_stack().empty())
        rLog(rlSimulator, "Pushing event with empty root stack");
    else
    {
        rLog(rlSimulator, "Root stack size: %i", ee->root_stack().size());
        rLog(rlSimulator, "Pushing: %s", tostring(ee->root_stack().top()).c_str());
    }
	return queue_.push(ee);
}
		
template<class FuncKernel_, template<class Event> class EventComparison_>
void
Simulator<FuncKernel_, EventComparison_>::
update(Key k, const Function& f)
{
	Event* ee = *k;
	ee->root_stack() = RootStack();								// no clear() in std::stack
	FunctionKernel::solve(f, ee->root_stack());
	while (!ee->root_stack().empty() && ee->root_stack().top() < current_time())
		ee->root_stack().pop();
	update(k);
}

template<class FuncKernel_, template<class Event> class EventComparison_>
void
Simulator<FuncKernel_, EventComparison_>::
process()
{
    Count(cSimulatorProcess);
	rLog(rlSimulator, "Queue size: %i", queue_.size());
	Key top = queue_.top();
	Event* e = *top;
	
	if (e->root_stack().empty()) 		{ reached_infinity_ = true; return; }
	else 								{ current_ = e->root_stack().top(); e->root_stack().pop();  }
	
    // Get the top element out of the queue, put it back depending on what process() says
    EventQueue tmp; tmp.prepend(top, queue_);

	if (e->process(this))				{ queue_.prepend(top, tmp); update(top); }
	else								{ delete e; }
}

template<class FuncKernel_, template<class Event> class EventComparison_>
void
Simulator<FuncKernel_, EventComparison_>::
update(Key i)
{
	queue_.update(i);
}
		
template<class FuncKernel_, template<class Event> class EventComparison_>
typename Simulator<FuncKernel_, EventComparison_>::Time
Simulator<FuncKernel_, EventComparison_>::
audit_time() const
{
	const_Key top = queue_.top();
	Event* e = *top;

	if (e->root_stack().empty()) return current_ + 1;
	else return FunctionKernel::between(e->root_stack().top(), current_);
}
		
template<class FuncKernel_, template<class Event> class EventComparison_>
std::ostream&
Simulator<FuncKernel_, EventComparison_>::
operator<<(std::ostream& out) const
{
	out << "Simulator: " << std::endl;
	return queue_.print(out, "  ");
}

template<class FuncKernel_, template<class Event> class EventComparison_>
std::ostream&
operator<<(std::ostream& out, const Simulator<FuncKernel_, EventComparison_>& s)
{
    return s.operator<<(out);
}

template<class FuncKernel_, template<class Event> class EventComparison_>
std::ostream&
operator<<(std::ostream& out, const typename Simulator<FuncKernel_, EventComparison_>::Event& e)
{
    return e.operator<<(out);
}
