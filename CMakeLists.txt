# CMakeList.txt : CMake project for discordbot, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("discordbot")

add_subdirectory(libs/DPP)
# Add source to this project's executable.
add_executable (discordbot "discordbot.cpp" "discordbot.h")

target_link_libraries(${PROJECT_NAME}
    dpp
    curl
)
target_include_directories(${PROJECT_NAME} PRIVATE
    libs/DPP/include
    libs/openai
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17 # or 20 if you want something more recent
    CXX_STANDARD_REQUIRED ON
)
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET discordbot PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
