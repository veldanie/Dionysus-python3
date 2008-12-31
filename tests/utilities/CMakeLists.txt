set							(targets						
							 test-set-iterators
							 test-consistencylist
							 test-orderlist
							 test-counters)

foreach 					(t ${targets})
	add_executable			(${t} ${t}.cpp)
	target_link_libraries	(${t} ${libraries})
endforeach 					(t ${targets})
