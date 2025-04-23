# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("ISP_Simulation_Pipeline")
set(OpenCV_DIR "C:/Libraries_cpp/opencv/build")
find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 17)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

include_directories(include)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(ISP_Simulation_Pipeline
    src/main.cpp
    modules/demosaic.cpp
    modules/imageio.cpp
)

# Set working directory to the project root
set_target_properties(ISP_Simulation_Pipeline PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_link_libraries(ISP_Simulation_Pipeline PRIVATE ${OpenCV_LIBS})

add_executable(demosaic_tests 
    tests/test_demosaic.cpp
    modules/demosaic.cpp
    modules/imageio.cpp
)

target_include_directories(demosaic_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/external/catch2
    ${CMAKE_SOURCE_DIR}/modules
)

target_link_libraries(demosaic_tests PRIVATE ${OpenCV_LIBS})
