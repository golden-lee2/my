cmake_minimum_required(VERSION 3.10)

project(HelloWorld)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加包含目录
include_directories(${PROJECT_SOURCE_DIR}/database)
include_directories(${PROJECT_SOURCE_DIR}/database/include)
link_directories(${PROJECT_SOURCE_DIR}/database/lib)


find_package(Crow CONFIG REQUIRED)

add_executable(main main.cpp server/server.cpp database/database.cpp)
target_link_libraries(main PRIVATE Crow::Crow libmysql)
