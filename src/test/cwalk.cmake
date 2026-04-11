set(CMAKE_POLICY_VERSION_MINIMUM 3.21)

FetchContent_Declare(
        cwalk
        GIT_REPOSITORY https://github.com/likle/cwalk.git
	GIT_TAG v1.2.9
	EXCLUDE_FROM_ALL #dont install, its just for testing
	FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(cwalk)
