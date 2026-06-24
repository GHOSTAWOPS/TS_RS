add_executable(tsrs_app_smoke_tests
    smoke_tests.cpp
)

target_link_libraries(tsrs_app_smoke_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_app_smoke_tests COMMAND $<TARGET_FILE:tsrs_app_smoke_tests>)

add_executable(tsrs_space_list_parser_tests
    rebarsmart/space_list_parser_tests.cpp
)

target_link_libraries(tsrs_space_list_parser_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_space_list_parser_tests COMMAND $<TARGET_FILE:tsrs_space_list_parser_tests>)

add_executable(tsrs_priority_count_distributor_tests
    rebarsmart/priority_count_distributor_tests.cpp
)

target_link_libraries(tsrs_priority_count_distributor_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_priority_count_distributor_tests COMMAND $<TARGET_FILE:tsrs_priority_count_distributor_tests>)

add_executable(tsrs_priority_space_distributor_tests
    rebarsmart/priority_space_distributor_tests.cpp
)

target_link_libraries(tsrs_priority_space_distributor_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_priority_space_distributor_tests COMMAND $<TARGET_FILE:tsrs_priority_space_distributor_tests>)

add_executable(tsrs_priority_list_distributor_tests
    rebarsmart/priority_list_distributor_tests.cpp
)

target_link_libraries(tsrs_priority_list_distributor_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_priority_list_distributor_tests COMMAND $<TARGET_FILE:tsrs_priority_list_distributor_tests>)

add_executable(tsrs_guide_surface_offset_calculator_tests
    rebarsmart/guide_surface_offset_calculator_tests.cpp
)

target_link_libraries(tsrs_guide_surface_offset_calculator_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_guide_surface_offset_calculator_tests COMMAND $<TARGET_FILE:tsrs_guide_surface_offset_calculator_tests>)

add_executable(tsrs_guide_curve_zone_calculator_tests
    rebarsmart/guide_curve_zone_calculator_tests.cpp
)

target_link_libraries(tsrs_guide_curve_zone_calculator_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_guide_curve_zone_calculator_tests COMMAND $<TARGET_FILE:tsrs_guide_curve_zone_calculator_tests>)

add_executable(tsrs_fix_distance_generator_tests
    rebarsmart/fix_distance_generator_tests.cpp
)

target_link_libraries(tsrs_fix_distance_generator_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_fix_distance_generator_tests COMMAND $<TARGET_FILE:tsrs_fix_distance_generator_tests>)

add_executable(tsrs_fix_number_generator_tests
    rebarsmart/fix_number_generator_tests.cpp
)

target_link_libraries(tsrs_fix_number_generator_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_fix_number_generator_tests COMMAND $<TARGET_FILE:tsrs_fix_number_generator_tests>)

add_executable(tsrs_rebar_model_transaction_tests
    domain/rebar_model_transaction_tests.cpp
)

target_link_libraries(tsrs_rebar_model_transaction_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_rebar_model_transaction_tests
    COMMAND $<TARGET_FILE:tsrs_rebar_model_transaction_tests>)

add_executable(tsrs_geometry_engine_contract_tests
    geometry/geometry_engine_contract_tests.cpp
)

target_link_libraries(tsrs_geometry_engine_contract_tests
    PRIVATE
        tsrs_app_core
)

add_test(NAME tsrs_geometry_engine_contract_tests COMMAND $<TARGET_FILE:tsrs_geometry_engine_contract_tests>)

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

add_test(NAME tsrs_occt_geometry_engine_tests COMMAND $<TARGET_FILE:tsrs_occt_geometry_engine_tests>)

if(TSRS_VCPKG_INSTALLED_DIR)
    set_tests_properties(tsrs_occt_geometry_engine_tests PROPERTIES
        ENVIRONMENT "PATH=${TSRS_VCPKG_INSTALLED_DIR}/bin;$ENV{PATH}")
endif()

add_executable(tsrs_step_import_service_tests
    step/step_import_service_tests.cpp
)

target_link_libraries(tsrs_step_import_service_tests
    PRIVATE
        tsrs_step
)

if(TSRS_VCPKG_RUNTIME_BIN_DIR)
    add_custom_command(TARGET tsrs_step_import_service_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${TSRS_VCPKG_RUNTIME_BIN_DIR}"
            $<TARGET_FILE_DIR:tsrs_step_import_service_tests>
        VERBATIM)
