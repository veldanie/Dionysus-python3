template<class SortDS_, class TrajectoryExtractor_, class Simulator_>
KineticSort<SortDS_, TrajectoryExtractor_, Simulator_>::
KineticSort(SortDS* sort, Simulator* simulator, SwapCallback swap_callback):
	sort_(sort), swap_callback_(swap_callback)
{
	for (SortDSIterator cur = sort->begin(); cur != sort->end(); ++cur)
		list_.push_back(Node(cur, simulator->null_key()));
	schedule_swaps(list_.begin(), list_.end(), simulator);
}


template<class SortDS_, class TrajectoryExtractor_, class Simulator_>
template<class InputIterator>
void
KineticSort<SortDS_, TrajectoryExtractor_, Simulator_>::
insert(iterator pos, InputIterator f, InputIterator l, Simulator* simulator)
{
	iterator previous = pos; --previous;
	if (previous != list_.end()) simulator->remove(previous->swap_event_key);

	sort_->insert(pos->element, f, l);

	SortDSIterator cur = boost::next(previous)->element;
	while(cur != pos->element)
		list_.insert(pos->element, Node(cur++));
	if (previous != list_.end()) 
		schedule_swaps(previous, pos, simulator);
	else
		schedule_swaps(list_.begin(), pos, simulator);
}

template<class SortDS_, class TrajectoryExtractor_, class Simulator_>
void
KineticSort<SortDS_, TrajectoryExtractor_, Simulator_>::
erase(iterator pos, Simulator* simulator)
{
	simulator->remove(pos->swap_event_key);
	sort_->erase(pos->element);
	iterator prev = pos; --prev;
	list_.erase(pos);
	schedule_swaps(prev, simulator);
}

template<class SortDS_, class TrajectoryExtractor_, class Simulator_>
void
KineticSort<SortDS_, TrajectoryExtractor_, Simulator_>::
update_trajectory(iterator pos, Simulator* simulator)
{
	iterator prev = boost::prior(pos);
	if (prev != list_.end())
	{
		simulator->remove(prev->swap_event_key);
		schedule_swaps(prev, simulator);
	}

	if (boost::next(pos) != list_.end())
	{
		simulator->remove(pos->swap_event_key);
		schedule_swaps(pos, simulator);
	}
}


template<class SortDS_, class TrajectoryExtractor_, class Simulator_>
void						
KineticSort<SortDS_, TrajectoryExtractor_, Simulator_>::
swap(iterator pos, Simulator* simulator)
{
	swap_callback_(sort_, pos->element);
	
	// TODO: add assertion that boost::next(pos) != list_.end()
	
	// Remove events
	iterator prev = boost::prior(pos);
	if (prev != list_.end())
		simulator->remove(prev->swap_event_key);
	iterator next = boost::next(pos);
	simulator->remove(next->swap_event_key);

	// Swap
	list_.splice(pos, list_, next);
	
	// update events
	next->swap_event_key = pos->swap_event_key;
	static_cast<SwapEvent*>(*(next->swap_event_key))->set_position(next);
	schedule_swaps(prev, simulator);
	schedule_swaps(pos, simulator);
	//audit(simulator);
}

