add_executable(embview_tests
    test_Protocol.cpp
    test_DataStore.cpp
    test_TransportFactory.cpp
)

target_include_directories(embview_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(embview_tests PRIVATE
    embview_core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(embview_tests)