endif()

add_test(NAME tsrs_step_import_service_tests COMMAND $<TARGET_FILE:tsrs_step_import_service_tests>)

if(TSRS_VCPKG_INSTALLED_DIR)
    set_tests_properties(tsrs_step_import_service_tests PROPERTIES
        ENVIRONMENT "PATH=${TSRS_VCPKG_INSTALLED_DIR}/bin;$ENV{PATH}")
endif()

add_executable(tsrs_topology_binding_registry_tests
    step/topology_binding_registry_tests.cpp
)

target_link_libraries(tsrs_topology_binding_registry_tests
    PRIVATE
        tsrs_step
)

if(TSRS_VCPKG_RUNTIME_BIN_DIR)
    add_custom_command(TARGET tsrs_topology_binding_registry_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${TSRS_VCPKG_RUNTIME_BIN_DIR}"
            $<TARGET_FILE_DIR:tsrs_topology_binding_registry_tests>
        VERBATIM)
endif()

add_test(NAME tsrs_topology_binding_registry_tests
    COMMAND $<TARGET_FILE:tsrs_topology_binding_registry_tests>)

if(TSRS_VCPKG_INSTALLED_DIR)
    set_tests_properties(tsrs_topology_binding_registry_tests PROPERTIES
        ENVIRONMENT "PATH=${TSRS_VCPKG_INSTALLED_DIR}/bin;$ENV{PATH}")
endif()

add_executable(tsrs_step_import_command_service_tests
    application/step_import_command_service_tests.cpp
)

target_link_libraries(tsrs_step_import_command_service_tests
    PRIVATE
        tsrs_application
)

if(TSRS_VCPKG_RUNTIME_BIN_DIR)
    add_custom_command(TARGET tsrs_step_import_command_service_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${TSRS_VCPKG_RUNTIME_BIN_DIR}"
            $<TARGET_FILE_DIR:tsrs_step_import_command_service_tests>
        VERBATIM)
endif()

add_test(NAME tsrs_step_import_command_service_tests COMMAND $<TARGET_FILE:tsrs_step_import_command_service_tests>)

if(TSRS_VCPKG_INSTALLED_DIR)
    set_tests_properties(tsrs_step_import_command_service_tests PROPERTIES
        ENVIRONMENT "PATH=${TSRS_VCPKG_INSTALLED_DIR}/bin;$ENV{PATH}")
endif()

add_executable(tsrs_command_service_guardrails_tests
    application/command_service_guardrails_tests.cpp
)

target_link_libraries(tsrs_command_service_guardrails_tests
    PRIVATE
        tsrs_application
)

add_test(NAME tsrs_command_service_guardrails_tests
    COMMAND $<TARGET_FILE:tsrs_command_service_guardrails_tests>)

add_executable(tsrs_rebar_creation_command_service_tests
    application/rebar_creation_command_service_tests.cpp
)

target_link_libraries(tsrs_rebar_creation_command_service_tests
    PRIVATE
        tsrs_application
)

add_test(NAME tsrs_rebar_creation_command_service_tests
    COMMAND $<TARGET_FILE:tsrs_rebar_creation_command_service_tests>)

add_executable(tsrs_tsrebar_project_file_tests
    project/tsrebar_project_file_tests.cpp
)

target_link_libraries(tsrs_tsrebar_project_file_tests
    PRIVATE
        tsrs_project_tsrebar
)

add_test(NAME tsrs_tsrebar_project_file_tests
    COMMAND $<TARGET_FILE:tsrs_tsrebar_project_file_tests>)

set_tests_properties(tsrs_tsrebar_project_file_tests PROPERTIES
    TIMEOUT 10)

if(TSRS_VCPKG_RUNTIME_BIN_DIR OR TSRS_VCPKG_DEBUG_RUNTIME_BIN_DIR)
    set(_tsrs_project_runtime_path "$<$<CONFIG:Debug>:${TSRS_VCPKG_DEBUG_RUNTIME_BIN_DIR}>$<$<NOT:$<CONFIG:Debug>>:${TSRS_VCPKG_RUNTIME_BIN_DIR}>")
    set_tests_properties(tsrs_tsrebar_project_file_tests PROPERTIES
        ENVIRONMENT "PATH=${_tsrs_project_runtime_path};$ENV{PATH}"
        TIMEOUT 10)
