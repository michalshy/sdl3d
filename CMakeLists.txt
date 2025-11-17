cmake_minimum_required(VERSION 3.20)
project(Skeletal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(external)

# Discover SDL3 and GLM
set(SDL_SHARED OFF) # optional: build static
set(SDL_STATIC ON)
add_subdirectory(external/SDL)

add_subdirectory(external/glm)

add_subdirectory(external/assimp)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
)

# collect headers so IDEs show them (not required for build)
file(GLOB_RECURSE HDR_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(Skeletal ${SRC_FILES} ${HDR_FILES})

target_link_libraries(Skeletal PRIVATE
    SDL3::SDL3
    glm::glm
    assimp::assimp
)

if (WIN32 AND MSVC)
    set_target_properties(Skeletal PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Skeletal>"
    )
endif()

# Copy res directory to the target runtime directory after build
add_custom_command(TARGET Skeletal POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:Skeletal>/res"
    COMMENT "Copying res/ to runtime directory"
)