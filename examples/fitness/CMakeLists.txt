set							(targets						
							 avida-distance
                             avida-landscape)
							 
foreach 					(t ${targets})
	add_executable			(${t} ${t}.cpp)
	target_link_libraries	(${t} ${libraries})
endforeach 					(t ${targets})
