Counter*
Counter::
get_child(const std::string& path, std::string::size_type pos)
{
	if (pos >= path.size())
		return this;

	std::string::size_type slash_pos = path.find('/', pos);
	if (slash_pos == std::string::npos)
		slash_pos = path.size();

	std::string child_name = path.substr(pos, slash_pos - pos);
	SubCounterMap::iterator child = subcounters_.find(child_name);

	if (child != subcounters_.end())
		return child->second->get_child(path, slash_pos + 1);
	else	
		return (subcounters_[child_name] = new Counter(path.substr(0, slash_pos)))->get_child(path, slash_pos + 1);
}

Counter::
~Counter()
{ 
	if (full_name_ == "")
		print(); 

	for (SubCounterMap::iterator cur = subcounters_.begin(); cur != subcounters_.end(); ++cur)
		delete cur->second;
}

inline
void
Counter::
print()
{
	// FIXME: add (colored) timestamp
	std::cout << "Counter [" << full_name_ << "]: " << count << std::endl;
	for (SubCountMap::const_iterator cur = subcount.begin(); cur != subcount.end(); ++cur)
		std::cout << "  " << cur->first << ": " << cur->second << std::endl;
	for (SubCounterMap::iterator cur = subcounters_.begin(); cur != subcounters_.end(); ++cur)
		cur->second->print();
}	

