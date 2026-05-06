set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

FetchContent_Declare(
        hashtable
        GIT_REPOSITORY https://github.com/xtreme8000/hashtable
        CMAKE_CACHE_ARGS -DBUILD_SHARED_LIBS=OFF  
        OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(hashtable)
