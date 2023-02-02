cmake_minimum_required(VERSION 3.1)
project(simple_threadpool)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-D_GLIBCXX_USE_CXX14_ABI=0)
add_compile_options(-g)

include_directories(${PROJECT_SOURCE_DIR}/include)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_FILES)
add_executable(threadpool ${SRC_FILES} main.cc)


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(threadpool Threads::Threads)