project						(Dionysus)

option						(debug				"Build Dionysus with debugging on" 		OFF)
option						(counters			"Build Dionysus with counters on" 		OFF)
option						(optimize			"Build Dionysus with optimization"		ON)

# Find everything that's always required
find_package				(Boost REQUIRED)
find_package				(Doxygen)
find_library				(dsrpdb_LIBRARY 			NAMES dsrpdb)
find_path					(dsrpdb_INCLUDE_DIR 		dsrpdb/Protein.h)

set							(libraries 					${libraries}
														${dsrpdb_LIBRARY})

#CGAL
execute_process				(COMMAND ${CMAKE_MAKE_PROGRAM} -f ${CMAKE_CURRENT_SOURCE_DIR}/FindCGAL.Makefile libpaths
							 OUTPUT_VARIABLE cgal_libpaths)
execute_process				(COMMAND ${CMAKE_MAKE_PROGRAM} -f ${CMAKE_CURRENT_SOURCE_DIR}/FindCGAL.Makefile ldflags
							 OUTPUT_VARIABLE cgal_ldflags)
execute_process				(COMMAND ${CMAKE_MAKE_PROGRAM} -f ${CMAKE_CURRENT_SOURCE_DIR}/FindCGAL.Makefile cxxflags
							 OUTPUT_VARIABLE cgal_cxxflags)
execute_process				(COMMAND ${CMAKE_MAKE_PROGRAM} -f ${CMAKE_CURRENT_SOURCE_DIR}/FindCGAL.Makefile libpath
							 OUTPUT_VARIABLE cgal_libpath)
#string						(REPLACE "\n" "" cgal_libpaths	${cgal_libpaths})
#string						(REPLACE "\n" "" cgal_ldflags 	${cgal_ldflags})
string						(REPLACE "\n" "" cgal_cxxflags 	${cgal_cxxflags})
string						(REPLACE "\n" "" cgal_libpath 	${cgal_libpath})
add_definitions				(${cgal_cxxflags})
find_library				(cgal_LIBRARY				NAMES CGAL
														PATHS ${cgal_libpath})
find_library				(core_LIBRARY				NAMES core++
														PATHS ${cgal_libpath})
find_library				(mpfr_LIBRARY				NAMES mpfr)
find_library				(gmp_LIBRARY				NAMES gmp)
find_library				(gmpxx_LIBRARY				NAMES gmpxx)
find_library				(m_LIBRARY					NAMES m)

set							(cgal_libraries 			${cgal_LIBRARY} 
														${core_LIBRARY}
														${mpfr_LIBRARY} 
														${gmp_LIBRARY} 
														${gmpxx_LIBRARY} 
														${m_LIBRARY})

# Debugging
if							(debug)
	find_library			(cwd_LIBRARY				NAMES cwd)
	find_path				(cwd_INCLUDE_DIR			libcwd/debug.h)
	set 					(cwd_INCLUDE_DIR			${cwd_INCLUDE_DIR}/libcwd)
	add_definitions			(-DCWDEBUG -g)
	set						(external_sources			${CMAKE_CURRENT_SOURCE_DIR}/src/debug.cpp)
	set						(libraries 					${libraries} ${cwd_LIBRARY})
else						(debug)
	add_definitions			(-DNDEBUG)
endif						(debug)

# Counters
if							(counters)
	add_definitions			(-DCOUNTERS)
endif						(counters)

# Optimization
if							(optimize GREATER 0)
	add_definitions			(-O${optimize})
elseif						(optimize)
	add_definitions			(-O)
endif						(optimize)
endif						(optimize GREATER 0)


# Set includes
include_directories			(${CMAKE_CURRENT_BINARY_DIR}
							 ${CMAKE_CURRENT_SOURCE_DIR}/include
							 ${Boost_INCLUDE_DIR}
							 ${dsrpdb_INCLUDE_DIR}
							 ${cwd_INCLUDE_DIR})

# Doxygen (FIXME)
if							(DOXYGEN_FOUND)
#	add_custom_target 		(docs ALL 
#							${DOXYGEN_EXECUTABLE} Doxyfile
#							DEPENDS Doxyfile)
endif						(DOXYGEN_FOUND)

# Set external sources
add_subdirectory			(examples)
add_subdirectory			(tests)
