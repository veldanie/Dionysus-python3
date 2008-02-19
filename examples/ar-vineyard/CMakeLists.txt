set							(targets						
							 ar-vineyard)
							 
add_definitions				(${cgal_cxxflags})
foreach 					(t ${targets})
	add_executable			(${t} ${t}.cpp ${external_sources})
	target_link_libraries	(${t} ${libraries} ${cgal_libraries} ${synaps_libraries})
endforeach 					(t)
