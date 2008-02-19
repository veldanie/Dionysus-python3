set							(targets						
							 test-grid2D 
							 pdbdistance-vineyard
							 combustion-vineyard)
							 
add_definitions				(${cgal_cxxflags})
foreach 					(t ${targets})
	add_executable			(${t} ${t}.cpp)
	target_link_libraries	(${t} ${libraries} ${cgal_libraries} ${dsrpdb_LIBRARY})
endforeach 					(t ${targets})
