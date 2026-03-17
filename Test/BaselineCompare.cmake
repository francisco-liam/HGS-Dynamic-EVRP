if(NOT DEFINED ROOT)
    message(FATAL_ERROR "ROOT variable must be provided")
endif()

get_filename_component(ROOT "${ROOT}" ABSOLUTE)

if(NOT DEFINED BINARY_DIR)
    message(FATAL_ERROR "BINARY_DIR variable must be provided")
endif()

get_filename_component(BINARY_DIR "${BINARY_DIR}" ABSOLUTE)

set(HGS_PATH "${BINARY_DIR}/hgs")
if(NOT EXISTS "${HGS_PATH}")
    message(FATAL_ERROR "Executable not found at ${HGS_PATH}. Build target 'bin' first.")
endif()

set(BASELINE_DIR "${ROOT}/baseline/vidal")
set(MANIFEST_PATH "${BASELINE_DIR}/manifest.csv")

if(NOT EXISTS "${MANIFEST_PATH}")
    message(FATAL_ERROR "Missing baseline manifest at ${MANIFEST_PATH}. Run BaselineCapture first.")
endif()

set(COMPARE_DIR "${BINARY_DIR}/baseline_compare")
file(REMOVE_RECURSE "${COMPARE_DIR}")
file(MAKE_DIRECTORY "${COMPARE_DIR}")

file(STRINGS "${MANIFEST_PATH}" MANIFEST_LINES)

set(HAS_MISMATCH FALSE)
foreach(MANIFEST_LINE ${MANIFEST_LINES})
    if(MANIFEST_LINE STREQUAL "")
        continue()
    endif()

    if(MANIFEST_LINE MATCHES "^instance,round,seed,cost")
        continue()
    endif()

    string(REPLACE "," ";" FIELDS "${MANIFEST_LINE}")
    list(LENGTH FIELDS FIELD_COUNT)
    if(NOT FIELD_COUNT EQUAL 4)
        message(FATAL_ERROR "Malformed manifest line: ${MANIFEST_LINE}")
    endif()

    list(GET FIELDS 0 INSTANCE)
    list(GET FIELDS 1 ROUND)
    list(GET FIELDS 2 SEED)
    list(GET FIELDS 3 EXPECTED_COST)

    set(CURRENT_SOL "${COMPARE_DIR}/${INSTANCE}.sol")
    set(BASELINE_SOL "${BASELINE_DIR}/${INSTANCE}.sol")

    execute_process(
            COMMAND "${HGS_PATH}" "${ROOT}/Instances/CVRP/${INSTANCE}.vrp" "${CURRENT_SOL}" -seed ${SEED} -round ${ROUND}
            WORKING_DIRECTORY "${BINARY_DIR}"
            RESULT_VARIABLE RUN_RESULT
    )
    if(NOT RUN_RESULT EQUAL 0)
        message(FATAL_ERROR "Solver execution failed for ${INSTANCE}")
    endif()

    file(STRINGS "${CURRENT_SOL}" SOLUTION_LINES)
    list(GET SOLUTION_LINES -1 LAST_LINE)
    string(REGEX MATCH "Cost[ ]+([-+0-9.eE]+)" COST_MATCH "${LAST_LINE}")
    if(NOT COST_MATCH)
        message(FATAL_ERROR "Could not parse cost for ${INSTANCE}")
    endif()
    string(REGEX REPLACE "Cost[ ]+" "" ACTUAL_COST "${COST_MATCH}")

    if(NOT "${ACTUAL_COST}" STREQUAL "${EXPECTED_COST}")
        message(SEND_ERROR "Baseline objective mismatch for ${INSTANCE}: expected ${EXPECTED_COST}, got ${ACTUAL_COST}")
        set(HAS_MISMATCH TRUE)
    endif()

    if(EXISTS "${BASELINE_SOL}")
        execute_process(
                COMMAND "${CMAKE_COMMAND}" -E compare_files "${CURRENT_SOL}" "${BASELINE_SOL}"
                RESULT_VARIABLE SOL_COMPARE_RESULT
        )
        if(NOT SOL_COMPARE_RESULT EQUAL 0)
            message(WARNING "Solution text differs for ${INSTANCE} (diagnostic only).")
        endif()
    else()
        message(WARNING "Missing baseline solution file for ${INSTANCE}: ${BASELINE_SOL}")
    endif()
endforeach()

if(HAS_MISMATCH)
    message(FATAL_ERROR "Baseline comparison failed")
endif()

message(STATUS "Baseline comparison passed")
