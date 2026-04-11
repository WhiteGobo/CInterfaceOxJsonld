function(extract_info_from_jsonld_manifest manifest_path var_baseIri var_testarray_json var_testarray_length)

	set(toRdf_manifest ${manifest_path})
	file(READ "${toRdf_manifest}" TORDFMANIFEST_STRING)
	#cmakes json cant handle ';' so we just remove them.
	string(REPLACE ";" "" TORDFMANIFEST_STRING "${TORDFMANIFEST_STRING}")
	string(JSON ${var_baseIri} GET ${TORDFMANIFEST_STRING} "baseIri")
	string(JSON ${var_testarray_json}
		GET ${TORDFMANIFEST_STRING} sequence
	)
	string(JSON ${var_testarray_length}
		LENGTH ${testlist}
	)

	return(PROPAGATE
		${var_baseIri}
		${var_testarray_json}
		${var_testarray_length})
endfunction()

function(configure_toRdfTest basePath baseIri testdata_json testsuffix)
	set(extras "")
	string(JSON id GET ${testdata_json} "@id")
	string(JSON type GET ${testdata_json} "@type")
	string(JSON name GET ${testdata_json} "name")
	string(JSON purpose GET ${testdata_json} "purpose")
	string(JSON input GET ${testdata_json} "input")
	set(inputfile "${basePath}/${input}")
	string(CONCAT testuri ${baseIri} ${input})
	string(JSON extraOption ERROR_VARIABLE finderror
		GET ${testdata_json} "option")
	if(extraOption)
		string(JSON specVersion ERROR_VARIABLE finderror
			GET ${extraOption} "specVersion")
		string(JSON processingMode ERROR_VARIABLE finderror
			GET ${extraOption} "processingMode")
		string(JSON internetAccess ERROR_VARIABLE finderror
			GET ${extraOption} "internetAccess")
		if(specVersion)
			string(REGEX REPLACE "json-ld-" "" tmp ${specVersion})
			list(APPEND extras "--version")
			list(APPEND extras "${tmp}")
		endif()
		if(processingMode)
			string(REGEX REPLACE "json-ld-" "" tmp ${processingMode})
			list(APPEND extras "--processing-mode")
			list(APPEND extras "${tmp}")
		endif()
		if(internetAccess)
			list(APPEND extras "--enable-load-over-http")
		endif()
	endif(extraOption)
	string(FIND ${type} PositiveEvaluationTest isPET)
	string(FIND ${type} PositiveSyntaxTest isPST)
	string(FIND ${type} NegativeEvaluationTest isNET)
	string(FIND ${type} NegativeEvalTest isMyNET)
	set(testname "${testsuffix}${id}")
	if(isPET GREATER_EQUAL 0)
		string(JSON expect GET ${testdata_json} "expect")
		set(expect "${basePath}/${expect}")
		add_test(
			NAME "${testname}" COMMAND testdriver_toRdfTest
			"--PositiveEvaluationTest"
			"--name" ${name}
			"--purpose" ${purpose}
			"--expect" ${expect}
			"--input" ${inputfile}
			"--base-uri" ${testuri}
			${extras}
		)
		set_property(TEST "${testname}" PROPERTY
			LABELS "PositiveEvaluationTest"
		)
	elseif(isPST GREATER_EQUAL 0)
		add_test(
			NAME "${testname}" COMMAND testdriver_toRdfTest
			#"--PositiveSyntaxTest"
			"--name" ${name}
			"--purpose" ${purpose}
			"--input" ${inputfile}
			"--base-uri" ${testuri}
		)
		set_property(TEST "${testname}" PROPERTY
			LABELS "PositiveSyntaxTest"
		)
	elseif(isNET GREATER_EQUAL 0)
		string(JSON expectError GET ${testdata_json} "expectErrorCode")
		add_test(
			NAME "${testname}" COMMAND testdriver_toRdfTest
			"--NegativeSyntaxTest"
			"--purpose" ${purpose}
			"--input" ${inputfile}
			"--base-uri" ${testuri}
		)
		set_property(TEST "${testname}" PROPERTY
			LABELS "NegativeEvaluationTest"
		)
		set_property(TEST "${testname}" PROPERTY
			PASS_REGULAR_EXPRESSION "${expectError}"
		)
	elseif(isMyNET GREATER_EQUAL 0)
		string(JSON expect GET ${testdata_json} "expect")
		set(expect "${basePath}/${expect}")
		add_test(
			NAME "${testname}" COMMAND testdriver_toRdfTest
			"--NegativeEvaluationTest"
			"--name" ${name}
			"--purpose" ${purpose}
			"--expect" ${expect}
			"--input" ${inputfile}
			"--base-uri" ${testuri}
			${extras}
		)
		set_property(TEST "${testname}" PROPERTY
			LABELS "MyNegativeEvaluationTest"
		)
	else()
		message(FATAL_ERROR "Not handled test type: ${type}")
	endif()
endfunction()
