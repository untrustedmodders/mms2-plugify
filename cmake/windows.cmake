add_definitions(
    -DCOMPILER_MSVC -DCOMPILER_MSVC64 -D_WIN32 -D_WINDOWS -D_ALLOW_KEYWORD_MACROS -D__STDC_LIMIT_MACROS
    -D_CRT_SECURE_NO_WARNINGS=1 -D_CRT_SECURE_NO_DEPRECATE=1 -D_CRT_NONSTDC_NO_DEPRECATE=1
)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819 /wd4828 /wd5033 /permissive- /utf-8 /wd4005 /MP")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF")

set(PLUGIFY_LINK_LIBRARIES
        ${SOURCESDK_LIB}/public/win64/tier0.lib
        ${SOURCESDK_LIB}/public/win64/tier1.lib
        ${SOURCESDK_LIB}/public/win64/interfaces.lib
        #${SOURCESDK_LIB}/public/win64/mathlib.lib
        miniz
        sha256
        winhttp.lib
        plugify::plugify
)

if(NOT COMPILER_SUPPORTS_FORMAT)
    set(PLUGIFY_LINK_LIBRARIES ${PLUGIFY_LINK_LIBRARIES} fmt::fmt-header-only)
endif()