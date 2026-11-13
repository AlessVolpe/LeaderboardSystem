# Install into a disposable prefix; compile and test an independent project.
# No src/internal directory or build-tree include path is supplied.
if(NOT DEFINED ROOT OR NOT DEFINED BUILD_ROOT OR NOT DEFINED GENERATOR)
    message(FATAL_ERROR "ROOT, BUILD_ROOT, GENERATOR are required")
endif()
set(prefix "${BUILD_ROOT}/installed-consumer-prefix")
set(consumer_build "${BUILD_ROOT}/installed-consumer-build")

execute_process(
    COMMAND "${CMAKE_COMMAND}" --install "${BUILD_ROOT}" --prefix "${prefix}" --config "${CONFIG}"
    RESULT_VARIABLE status
    OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 0)
    message(FATAL_ERROR "Library installation failed:\n${out}\n${err}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -S "${ROOT}/examples/external_consumer" -B "${consumer_build}"
            -G "${GENERATOR}" "-DCMAKE_PREFIX_PATH=${prefix}"
            "-DCMAKE_C_COMPILER=${C_COMPILER}" "-DCMAKE_BUILD_TYPE=Debug"
            "-DLEADERBOARD_EXTERNAL_SANITIZERS=${SANITIZERS}"
    RESULT_VARIABLE status
    OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 0)
    message(FATAL_ERROR "External consumer configuration failed:\n${out}\n${err}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${consumer_build}" --config "${CONFIG}"
    RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 0)
    message(FATAL_ERROR "External consumer build failed:\n${out}\n${err}")
endif()

execute_process(
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${consumer_build}" --output-on-failure
            -C "${CONFIG}"
    RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 0)
    message(FATAL_ERROR "External consumer test failed:\n${out}\n${err}")
endif()
