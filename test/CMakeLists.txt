add_executable(SimZipTest main.cpp)
target_link_libraries(SimZipTest PRIVATE SimZip)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data.txt
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME SimZipTest
        COMMAND $<TARGET_FILE:SimZipTest>
)

