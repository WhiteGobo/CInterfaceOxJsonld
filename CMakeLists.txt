
cmake_minimum_required(VERSION 3.25)

#add debugging information to binaries
#add_compile_options(-g)

project(CInterfaceOxJsonld VERSION 0.1.0 LANGUAGES C)

add_subdirectory(src)


## Testing

enable_testing()
add_subdirectory(test)
