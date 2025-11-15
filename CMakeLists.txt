cmake_minimum_required(VERSION 3.20)
project(Skeletal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Discover SDL3 and GLM
set(SDL_SHARED OFF) # optional: build static
set(SDL_STATIC ON)
add_subdirectory(external/SDL)

find_package(glm REQUIRED)

add_executable(Skeletal
    src/main.cpp
)

target_link_libraries(Skeletal PRIVATE
    SDL3::SDL3
    glm::glm
)

# Copy res directory to the target runtime directory after build
add_custom_command(TARGET Skeletal POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:Skeletal>/res"
    COMMENT "Copying res/ to runtime directory"
)