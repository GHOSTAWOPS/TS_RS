add_executable(tsrs_app_smoke_tests
    smoke_tests.cpp
)

target_link_libraries(tsrs_app_smoke_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_app_smoke_tests COMMAND tsrs_app_smoke_tests)
