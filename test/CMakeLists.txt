
add_subdirectory(nquadsparser)

include(cwalk.cmake)

FetchContent_Declare(
	BasicRDFGraphComparator
	GIT_REPOSITORY https://github.com/WhiteGobo/BasicRDFGraphComparator.git
	GIT_TAG 8b244ca1826c024a641d98f0bb52e3c649e79972
	FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(BasicRDFGraphComparator)

include(CTest)
include(helper_functions.cmake)


add_executable(testdriver_toRdfTest
	testdriverA.c
	#helper_lib_graph.c
	#helper_lib_graph.h
)

target_link_libraries(testdriver_toRdfTest PUBLIC
	CInterfaceOxJsonld
)

target_link_libraries(testdriver_toRdfTest PRIVATE
	nquadsparser
	BasicRDFGraphComparator
	cwalk
)

target_include_directories(testdriver_toRdfTest PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

include(toRdftests.cmake)
