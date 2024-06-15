macro(set_common_compile_definitions TARGET_NAME)
    target_compile_definitions(${TARGET_NAME} PRIVATE
            META_IS_SOURCE2
            _LINUX
            POSIX
            LINUX
            GNUC
            COMPILER_GCC
            PLATFORM_64BITS
            _GLIBCXX_USE_CXX11_ABI=0
    )
endmacro()

macro(set_additional_compile_definitions TARGET_NAME)
    target_compile_definitions(${TARGET_NAME} PRIVATE
            stricmp=strcasecmp
            _stricmp=strcasecmp
            _strnicmp=strncasecmp
            strnicmp=strncasecmp
            _snprintf=snprintf
            _vsnprintf=vsnprintf
            _alloca=alloca
            strcmpi=strcasecmp
    )
endmacro()

macro(set_compile_options TARGET_NAME)
    target_compile_options(${TARGET_NAME} PRIVATE
            -Wall
            -Wno-uninitialized
            -Wno-switch
            -Wno-unused
            -Wno-non-virtual-dtor
            -Wno-overloaded-virtual
            -Wno-conversion-null
            -Wno-write-strings
            -Wno-invalid-offsetof
            -Wno-reorder
            -mfpmath=sse
            -msse
            -fno-strict-aliasing
            -fno-threadsafe-statics
            -v
            -fvisibility=default
    )
endmacro()

set_common_compile_definitions(${PROJECT_NAME})
set_additional_compile_definitions(${PROJECT_NAME})
set_compile_options(${PROJECT_NAME})

SET(PLUGIFY_LINK_LIBRARIES
        ${SOURCESDK_LIB}/linux64/libtier0.so
        ${SOURCESDK_LIB}/linux64/tier1.a
        ${SOURCESDK_LIB}/linux64/interfaces.a
        #${SOURCESDK_LIB}/linux64/mathlib.a
        miniz
        sha256
        curl
        plugify::plugify
)

if(NOT COMPILER_SUPPORTS_FORMAT)
    set(PLUGIFY_LINK_LIBRARIES ${PLUGIFY_LINK_LIBRARIES} fmt::fmt-header-only)
endif()