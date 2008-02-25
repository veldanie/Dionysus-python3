set							(targets						
							 ar-vineyard)
	
find_library                (boost_signals_LIBRARY      NAMES boost_signals
                                                        PATHS ${Boost_LIBRARY_DIR})

add_definitions				(${cgal_cxxflags})
foreach 					(t ${targets})
	add_executable			(${t} ${t}.cpp ${external_sources})
	target_link_libraries	(${t}   ${libraries} ${cgal_libraries} 
                                    ${synaps_libraries} ${boost_signals_LIBRARY})
endforeach 					(t)
