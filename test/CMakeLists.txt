include(cwalk.cmake)

FetchContent_Declare(
	BasicRDFGraphComparator
	GIT_REPOSITORY https://github.com/WhiteGobo/BasicRDFGraphComparator.git
	GIT_TAG 8b244ca1826c024a641d98f0bb52e3c649e79972
	FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(BasicRDFGraphComparator)

FetchContent_Declare(
	NQuadsRDF
	GIT_REPOSITORY https://github.com/WhiteGobo/NQuadsRDF.git
	GIT_TAG f98a85359f8caab0836d6ed1c27a421f0f027ded
	FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(NQuadsRDF)

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
	NQuadsRDF
	BasicRDFGraphComparator
	cwalk
)

target_include_directories(testdriver_toRdfTest PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

include(toRdftests.cmake)
