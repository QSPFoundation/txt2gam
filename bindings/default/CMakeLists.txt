add_library(txt2gam_default t2g_default.c)
target_include_directories(txt2gam_default
    PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_options(txt2gam_default PRIVATE ${PROJECT_COMPILER_FLAGS})
target_link_libraries(txt2gam_default PUBLIC txt2gam)

# Installation

install(TARGETS txt2gam_default
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES t2g_default.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/txt2gam")
