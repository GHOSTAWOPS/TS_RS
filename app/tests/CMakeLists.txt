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

add_executable(tsrs_geometry_engine_contract_tests
    geometry/geometry_engine_contract_tests.cpp
)

target_link_libraries(tsrs_geometry_engine_contract_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_geometry_engine_contract_tests COMMAND tsrs_geometry_engine_contract_tests)

add_executable(tsrs_occt_geometry_engine_tests
    geometry/occt_geometry_engine_tests.cpp
)

target_link_libraries(tsrs_occt_geometry_engine_tests
    PRIVATE
        tsrs_geometry_occt
)

if(TSRS_VCPKG_RUNTIME_BIN_DIR)
    add_custom_command(TARGET tsrs_occt_geometry_engine_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${TSRS_VCPKG_RUNTIME_BIN_DIR}"
            $<TARGET_FILE_DIR:tsrs_occt_geometry_engine_tests>
        VERBATIM)
endif()

add_test(NAME tsrs_occt_geometry_engine_tests COMMAND tsrs_occt_geometry_engine_tests)

if(TSRS_VCPKG_INSTALLED_DIR)
    set_tests_properties(tsrs_occt_geometry_engine_tests PROPERTIES
        ENVIRONMENT "PATH=${TSRS_VCPKG_INSTALLED_DIR}/bin;$ENV{PATH}")
endif()