endif()

add_executable(tsrs_command_service_boundary_scan_tests
    application/command_service_boundary_scan_tests.cpp
)

add_test(NAME tsrs_command_service_boundary_scan_tests
    COMMAND $<TARGET_FILE:tsrs_command_service_boundary_scan_tests>)

add_executable(tsrs_step_import_local_sample_probe_tests
    application/step_import_local_sample_probe_tests.cpp
)

target_link_libraries(tsrs_step_import_local_sample_probe_tests
    PRIVATE
        tsrs_application
)

if(TSRS_VCPKG_RUNTIME_BIN_DIR)
    add_custom_command(TARGET tsrs_step_import_local_sample_probe_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${TSRS_VCPKG_RUNTIME_BIN_DIR}"
            $<TARGET_FILE_DIR:tsrs_step_import_local_sample_probe_tests>
        VERBATIM)
endif()

add_test(NAME tsrs_step_import_local_sample_probe_tests
    COMMAND $<TARGET_FILE:tsrs_step_import_local_sample_probe_tests>)

if(TSRS_VCPKG_INSTALLED_DIR)
    set_tests_properties(tsrs_step_import_local_sample_probe_tests PROPERTIES
        ENVIRONMENT "PATH=${TSRS_VCPKG_INSTALLED_DIR}/bin;$ENV{PATH}")
endif()

add_executable(tsrs_detail_package_reader_tests
    drawing/detail_package_reader_tests.cpp
)

target_link_libraries(tsrs_detail_package_reader_tests
    PRIVATE
        tsrs_detail
)

add_test(NAME tsrs_detail_package_reader_tests
    COMMAND $<TARGET_FILE:tsrs_detail_package_reader_tests>)

add_executable(tsrs_detail_package_writer_tests
    drawing/detail_package_writer_tests.cpp
)

target_link_libraries(tsrs_detail_package_writer_tests
    PRIVATE
        tsrs_detail
)

add_test(NAME tsrs_detail_package_writer_tests
    COMMAND $<TARGET_FILE:tsrs_detail_package_writer_tests>)

add_test(NAME tsrs_detail_package_reader_todo66_fixture_probe
    COMMAND $<TARGET_FILE:tsrs_detail_package_reader_tests> --todo66-fixture)
set_tests_properties(tsrs_detail_package_reader_todo66_fixture_probe PROPERTIES
    SKIP_RETURN_CODE 77)

add_test(NAME tsrs_detail_package_writer_todo66_fixture_probe
    COMMAND $<TARGET_FILE:tsrs_detail_package_writer_tests> --todo66-fixture)
set_tests_properties(tsrs_detail_package_writer_todo66_fixture_probe PROPERTIES
    SKIP_RETURN_CODE 77)

if(TSRS_VCPKG_RUNTIME_BIN_DIR OR TSRS_VCPKG_DEBUG_RUNTIME_BIN_DIR)
    set(_tsrs_detail_runtime_path "$<$<CONFIG:Debug>:${TSRS_VCPKG_DEBUG_RUNTIME_BIN_DIR}>$<$<NOT:$<CONFIG:Debug>>:${TSRS_VCPKG_RUNTIME_BIN_DIR}>")
    set_tests_properties(tsrs_detail_package_reader_tests PROPERTIES
        ENVIRONMENT "PATH=${_tsrs_detail_runtime_path};$ENV{PATH}")
    set_tests_properties(tsrs_detail_package_writer_tests PROPERTIES
        ENVIRONMENT "PATH=${_tsrs_detail_runtime_path};$ENV{PATH}")
    set_tests_properties(tsrs_detail_package_reader_todo66_fixture_probe PROPERTIES
        ENVIRONMENT "PATH=${_tsrs_detail_runtime_path};$ENV{PATH}"
        SKIP_RETURN_CODE 77)
    set_tests_properties(tsrs_detail_package_writer_todo66_fixture_probe PROPERTIES
        ENVIRONMENT "PATH=${_tsrs_detail_runtime_path};$ENV{PATH}"
        SKIP_RETURN_CODE 77)