template<class SortDS_, class TrajectoryExtractor_, class Simulator_>
bool
KineticSort<SortDS_, TrajectoryExtractor_, Simulator_>::
audit(Simulator* simulator) const
{
	typedef 		typename Simulator::RationalFunction		RationalFunction;
	typedef 		typename Simulator::Time					Time;
	
	Time t = simulator->audit_time();
	std::cout << "Auditing at " << t << std::endl;

	TrajectoryExtractor	te;
	
	typename NodeList::const_iterator next = list_.begin();
	typename NodeList::const_iterator cur = next++;
	RationalFunction cur_trajectory = te(cur->element);
	while (next != list_.end())
	{
		(*(cur->swap_event_key))->print(std::cout << "  ") << std::endl;

		RationalFunction next_trajectory = te(next->element);
		std::cout << "  Auditing:   " << cur_trajectory << ", " << next_trajectory << std::endl;
		std::cout << "  Difference: " << next_trajectory - cur_trajectory << std::endl;
		std::cout << "  Sign at:    " << t << ", " << PolynomialKernel::sign_at(next_trajectory - cur_trajectory, t) << std::endl;
		if (PolynomialKernel::sign_at(next_trajectory - cur_trajectory, t) == -1)
		{
			std::cout << "Audit failed at " << *cur->element << ", " << *next->element << std::endl;
			return false;
		}

		cur_trajectory = next_trajectory;
		cur = next++;
	}
	if (cur != list_.end()) (*(cur->swap_event_key))->print(std::cout << "  ") << std::endl;
	return true;
}

		
template<class SortDS_, class TrajectoryExtractor_, class Simulator_>
void						
KineticSort<SortDS_, TrajectoryExtractor_, Simulator_>::
schedule_swaps(iterator b, iterator e, Simulator* simulator)
{
	typedef 		typename Simulator::RationalFunction		RationalFunction;
	
	TrajectoryExtractor	te;
	
	iterator next = b; 
	iterator cur = next++;
	RationalFunction cur_trajectory = te(cur->element);
	while (next != e)
	{
		RationalFunction next_trajectory = te(next->element);
		std::cout << "Next trajectory: " << next_trajectory << std::endl;
		// TODO: add assertion that (next_trajectory - cur_trajectory)(s->curren_time()) > 0
		cur->swap_event_key = simulator->add(next_trajectory - cur_trajectory, SwapEvent(this, cur));
		cur = next++;
		cur_trajectory = next_trajectory;
	}
	if (cur != e) schedule_swaps(cur, simulator);
}

template<class SortDS_, class TrajectoryExtractor_, class Simulator_>
void						
KineticSort<SortDS_, TrajectoryExtractor_, Simulator_>::
schedule_swaps(iterator i, Simulator* simulator)
{
	typedef 		typename Simulator::RationalFunction		RationalFunction;
	
	if (i == list_.end()) return;
	if (boost::next(i) == list_.end())
	{
		i->swap_event_key = simulator->add(SwapEvent(this, i));
		return;
	}

	TrajectoryExtractor	te;
	
	iterator next = boost::next(i); 
	RationalFunction i_trajectory = te(i->element);
	RationalFunction next_trajectory = te(next->element);
	
	//std::cout << "Updating swaps for: " << i_trajectory << ", " << next_trajectory << std::endl;
	//std::cout << "Difference:         " << next_trajectory - i_trajectory << std::endl;

	i->swap_event_key = simulator->add(next_trajectory - i_trajectory, SwapEvent(this, i));
	//i->swap_event_key = simulator->add(next_trajectory, SwapEvent(this, i));
}

/* SwapEvent */
template<class SortDS_, class TrajectoryExtractor_, class Simulator_>
class KineticSort<SortDS_, TrajectoryExtractor_, Simulator_>::SwapEvent: public Simulator::Event
{
	public:
		typedef						typename Simulator::Event					Parent;

									SwapEvent(const SwapEvent& e):
										sort_(e.sort_), pos_(e.pos_)			{}
									SwapEvent(KineticSort* sort, iterator pos):
										sort_(sort), pos_(pos)					{}

		virtual bool				process(Simulator* s) const;
		void						set_position(iterator i)					{ pos_ = i; }
		iterator					position() const							{ return pos_; }
		std::ostream&				print(std::ostream& out) const;

	private:
		KineticSort*				sort_;
		iterator					pos_;
};

template<class SortDS_, class TrajectoryExtractor_, class Simulator_>
bool
KineticSort<SortDS_, TrajectoryExtractor_, Simulator_>::SwapEvent::
process(Simulator* s) const
{ 
	std::cout << "Swapping. Current time: " << s->current_time() << std::endl;
	sort_->swap(pos_, s); 
	return true; 
}

template<class SortDS_, class TrajectoryExtractor_, class Simulator_>
std::ostream&				
KineticSort<SortDS_, TrajectoryExtractor_, Simulator_>::SwapEvent::
print(std::ostream& out) const
{
	Parent::print(out) << ", SwapEvent at " << TrajectoryExtractor_()(position()->element);
	return out;
}

