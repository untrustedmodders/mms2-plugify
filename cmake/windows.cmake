macro(set_common_compile_definitions TARGET_NAME)
    target_compile_definitions(${TARGET_NAME} PRIVATE
            META_IS_SOURCE2
            COMPILER_MSVC
            COMPILER_MSVC64
            _WIN32
            _WINDOWS
            _ALLOW_KEYWORD_MACROS
            __STDC_LIMIT_MACROS
            _CRT_SECURE_NO_WARNINGS=1
            _CRT_SECURE_NO_DEPRECATE=1
            _CRT_NONSTDC_NO_DEPRECATE=1
    )
endmacro()

# Define the macro to set MSVC specific compile options
macro(set_compile_options TARGET_NAME)
    target_compile_options(${TARGET_NAME} PRIVATE
            /wd4819
            /wd4828
            /wd5033
            /permissive-
            /utf-8
            /wd4005
    )
endmacro()

set_common_compile_definitions(${PROJECT_NAME})
set_compile_options(${PROJECT_NAME})

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