endif()

add_executable(tsrs_step_model_display_style_tests
    presentation/step_model_display_style_tests.cpp
)

target_link_libraries(tsrs_step_model_display_style_tests
    PRIVATE
        tsrs_ui
)

add_test(NAME tsrs_step_model_display_style_tests COMMAND $<TARGET_FILE:tsrs_step_model_display_style_tests>)

if(TSRS_QT_PLATFORM_PLUGIN_PATH OR TSRS_QT_DEBUG_PLATFORM_PLUGIN_PATH)
    set(_tsrs_qt_platform_plugin_path "$<$<CONFIG:Debug>:${TSRS_QT_DEBUG_PLATFORM_PLUGIN_PATH}>$<$<NOT:$<CONFIG:Debug>>:${TSRS_QT_PLATFORM_PLUGIN_PATH}>")
    set(_tsrs_qt_runtime_path "$<$<CONFIG:Debug>:${TSRS_VCPKG_DEBUG_RUNTIME_BIN_DIR}>$<$<NOT:$<CONFIG:Debug>>:${TSRS_VCPKG_RUNTIME_BIN_DIR}>")
endif()

add_executable(tsrs_occ_viewer_selection_tests
    presentation/occ_viewer_selection_tests.cpp
)

target_link_libraries(tsrs_occ_viewer_selection_tests
    PRIVATE
        tsrs_ui
)

add_test(NAME tsrs_occ_viewer_selection_tests
    COMMAND $<TARGET_FILE:tsrs_occ_viewer_selection_tests>)

if(TSRS_QT_PLATFORM_PLUGIN_PATH OR TSRS_QT_DEBUG_PLATFORM_PLUGIN_PATH)
    set_tests_properties(tsrs_occ_viewer_selection_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=minimal;QT_QPA_PLATFORM_PLUGIN_PATH=${_tsrs_qt_platform_plugin_path};PATH=${_tsrs_qt_runtime_path};$ENV{PATH}")
endif()

add_executable(tsrs_main_window_skeleton_tests
    ui/main_window_skeleton_tests.cpp
)

target_link_libraries(tsrs_main_window_skeleton_tests
    PRIVATE
        tsrs_ui
)

add_test(NAME tsrs_main_window_skeleton_tests COMMAND $<TARGET_FILE:tsrs_main_window_skeleton_tests>)

if(TSRS_QT_PLATFORM_PLUGIN_PATH OR TSRS_QT_DEBUG_PLATFORM_PLUGIN_PATH)
    set_tests_properties(tsrs_main_window_skeleton_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=minimal;QT_QPA_PLATFORM_PLUGIN_PATH=${_tsrs_qt_platform_plugin_path};PATH=${_tsrs_qt_runtime_path};$ENV{PATH}")
endif()

add_executable(tsrs_app_startup_smoke_tests
    ui/app_startup_smoke_tests.cpp
)

target_link_libraries(tsrs_app_startup_smoke_tests
    PRIVATE
        Qt6::Core
        tsrs_application
)

add_test(NAME tsrs_app_startup_smoke_tests
    COMMAND $<TARGET_FILE:tsrs_app_startup_smoke_tests> $<TARGET_FILE:tsrs_app>)

if(TSRS_QT_PLATFORM_PLUGIN_PATH OR TSRS_QT_DEBUG_PLATFORM_PLUGIN_PATH)
    set(_tsrs_qt_platform_plugin_path "$<$<CONFIG:Debug>:${TSRS_QT_DEBUG_PLATFORM_PLUGIN_PATH}>$<$<NOT:$<CONFIG:Debug>>:${TSRS_QT_PLATFORM_PLUGIN_PATH}>")
    set(_tsrs_qt_runtime_path "$<$<CONFIG:Debug>:${TSRS_VCPKG_DEBUG_RUNTIME_BIN_DIR}>$<$<NOT:$<CONFIG:Debug>>:${TSRS_VCPKG_RUNTIME_BIN_DIR}>")
    set_tests_properties(tsrs_app_startup_smoke_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=minimal;QT_QPA_PLATFORM_PLUGIN_PATH=${_tsrs_qt_platform_plugin_path};PATH=${_tsrs_qt_runtime_path};$ENV{PATH}")
endif()
