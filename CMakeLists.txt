cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

if ("${APP_VERSION}" STREQUAL "")
	set(APP_VERSION "0.0.0" CACHE STRING "App version" FORCE)
endif()

project(txt2gam VERSION ${APP_VERSION} HOMEPAGE_URL "https://qsp.org")

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(GNUInstallDirs)

# char16_t / <uchar.h> require C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	set(PROJECT_COMPILER_FLAGS "-Wall")
	set(PROJECT_LINKER_FLAGS "")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-char-subscripts -Wno-unused-variable")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -DNDEBUG")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
	set(PROJECT_COMPILER_FLAGS "/W4")
	set(PROJECT_LINKER_FLAGS "")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
	set(PROJECT_COMPILER_FLAGS "")
	set(PROJECT_LINKER_FLAGS "")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -DNDEBUG")
else()
	message(FATAL_ERROR "Compiler is not supported")
endif()

configure_file(txt2gam_config.h.cmakein txt2gam_config.h @ONLY)

# Library

set(TXT2GAM_LIB_SOURCES
	src/coding.c
	src/locations.c
	src/locdata.c
	src/memwatch.c
	src/t2g_api.c
	src/text.c
	src/sys.c
)
add_library(txt2gam ${TXT2GAM_LIB_SOURCES})
if (WIN32)
	target_compile_definitions(txt2gam PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
if (SPEC_PASS)
	target_compile_definitions(txt2gam PRIVATE SPEC_PASS=${SPEC_PASS})
endif()
target_compile_options(txt2gam PRIVATE ${PROJECT_COMPILER_FLAGS})
target_link_options(txt2gam PRIVATE ${PROJECT_LINKER_FLAGS})

target_include_directories(txt2gam
	INTERFACE
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/txt2gam>"
)

generate_export_header(txt2gam
	BASE_NAME Txt2gam
	EXPORT_MACRO_NAME TXT2GAM_EXTERN
)

export(TARGETS txt2gam NAMESPACE Txt2gam:: FILE "${CMAKE_CURRENT_BINARY_DIR}/Txt2gamTargets.cmake")

# Installation

install(TARGETS txt2gam EXPORT Txt2gamTargets)

install(FILES
		src/coding.h
		src/declarations.h
		src/locations.h
		src/locdata.h
		src/sys.h
		src/t2g_api.h
		src/t2g_types.h
		src/text.h
		"${CMAKE_CURRENT_BINARY_DIR}/txt2gam_config.h"
		"${CMAKE_CURRENT_BINARY_DIR}/txt2gam_export.h"
	DESTINATION
		"${CMAKE_INSTALL_INCLUDEDIR}/txt2gam"
)

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/cmake/Txt2gam/Txt2gamConfigVersion.cmake" COMPATIBILITY AnyNewerVersion)
configure_package_config_file(Txt2gamConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/cmake/Txt2gam/Txt2gamConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Txt2gam"
)

install(EXPORT Txt2gamTargets FILE Txt2gamTargets.cmake NAMESPACE Txt2gam:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Txt2gam")
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

if (PROJECT_IS_TOP_LEVEL)
	if (EMSCRIPTEN)
		add_subdirectory(bindings/wasm)
	else()
		add_subdirectory(bindings/default)
		add_subdirectory(app)
	endif()
endif()
