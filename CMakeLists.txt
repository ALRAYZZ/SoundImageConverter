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
)

# Include directories
target_include_directories(SoundImageConverter PRIVATE
		${CMAKE_SOURCE_DIR}/include
		${CMAKE_SOURCE_DIR}/lib/stb
		${CMAKE_SOURCE_DIR}/lib/libsndfile/
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

# Output directories for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Copy sndfile DLL
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