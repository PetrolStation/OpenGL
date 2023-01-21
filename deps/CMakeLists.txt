set( USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "Use MSVC runtime library DLL" FORCE )
set( GLAD_INSTALL                 ON  CACHE BOOL "Generate installation target" FORCE )

add_subdirectory(glad/cmake)