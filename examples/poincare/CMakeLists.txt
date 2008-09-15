set							(targets						
							 poincare)
							 
foreach 					(t ${targets})
	add_executable			(${t} ${t}.cpp)
	target_link_libraries	(${t} ${libraries} ${boost_program_options_LIBRARY})
endforeach 					(t ${targets})
