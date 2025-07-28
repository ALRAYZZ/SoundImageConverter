cmake_minimum_required(VERSION 3.15)
project(SoundImageConverter LANGUAGES CXX)

#Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Define the executable
add_executable(SoundImageConverter
	src/main.cpp
	src/Encoder.cpp
	src/Decoder.cpp
	lib/imgui/imgui.cpp
	lib/imgui/imgui_draw.cpp
	lib/imgui/imgui_widgets.cpp
	lib/imgui/imgui_tables.cpp
	lib/imgui/imgui_impl_sdl2.cpp
	lib/imgui/imgui_impl_opengl3.cpp
	lib/tinyfiledialogs/tinyfiledialogs.c
)

# Include directories
target_include_directories(SoundImageConverter PRIVATE
		${CMAKE_SOURCE_DIR}/include
		${CMAKE_SOURCE_DIR}/lib/stb
		${CMAKE_SOURCE_DIR}/lib/libsndfile/
		${CMAKE_SOURCE_DIR}/lib/SDL2/include
		${CMAKE_SOURCE_DIR}/lib/imgui
		${CMAKE_SOURCE_DIR}/lib/tinyfiledialogs
)

# Find libsndfile
find_library(SNDFILE_LIBRARY
	NAMES sndfile libsndfile
	PATHS ${CMAKE_SOURCE_DIR}/lib/libsndfile
	PATH_SUFFIXES lib
)

if (SNDFILE_LIBRARY)
target_link_libraries(SoundImageConverter PRIVATE ${SNDFILE_LIBRARY})
else()
	message(FATAL_ERROR "sndfile library not found. Please place sndfile in lib/libsndfile/.")
endif()

# Find SDL2
find_library(SDL2_LIBRARY
	NAMES SDL2
	PATHS ${CMAKE_SOURCE_DIR}/lib/SDL2/lib
)

find_library(SDL2MAIN_LIBRARY
NAMES SDL2main
PATHS ${CMAKE_SOURCE_DIR}/lib/SDL2/lib
)

if (SDL2_LIBRARY)
target_link_libraries(SoundImageConverter PRIVATE ${SDL2_LIBRARY})
else()
message(FATAL_ERROR "SDL2 not found. Please place SDL2 in lib/SDL2/")
endif()

# Link OpenGL (required by ImGui)
find_package(OpenGL REQUIRED)
target_link_libraries(SoundImageConverter PRIVATE OpenGL::GL)

# Output directories for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Copy sndfile and SDL2 DLLs to the build directory
if (WIN32)
	# Copy DLL to build directory
	add_custom_command(TARGET SoundImageConverter POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/lib/libsndfile/sndfile.dll"
		$<TARGET_FILE_DIR:SoundImageConverter>
	)

	# Also copy to the runtime output directory
	add_custom_command(TARGET SoundImageConverter POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/lib/libsndfile/sndfile.dll"
		"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	)
	add_custom_command(TARGET SoundImageConverter POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/lib/SDL2/lib/SDL2.dll"
		$<TARGET_FILE_DIR:SoundImageConverter>
	)
	add_custom_command(TARGET SoundImageConverter POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/lib/SDL2/lib/SDL2.dll"
		"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	)
endif()

# Copy resources folder to build directory
add_custom_command(TARGET SoundImageConverter POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/resources"
		$<TARGET_FILE_DIR:SoundImageConverter>/resources
)

# Optional: Enable warnings and optimizations
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(SoundImageConverter PRIVATE /W4)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(SoundImageConverter PRIVATE -Wall -Wextra -Wpedantic)
endif()