add_subdirectory(utils)

add_library(wdet SHARED wdet.cpp Filter.cpp WordFind.cpp)
target_link_libraries(wdet utils)

add_executable(wdet_encrypt encrypt.cpp)
target_link_libraries(wdet_encrypt utils)

add_executable(wdet_test1 test1.cpp)
target_link_libraries(wdet_test1 wdet)

add_executable(wdet_test2 test2.cpp opendir.cpp)
target_link_libraries(wdet_test2 wdet)

install(TARGETS wdet LIBRARY DESTINATION lib)
install(FILES wdet.h DESTINATION include)
install(TARGETS wdet_encrypt wdet_test1 wdet_test2 DESTINATION bin)
