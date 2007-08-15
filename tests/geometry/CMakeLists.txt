set							(targets
							 euclidean
							 polynomial
							 test-eventqueue
							 test-kinetic-sort
							 test-linalg)

foreach 					(t ${targets})
	add_executable			(${t} ${t}.cpp)
	target_link_libraries	(${t} ${synaps_libraries})
endforeach 					(t ${targets})

