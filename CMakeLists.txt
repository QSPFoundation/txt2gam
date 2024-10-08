cmake_minimum_required(VERSION 3.13)
project(txt2gam)

if (CMAKE_COMPILER_IS_GNUCC)
	set(PROJECT_COMPILER_FLAGS "-Wall")
	set(PROJECT_LINKER_FLAGS "-no-pie")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-char-subscripts -Wno-unused-variable")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -DNDEBUG")
endif()
if (MSVC)
	set(PROJECT_COMPILER_FLAGS "/W4")
	set(PROJECT_LINKER_FLAGS "")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(TXT2GAM_SOURCES
	src/coding.c
	src/locations.c
	src/main.c
	src/memwatch.c
	src/text.c
)
add_executable(txt2gam ${TXT2GAM_SOURCES})
target_compile_definitions(txt2gam PRIVATE _UNICODE)
if (WIN32)
	target_compile_definitions(txt2gam PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
if (SPEC_PASS)
	target_compile_definitions(txt2gam PRIVATE SPEC_PASS=${SPEC_PASS})
endif()
target_compile_options(txt2gam PRIVATE ${PROJECT_COMPILER_FLAGS})
target_link_options(txt2gam PRIVATE ${PROJECT_LINKER_FLAGS})
install(TARGETS txt2gam
		RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
		LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
		ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/static"
)
