find_package                (PythonLibs)
link_libraries              (${PYTHON_LIBRARIES})
include_directories         (${PYTHON_INCLUDE_PATH})
link_libraries              (${Boost_PYTHON3_LIBRARY})

# currently can't build bindings with counters support, eventually FIXME
remove_definitions          (-DCOUNTERS)
set                         (sources
                                                dionysus.cpp 
                                                filtration.cpp
                                                chain.cpp
                                                static-persistence.cpp
                                                dynamic-persistence.cpp
                                                persistence-diagram.cpp
                                                simplex.cpp
                                                birthid.cpp
                                                zigzag-persistence.cpp
                                                cohomology-persistence.cpp
                                                rips.cpp
                                                distances.cpp
                            )
set                         (bindings_libraries ${libraries})

if                          (CGAL_FOUND)
    include                 (${CGAL_USE_FILE})

    set                     (sources            ${sources}
                                                alphashapes3d.cpp
                                                alphashapes2d.cpp)
    add_definitions         (${CGAL_CXX_FLAGS_INIT})
    include_directories     (${CGAL_INCLUDE_DIRS})

    link_libraries          (${CGAL_LIBRARY} ${CGAL_3RD_PARTY_LIBRARIES})
else                            (CGAL_FOUND)
    message(STATUS "CGAL not found, alphashape bindings will not be built")
    add_definitions         (-DNO_CGAL)
endif                       (CGAL_FOUND)

add_library                 (_dionysus SHARED   ${sources})
target_link_libraries       (_dionysus          ${link_libraries})


# Python files and the symlink
add_custom_target           (dionysus ALL
                             ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/dionysus ${CMAKE_CURRENT_BINARY_DIR}/dionysus
                             DEPENDS            dionysus/__init__.py
                                                dionysus/distances.py
                            )

get_target_property         (_dionysus_location _dionysus LOCATION)
add_custom_target           (dionysus-link ALL 
                             ${CMAKE_COMMAND} -E create_symlink ${_dionysus_location} ${CMAKE_CURRENT_BINARY_DIR}/dionysus/_dionysus.so
                             DEPENDS _dionysus)
