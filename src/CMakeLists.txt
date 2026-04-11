cmake_minimum_required(VERSION 3.25)

#add debugging information to binaries
#add_compile_options(-g)

project(CInterfaceOxJsonld VERSION 0.1.0 LANGUAGES C)

find_package(OpenSSL 3.6 REQUIRED COMPONENTS SSL)

include(corrosion.cmake)

corrosion_import_crate(MANIFEST_PATH jsonld/Cargo.toml)
target_sources(CInterfaceOxJsonld INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
	FILES
	include/CInterfaceOxJsonld.h
)
target_link_libraries(CInterfaceOxJsonld INTERFACE
	OpenSSL::SSL
)
corrosion_install(TARGETS CInterfaceOxJsonld EXPORT CInterfaceOxJsonldtargets)


## Installation thingies

include(create-readme.cmake)

include(CMakePackageConfigHelpers)
set(ConfigPackageLocation lib/cmake/CInterfaceOxJsonld)
set(ConfigVersionCMake
	"${CMAKE_CURRENT_BINARY_DIR}/CInterfaceOxJsonld/CInterfaceOxJsonldConfigVersion.cmake"
)
write_basic_package_version_file(
	"${ConfigVersionCMake}"
	VERSION ${CInterfaceOxJsonld_VERSION}
	COMPATIBILITY AnyNewerVersion
)
install(EXPORT CInterfaceOxJsonldtargets
	FILE CInterfaceOxJsonldTargets.cmake
	NAMESPACE
		CInterfaceOxJsonld::
	DESTINATION
		${ConfigPackageLocation}
)
install(FILES
	#cmake/ClimbingStatsConfig.cmake
	"${ConfigVersionCMake}"
	DESTINATION ${ConfigPackageLocation}
	COMPONENT Release
)


## Testing

enable_testing()
add_subdirectory(test)
