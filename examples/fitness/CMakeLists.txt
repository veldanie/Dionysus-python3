set							(targets						
							 avida-distance
                             avida-landscape)
	
find_library                (boost_program_options_LIBRARY      NAME boost_program_options
                                                                PATHS ${Boost_LIBRARY_DIR})

foreach 					(t ${targets})
	add_executable			(${t} ${t}.cpp)
	target_link_libraries	(${t} ${libraries} ${boost_program_options_LIBRARY})
endforeach 					(t ${targets})
