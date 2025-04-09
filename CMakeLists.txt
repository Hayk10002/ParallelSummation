cmake_minimum_required(VERSION 3.10)

project(AtomicCounter)

set(CMAKE_CXX_STANDARD 23)

add_executable(main main.cpp)

# Add the main executable as a test
enable_testing()
add_test(NAME MainTest2 COMMAND main 100000000 10)
add_test(NAME MainTest3 COMMAND main 100000000 100)
add_test(NAME MainTest4 COMMAND main 100000000 1000)
add_test(NAME MainTest5 COMMAND main 100000000 10000)
add_test(NAME MainTest6 COMMAND main 100000000 100000)
add_test(NAME MainTest7 COMMAND main 100000000 1000000)

add_test(NAME MainTest12 COMMAND main 500000000 10)
add_test(NAME MainTest13 COMMAND main 500000000 100)
add_test(NAME MainTest14 COMMAND main 500000000 1000)
add_test(NAME MainTest15 COMMAND main 500000000 10000)
add_test(NAME MainTest16 COMMAND main 500000000 100000)
add_test(NAME MainTest17 COMMAND main 500000000 1000000)