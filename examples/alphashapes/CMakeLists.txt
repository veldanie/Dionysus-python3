set							(targets						
							 alphashapes3d
							 alpharadius)
							 
foreach 					(t ${targets})
	add_executable			(${t} ${t}.cpp ${external_sources})
	target_link_libraries	(${t} ${cgal_libraries})
endforeach 					(t ${targets})
