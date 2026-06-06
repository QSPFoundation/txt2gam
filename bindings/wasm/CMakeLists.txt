add_executable(txt2gam_wasm t2g_wasm.c)
target_include_directories(txt2gam_wasm
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)
target_compile_options(txt2gam_wasm PRIVATE ${PROJECT_COMPILER_FLAGS})
target_link_libraries(txt2gam_wasm PRIVATE txt2gam)

set_target_properties(txt2gam_wasm PROPERTIES
    OUTPUT_NAME txt2gam
    SUFFIX ".js"
)

target_link_options(txt2gam_wasm PRIVATE
    "SHELL:--post-js ${CMAKE_CURRENT_SOURCE_DIR}/t2g_wasm.js"
    "-sEXPORTED_FUNCTIONS=['_t2gInit','_t2gTerminate','_t2gWasmParseTextData','_t2gWasmEncodeTextToGame','_t2gWasmDecodeGameToText','_t2gWasmExtractStrings','_malloc','_free']"
    "-sEXPORTED_RUNTIME_METHODS=['HEAPU8','HEAP32','lengthBytesUTF16','stringToUTF16']"
    "-sWASM=1"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sMODULARIZE=1"
    "-sEXPORT_NAME=createT2gModule"
    "-sNO_EXIT_RUNTIME=1"
)

# Installation

install(FILES
        "$<TARGET_FILE_DIR:txt2gam_wasm>/txt2gam.js"
        "$<TARGET_FILE_DIR:txt2gam_wasm>/txt2gam.wasm"
    DESTINATION
        "${CMAKE_INSTALL_BINDIR}"
)

# Packaging

set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-wasm")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TXT2GAM WASM module")
set(CPACK_PACKAGE_VENDOR "QSP Foundation")
set(CPACK_PACKAGE_CONTACT "byte@qsp.org")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_VERBATIM_VARIABLES ON)

include(CPack)
