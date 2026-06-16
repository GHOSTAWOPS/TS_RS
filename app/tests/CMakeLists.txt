add_executable(tsrs_app_smoke_tests
    smoke_tests.cpp
)

target_link_libraries(tsrs_app_smoke_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_app_smoke_tests COMMAND tsrs_app_smoke_tests)

add_executable(tsrs_space_list_parser_tests
    rebarsmart/space_list_parser_tests.cpp
)

target_link_libraries(tsrs_space_list_parser_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_space_list_parser_tests COMMAND tsrs_space_list_parser_tests)

add_executable(tsrs_priority_count_distributor_tests
    rebarsmart/priority_count_distributor_tests.cpp
)

target_link_libraries(tsrs_priority_count_distributor_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_priority_count_distributor_tests COMMAND tsrs_priority_count_distributor_tests)

add_executable(tsrs_priority_space_distributor_tests
    rebarsmart/priority_space_distributor_tests.cpp
)

target_link_libraries(tsrs_priority_space_distributor_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_priority_space_distributor_tests COMMAND tsrs_priority_space_distributor_tests)

add_executable(tsrs_priority_list_distributor_tests
    rebarsmart/priority_list_distributor_tests.cpp
)

target_link_libraries(tsrs_priority_list_distributor_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_priority_list_distributor_tests COMMAND tsrs_priority_list_distributor_tests)

add_executable(tsrs_guide_surface_offset_calculator_tests
    rebarsmart/guide_surface_offset_calculator_tests.cpp
)

target_link_libraries(tsrs_guide_surface_offset_calculator_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_guide_surface_offset_calculator_tests COMMAND tsrs_guide_surface_offset_calculator_tests)

add_executable(tsrs_guide_curve_zone_calculator_tests
    rebarsmart/guide_curve_zone_calculator_tests.cpp
)

target_link_libraries(tsrs_guide_curve_zone_calculator_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_guide_curve_zone_calculator_tests COMMAND tsrs_guide_curve_zone_calculator_tests)
