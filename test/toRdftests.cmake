set(tmpbase "${CMAKE_CURRENT_SOURCE_DIR}/jsonld-api-testsuite")
set(toRdf_manifest "${tmpbase}/toRdf-manifest.jsonld")
extract_info_from_jsonld_manifest(
	${toRdf_manifest}
	baseIri testlist testlist_length)
math(EXPR range "${testlist_length} - 1")
foreach(x RANGE 0 ${range})
	string(JSON data GET ${testlist} ${x})
	configure_toRdfTest(${tmpbase} ${baseIri} ${data} "testsuite-" )
endforeach()

set(tmpbase "${CMAKE_CURRENT_SOURCE_DIR}/own-testsuite")
set(toRdf_manifest "${tmpbase}/toRdf-manifest.jsonld")
extract_info_from_jsonld_manifest(
	${toRdf_manifest}
	baseIri testlist testlist_length)
math(EXPR range "${testlist_length} - 1")
foreach(x RANGE 0 ${range})
	string(JSON data GET ${testlist} ${x})
	configure_toRdfTest(${tmpbase} ${baseIri} ${data} "owntestsuite-" )
endforeach()


message(WARNING "Some Tests are currently expected failures.")
foreach( x IN ITEMS
		"testsuite-#te076" "testsuite-#tc029" "testsuite-#te115"
		"testsuite-#te116"
		"testsuite-#tep02" "testsuite-#ter42"
		"testsuite-#ttn01"
		"testsuite-#tnt03" "testsuite-#tnt01" "testsuite-#tm005"
		"testsuite-#tli13" "testsuite-#tli12" "testsuite-#tjs21"
		"testsuite-#tjs20" "testsuite-#tjs19" "testsuite-#tjs17"
		"testsuite-#tjs16" "testsuite-#tjs15" "testsuite-#tjs14"
		"testsuite-#tjs13" "testsuite-#tjs12" "testsuite-#tjs11"
		"testsuite-#tjs10" "testsuite-#tjs09" "testsuite-#tjs08"
		"testsuite-#tjs07" "testsuite-#tjs06"
		"testsuite-#te091"
		"testsuite-#te090" "testsuite-#te089" "testsuite-#te077"
		"testsuite-#te071" "testsuite-#te062" "testsuite-#te026"
		"testsuite-#tdi11" "testsuite-#tdi10" "testsuite-#tc013"
		"testsuite-#t0123" "testsuite-#t0122"
		"testsuite-#te060" "testsuite-#tdi12"
		"testsuite-#ter02" "testsuite-#ter03" "testsuite-#ter05"
		"testsuite-#t0114" "testsuite-#t0115" "testsuite-#t0117"
		"testsuite-#t0118"
		"testsuite-#tc025" "testsuite-#tdi09"
		"testsuite-#te020" "testsuite-#te021"
		"testsuite-#te027" "testsuite-#te038"
		"testsuite-#te079" "testsuite-#te080" "testsuite-#te081"
		"testsuite-#te082" "testsuite-#te083" "testsuite-#te084"
		"testsuite-#te093" "testsuite-#te094" "testsuite-#te095"
		"testsuite-#te096" "testsuite-#te097" "testsuite-#te098"
		"testsuite-#te102" "testsuite-#te103" "testsuite-#te075"
		"testsuite-#te104" "testsuite-#te105" "testsuite-#te107"
		"testsuite-#te108" "testsuite-#tin06"
		"testsuite-#tm013" "testsuite-#tm014" "testsuite-#tm015"
		"testsuite-#tm016" "testsuite-#tpi11"
		"testsuite-#tpr25" "testsuite-#tpr43"
		"testsuite-#ttn02"
		"owntestsuite-#ter02" "owntestsuite-#ter03"
		"owntestsuite-#ter05"
)
	set_property(TEST "${x}" PROPERTY WILL_FAIL TRUE)
	get_property(tmp_labels TEST "${x}" PROPERTY LABELS)
	list(APPEND tmp_labels "ExpectError")
	set_property(TEST "${x}" PROPERTY LABELS "${tmp_labels}")
endforeach()
foreach( x IN ITEMS
		"testsuite-#ter49"
		"testsuite-#tpi01" "testsuite-#tso01" "testsuite-#tso03"
		"testsuite-#tso07" "testsuite-#tso10" "testsuite-#tso12"
		"testsuite-#tso13"
)
	set_property(TEST "${x}" PROPERTY
		PASS_REGULAR_EXPRESSION ""
	)
	get_property(tmp_labels TEST "${x}" PROPERTY LABELS)

	list(APPEND tmp_labels "SkipErrorDescription")
	set_property(TEST "${x}" PROPERTY LABELS "${tmp_labels}")
endforeach()
