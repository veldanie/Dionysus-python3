set							(targets						
							 triangle)
							 
foreach 					(t ${targets})
	add_executable			(${t} ${t}.cpp ${external_sources})
	target_link_libraries	(${t} ${libraries})
endforeach 					(t ${targets})
