find_package                (PythonLibs)
link_libraries              (${PYTHON_LIBRARIES})
include_directories         (${PYTHON_INCLUDE_PATH})
link_libraries              (${Boost_PYTHON_LIBRARY})

# currently can't build bindings with counters support, eventually FIXME
remove_definitions          (-DCOUNTERS)
add_library                 (_dionysus SHARED 
                                                dionysus.cpp 
                                                filtration.cpp
                                                chain.cpp
                                                static-persistence.cpp
                                                simplex.cpp
                                                zigzag-persistence.cpp

                                                test-optional.cpp
                            )

target_link_libraries       (_dionysus ${libraries})
