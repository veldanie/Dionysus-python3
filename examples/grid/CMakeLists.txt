set							(targets						
							 test-grid2D 
							 test-grid2D-vineyard
							 combustion-vineyard)

if                          (use_dsrpdb)
    set                     (targets                    ${targets}
                            							pdbdistance-vineyard)
endif                       (use_dsrpdb)

if                              (CGAL_FOUND)
    # add_definitions             (${CGAL_CXX_FLAGS_INIT})
    add_definitions             (-frounding-math)
    include_directories         (${CGAL_INCLUDE_DIRS})

    foreach 					(t ${targets})
        add_executable			(${t} ${t}.cpp)
        target_link_libraries	(${t} ${libraries} ${CGAL_LIBRARY} ${dsrpdb_LIBRARY})
    endforeach 					(t ${targets})
endif                           (CGAL_FOUND)
