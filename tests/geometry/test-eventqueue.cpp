#include <utilities/eventqueue.h>
#include <functional>
#include <iostream>

int main()
{
	typedef EventQueue<int, std::less<int> >		    EQ;
	typedef EQ::iterator								iterator;
	 
	EQ queue;

	iterator i = queue.push(4);
	queue.push(2);
	queue.push(7);
	iterator j = queue.push(6);
	queue.push(5);

	queue.replace(i,8);
	queue.remove(j);

	while (!queue.empty())
	{
		std::cout << *queue.top() << std::endl;
		queue.pop();
	}
}
