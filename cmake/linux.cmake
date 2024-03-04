add_definitions(-D_LINUX -DPOSIX -DLINUX -DGNUC -DCOMPILER_GCC -DPLATFORM_64BITS -D_GLIBCXX_USE_CXX11_ABI=0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dstrnicmp=strncasecmp -D_snprintf=snprintf")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp")

# Warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-uninitialized -Wno-switch -Wno-unused")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-non-virtual-dtor -Wno-overloaded-virtual")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-conversion-null -Wno-write-strings")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof -Wno-reorder")

# Others
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpmath=sse -msse -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-threadsafe-statics -v -fvisibility=default")

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
