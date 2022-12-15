cmake_minimum_required(VERSION 3.11)
project(PICKME LANGUAGES C CXX)
find_package(xgboost REQUIRED)

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
FetchContent_MakeAvailable(json)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_executable(pickme main.cpp config.cpp controller.cpp ib.cpp invoker.cpp monitor.cpp setup.cpp sock.cpp utils.cpp)
# target_compile_options(pickme PUBLIC -Wall -std=c++17)
target_compile_options(pickme PUBLIC -Wall -std=c++17 -fsanitize=address -g)
target_link_options(pickme PUBLIC -fsanitize=address)
target_link_libraries(pickme xgboost::xgboost pthread rdmacm ibverbs boost_iostreams nlohmann_json::nlohmann_json)