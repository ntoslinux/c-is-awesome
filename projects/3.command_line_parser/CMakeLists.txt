cmake_minimum_required(VERSION 3.4)
message("Before 'project':")
project(command_line_parser LANGUAGES C VERSION 1.2.3)
message("After 'project':")
message("  C: '${CMAKE_C_COMPILER}'")
message("  Version: '${PROJECT_VERSION}'")
message("  Version: '${command_line_parser_SOURCE_DIR}'")


add_executable(command_line_parser main.c)
