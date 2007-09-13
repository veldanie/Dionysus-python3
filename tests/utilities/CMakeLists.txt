set							(targets						
							 test-consistencylist
							 test-orderlist
							 test-counters)

foreach 					(t ${targets})
	add_executable			(${t} ${t}.cpp ${external_sources})
	target_link_libraries	(${t} ${libraries})
endforeach 					(t ${targets})
