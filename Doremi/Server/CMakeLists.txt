# CMake settings
cmake_minimum_required(VERSION 3.2.1)

# Root project settings
set(PROJECT_NAME Server)
project(${PROJECT_NAME})

# Set the files used in the target
file(GLOB_RECURSE HEADERS Headers/ *.h*)
file(GLOB_RECURSE SOURCES Source/ *.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
create_source_group("Header Files" "${CMAKE_CURRENT_SOURCE_DIR}/Include" ${HEADERS})
create_source_group("Source Files" "${CMAKE_CURRENT_SOURCE_DIR}/Source" ${SOURCES})
set(LIBRARIES GameCore Utilities)

# Set preprocessor definitions
SET(DEFINITIONS 
	${CUSTOM_TIMING_STATE}
)

# Add the target
add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE "${DEFINITIONS}")
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

# Set SUBSYSTEM
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")