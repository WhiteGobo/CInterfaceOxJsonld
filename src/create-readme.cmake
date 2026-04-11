set(readme_output "${CMAKE_CURRENT_BINARY_DIR}/README.md")
set(SKEL_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md.in")
add_custom_command(
	OUTPUT "${readme_output}"
	COMMAND cat "${SKEL_README}" > "${readme_output}"
	COMMAND cargo license #--output "${readme_output}"
	--color never -d #print output
	#-j #json output
	--current-dir "${CMAKE_CURRENT_SOURCE_DIR}/jsonld/"
	>> "${readme_output}"
	DEPENDS
	"${SKEL_README}"
)

add_custom_target(CInterfaceOxJsonldREADME ALL
	DEPENDS ${readme_output}
)

install(
	FILES ${readme_output}
	DESTINATION "./"
	COMPONENT Release
)
