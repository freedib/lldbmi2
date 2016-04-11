cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(BUILD_TESTS)
	file(GLOB_RECURSE tests_paths ${CMAKE_CURRENT_SOURCE_DIR}/tests/src/*.cpp)
	foreach( test_path ${tests_paths} )
		get_filename_component(test_source ${test_path} NAME )
	    string( REPLACE ".cpp" "" project_name ${test_source} )		# cut off .cpp
		project(${project_name})
	    add_executable( ${project_name} ${test_path} )
	endforeach( test_path ${tests_paths} )
endif(BUILD_TESTS)

project(lldbmi2)
execute_process(COMMAND git log --oneline
                COMMAND wc -l
                COMMAND tr -d " "
                COMMAND tr -d "\n"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE version_revision)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/templates/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/version.h)

if(USE_LIB_PATH)
find_library(LLDB_LIBRARY LLDB PATHS ${USE_LIB_PATH})
else(USE_LIB_PATH)
find_library(LLDB_LIBRARY LLDB PATHS "/Applications/Xcode.app/Contents/SharedFrameworks")
endif(USE_LIB_PATH)
mark_as_advanced(LLDB_LIBRARY)

set(CMAKE_INSTALL_RPATH "/Applications/Xcode.app/Contents/SharedFrameworks")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)

file(GLOB_RECURSE lldbmi2_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${lldbmi2_sources})
target_link_libraries(${PROJECT_NAME} ${LLDB_LIBRARY})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
