set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
add_library(utils STATIC Utility.cpp DirScanner.cpp)
