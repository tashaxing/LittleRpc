project(LittleRpc)
cmake_minimum_required(VERSION 2.8)

add_definitions(-std=c++11)

add_definitions(
   # zmq config
  -DZMQ_STATIC
)

add_subdirectory(deps/zeromq-4.2.1)

include_directories(
    deps/zeromq-4.2.1/include
	src/cpp
)

# server app
set(SERVER_SRC
    src/cpp/little_rpc.h
    src/cpp/little_rpc.cpp
    example/server.cpp
)

add_executable(LittleRpcServer ${SERVER_SRC})
target_link_libraries(LittleRpcServer libzmq-static)

# client app
set(CLIENT_SRC
    src/cpp/little_rpc.h
    src/cpp/little_rpc.cpp
    example/client.cpp
)

add_executable(LittleRpcClient ${CLIENT_SRC})
target_link_libraries(LittleRpcClient libzmq-static)
