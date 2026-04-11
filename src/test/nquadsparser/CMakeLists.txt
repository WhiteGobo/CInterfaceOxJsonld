FIND_PACKAGE(BISON 3.4 REQUIRED)
FIND_PACKAGE(FLEX 2.5.19 REQUIRED)


flex_target(nquads_scanner "nquads_parser.l"
	"${CMAKE_CURRENT_BINARY_DIR}/nquads_scanner.c"
)

if (NOT FLEX_nquads_scanner_DEFINED)
	message(FATAL_ERROR "flex failed.")
endif()

bison_target(nquads_parser "nquads_parser.y"
	"${CMAKE_CURRENT_BINARY_DIR}/nquads_parser.qq.c"
	DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/nquads_parser.qq.h"
)

if (NOT BISON_nquads_parser_DEFINED)
	message(FATAL_ERROR "bison failed.")
endif()
add_flex_bison_dependency(nquads_scanner nquads_parser)

message(INFO "generated ntriples parser source: ${FLEX_nquads_scanner_OUTPUTS} ${BISON_nquads_parser_OUTPUTS}")

add_library(nquadsparser
	${FLEX_nquads_scanner_OUTPUTS}
	${BISON_nquads_parser_OUTPUTS}
	nquads_parser.h
	term.h
)

target_include_directories(nquadsparser PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
)
