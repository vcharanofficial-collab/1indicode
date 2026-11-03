execute_process(
    COMMAND "${INDICODE}" --json-diagnostics check "${SOURCE}"
    RESULT_VARIABLE result
    ERROR_VARIABLE diagnostics
)

if(result EQUAL 0)
    message(FATAL_ERROR "type-error fixture unexpectedly passed")
endif()

string(JSON severity ERROR_VARIABLE json_error GET "${diagnostics}" 0 severity)
if(json_error OR NOT severity STREQUAL "error")
    message(FATAL_ERROR "expected a JSON error diagnostic, got: ${diagnostics}")
endif()

string(JSON code GET "${diagnostics}" 0 code)
string(JSON file GET "${diagnostics}" 0 file)
string(JSON line GET "${diagnostics}" 0 start line)
if(NOT code STREQUAL "E-SEMA-005" OR NOT file STREQUAL "${SOURCE}" OR NOT line EQUAL 1)
    message(FATAL_ERROR "JSON diagnostic fields are incorrect: ${diagnostics}")
endif()