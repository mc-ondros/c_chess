cmake_minimum_required(VERSION 3.10)
project(c_chess C)

set(CMAKE_C_STANDARD 99)

# Include all source files in the project
add_executable(c_chess 
    main.c 
    board.c 
    moves.c
)
