project						(Dionysus)

option						(logging 			"Build Dionysus with logging on" 		OFF)
option						(counters			"Build Dionysus with counters on" 		OFF)
option						(debug				"Build Dionysus with debugging on" 		OFF)
option						(optimize			"Build Dionysus with optimization"		ON)

# Find everything that's always required
find_package				(Boost REQUIRED)
find_package				(Doxygen)
find_library				(dsrpdb_LIBRARY 			NAMES dsrpdb)
find_path					(dsrpdb_INCLUDE_DIR 		dsrpdb/Protein.h)

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
find_library				(cgal_LIBRARY				NAMES CGAL
														PATHS ${cgal_libpath})
find_library				(core_LIBRARY				NAMES CGALcore++
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
set                         (cgal_cxxflags              ${cgal_cxxflags}
                                                        -DCGAL_NO_ASSERTIONS)

# SYNAPS
add_definitions				(-DBOOST_UBLAS_TYPE_CHECK=0)
find_library				(synaps_LIBRARY				NAMES synaps)
set							(synaps_libraries			${synaps_LIBRARY}
														${gmp_LIBRARY}
														${gmpxx_LIBRARY})

# Debugging
if							(debug)
	if 						(optimize)
			set				(cxx_flags					${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
	else					(optimize)
			set				(cxx_flags					${CMAKE_CXX_FLAGS_DEBUG})
	endif					(optimize)
else						(debug)
	if 						(optimize)
			set				(cxx_flags					${CMAKE_CXX_FLAGS_RELEASE})
	else					(optimize)
			set				(cxx_flags					${CMAKE_CXX_FLAGS})
	endif					(optimize)
endif						(debug)
add_definitions				(${cxx_flags})

# Logging
if 							(logging)
	find_library			(rlog_LIBRARY				NAMES rlog)
	find_path				(rlog_INCLUDE_DIR			rlog/rlog.h)
	set						(rlog_INCLUDE_DIR			${rlog_INCLUDE_DIR})
	add_definitions			(-DLOGGING -DRLOG_COMPONENT=dionysus)
	set						(libraries					${libraries} ${rlog_LIBRARY})
endif						(logging)

# Counters
if							(counters)
	add_definitions			(-DCOUNTERS)
endif						(counters)


# Set includes
include_directories			(${CMAKE_CURRENT_BINARY_DIR}
							 ${CMAKE_CURRENT_SOURCE_DIR}/include
							 ${Boost_INCLUDE_DIR}
							 ${dsrpdb_INCLUDE_DIR}
							 ${cwd_INCLUDE_DIR}
							 ${rlog_INCLUDE_DIR})

# Doxygen (FIXME)
if							(DOXYGEN_FOUND)
#	add_custom_target 		(docs ALL 
#							${DOXYGEN_EXECUTABLE} Doxyfile
#							DEPENDS Doxyfile)
endif						(DOXYGEN_FOUND)

# Process subdirectories
add_subdirectory			(examples)
add_subdirectory			(tests)
