# Cross-platform CTest wrapper: feed standard input without depending on a shell.
execute_process(
    COMMAND "${CLI}"
    INPUT_FILE "${INPUT}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)

if (EXPECT_INCOMPLETE)
    if ("${result}" STREQUAL "0")
        message(FATAL_ERROR "Incomplete input unexpectedly succeeded:\n${output}")
    endif ()
    if (NOT error_output MATCHES "Unable to read or allocate player names")
        message(FATAL_ERROR "Unexpected error for incomplete input:\n${error_output}")
    endif ()
elseif (EXPECT_ERROR)
    if ("${result}" STREQUAL "0")
        message(FATAL_ERROR "Invalid input unexpectedly succeeded:\n${output}")
    endif ()
    if (NOT error_output MATCHES "Expected a player count")
        message(FATAL_ERROR "Unexpected error for invalid input:\n${error_output}")
    endif ()
else ()
    if (NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "CLI exited with ${result}:\n${error_output}\n${output}")
    endif ()

    string(REGEX MATCHALL "Inserted player [^\n]+ with [0-9]+ points" inserted "${output}")
    list(LENGTH inserted inserted_count)
    if (NOT inserted_count EQUAL EXPECT_INSERTED)
        message(FATAL_ERROR "Expected ${EXPECT_INSERTED} inserts, saw ${inserted_count}:\n${output}")
    endif ()

    string(REGEX MATCHALL "Duplicate player name [^\n]+ ignored" duplicates "${output}")
    list(LENGTH duplicates duplicate_count)
    if (NOT duplicate_count EQUAL EXPECT_DUPLICATES)
        message(FATAL_ERROR "Expected ${EXPECT_DUPLICATES} duplicates, saw ${duplicate_count}:\n${output}")
    endif ()
endif ()
