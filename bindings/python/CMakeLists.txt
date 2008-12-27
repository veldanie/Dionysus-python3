link_libraries              (${Boost_PYTHON_LIBRARY})
include_directories         ("/usr/include/python2.6")
link_directories            ("/usr/lib/python2.6")
add_library                 (_dionysus SHARED 
                                                dionysus.cpp 
                                                filtration.cpp
                                                chain.cpp
                                                static-persistence.cpp
                                                simplex.cpp)
target_link_libraries       (_dionysus ${libraries})                                            
