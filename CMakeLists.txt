cmake_minimum_required(VERSION 3.5)

# Project
project(MTC)

# Sources
file(GLOB_RECURSE SOURCES_MTC src/*.c)
file(GLOB_RECURSE HEADERS_MTC include/*.h)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES_MTC} ${HEADERS_MTC})

# Includes
target_include_directories(${PROJECT_NAME} PUBLIC include)

# Flags
target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>: -O3 -Wall -Wextra -Werror -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wconversion -Wunreachable-code -Wno-error=maybe-uninitialized -Wno-error=unused-result -Wpedantic>
)

# Download/Update librairies if needed
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodules update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --remote --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --remote --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/lib/libwebsockets")	# nothing has been created
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# Add Libraries
set(LWS_WITH_SSL OFF)
set(LWS_WITHOUT_TESTAPPS ON)
set(LWS_WITHOUT_TEST_SERVER ON)
set(LWS_WITHOUT_TEST_SERVER_EXTPOLL ON)
set(LWS_WITHOUT_TEST_PING ON)
set(LWS_WITHOUT_TEST_CLIENT ON)
set(LWS_WITH_SHARED OFF)
add_subdirectory(lib/libwebsockets)

# Link Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE websockets)
