set							(targets						
							 alphashapes3d
							 alphashapes2d
							 alpharadius)
							 
add_definitions				(${cgal_cxxflags})
foreach 					(t ${targets})
	add_executable			(${t} ${t}.cpp)
	target_link_libraries	(${t} ${libraries} ${cgal_libraries})
endforeach 					(t ${targets})
