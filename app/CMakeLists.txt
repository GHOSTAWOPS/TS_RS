cmake_minimum_required(VERSION 3.21)

project(TS_RS_App
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(EXISTS "D:/Work/vcpkg/installed/x64-windows")
    set(TSRS_VCPKG_INSTALLED_DIR "D:/Work/vcpkg/installed/x64-windows"
        CACHE PATH "Local vcpkg installed directory")
    list(PREPEND CMAKE_PREFIX_PATH "D:/Work/vcpkg/installed/x64-windows")
    if(EXISTS "D:/Work/vcpkg/installed/x64-windows/share/jxr/FindJXR.cmake")
        list(PREPEND CMAKE_MODULE_PATH "D:/Work/vcpkg/installed/x64-windows/share/jxr")
    endif()
    if(NOT DEFINED OpenCASCADE_DIR
       AND EXISTS "D:/Work/vcpkg/installed/x64-windows/share/opencascade/OpenCASCADEConfig.cmake")
        set(OpenCASCADE_DIR
            "D:/Work/vcpkg/installed/x64-windows/share/opencascade"
            CACHE PATH "OpenCASCADE package directory")
    endif()
endif()

set(TSRS_VCPKG_RUNTIME_BIN_DIR)
if(TSRS_VCPKG_INSTALLED_DIR AND EXISTS "${TSRS_VCPKG_INSTALLED_DIR}/bin")
    set(TSRS_VCPKG_RUNTIME_BIN_DIR "${TSRS_VCPKG_INSTALLED_DIR}/bin")
endif()

set(TSRS_VCPKG_DEBUG_RUNTIME_BIN_DIR)
if(TSRS_VCPKG_INSTALLED_DIR AND EXISTS "${TSRS_VCPKG_INSTALLED_DIR}/debug/bin")
    set(TSRS_VCPKG_DEBUG_RUNTIME_BIN_DIR "${TSRS_VCPKG_INSTALLED_DIR}/debug/bin")
endif()

set(TSRS_QT_PLATFORM_PLUGIN_PATH)
if(TSRS_VCPKG_INSTALLED_DIR
   AND EXISTS "${TSRS_VCPKG_INSTALLED_DIR}/Qt6/plugins/platforms")
    set(TSRS_QT_PLATFORM_PLUGIN_PATH
        "${TSRS_VCPKG_INSTALLED_DIR}/Qt6/plugins/platforms")
endif()

set(TSRS_QT_DEBUG_PLATFORM_PLUGIN_PATH)
if(TSRS_VCPKG_INSTALLED_DIR
   AND EXISTS "${TSRS_VCPKG_INSTALLED_DIR}/debug/Qt6/plugins/platforms")
    set(TSRS_QT_DEBUG_PLATFORM_PLUGIN_PATH
        "${TSRS_VCPKG_INSTALLED_DIR}/debug/Qt6/plugins/platforms")
endif()

find_package(OpenCASCADE CONFIG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

if(DEFINED OpenCASCADE_INCLUDE_DIR)
    set(TSRS_OCCT_INCLUDE_DIR "${OpenCASCADE_INCLUDE_DIR}")
elseif(DEFINED OpenCASCADE_INCLUDE_DIRS)
    set(TSRS_OCCT_INCLUDE_DIR "${OpenCASCADE_INCLUDE_DIRS}")
else()
    find_path(TSRS_OCCT_INCLUDE_DIR NAMES Standard.hxx)
endif()

if(NOT TSRS_OCCT_INCLUDE_DIR)
    message(FATAL_ERROR "Missing OCCT include directory.")
endif()

set(TSRS_OCCT_GEOMETRY_LIBS
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKGeomBase
    TKBRep
    TKGeomAlgo
    TKTopAlgo)

set(TSRS_OCCT_STEP_LIBS
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKGeomBase
    TKBRep
    TKGeomAlgo
    TKTopAlgo
    TKXSBase
    TKDE
    TKDESTEP)

set(TSRS_OCCT_VIEWER_LIBS
    TKService
    TKV3d
    TKOpenGl)

add_library(tsrs_app_core STATIC
    src/core/AppBuildInfo.cpp
    src/drawing/schedule/ScheduleModel.cpp
    src/domain/rebar/RebarModel.cpp
    src/rebarsmart/space_list/SpaceListParser.cpp
    src/rebarsmart/distribute/PriorityCountDistributor.cpp
    src/rebarsmart/distribute/PrioritySpaceDistributor.cpp
    src/rebarsmart/distribute/PriorityListDistributor.cpp
    src/rebarsmart/zone/GuideSurfaceOffsetCalculator.cpp
    src/rebarsmart/zone/GuideCurveZoneCalculator.cpp
    src/rebarsmart/generators/FixDistanceGenerator.cpp
    src/rebarsmart/generators/FixNumberGenerator.cpp
)

target_include_directories(tsrs_app_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(tsrs_app_core PUBLIC cxx_std_17)

add_library(tsrs_project_tsrebar STATIC
    src/project/tsrebar/TsrebarProjectFile.cpp
)

target_include_directories(tsrs_project_tsrebar
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(tsrs_project_tsrebar
    PUBLIC
        Qt6::Core
        tsrs_app_core
)

add_library(tsrs_geometry_occt STATIC
    src/geometry/occt/OcctGeometryEngine.cpp
)

target_include_directories(tsrs_geometry_occt
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        "${TSRS_OCCT_INCLUDE_DIR}"
)

target_link_libraries(tsrs_geometry_occt
    PUBLIC
        tsrs_app_core
        ${TSRS_OCCT_GEOMETRY_LIBS}
)

add_library(tsrs_step STATIC
    src/step/StepImportService.cpp
    src/step/StepUnitPolicy.cpp
    src/step/ShapeStore.cpp
    src/step/TopologyBindingRegistry.cpp
)

target_include_directories(tsrs_step
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        "${TSRS_OCCT_INCLUDE_DIR}"
)

target_link_libraries(tsrs_step
    PUBLIC
        ${TSRS_OCCT_STEP_LIBS}
)

add_library(tsrs_application STATIC
    src/application/commands/DetailPackageCommandService.cpp
    src/application/commands/RebarCreationCommandService.cpp
    src/application/commands/SelectionCommandService.cpp
    src/application/commands/StepImportCommandService.cpp
    src/application/import/ImportedModelStore.cpp
    src/application/rebarsmart/RebarCreationCommandServiceFactory.cpp
    src/application/rebarsmart/RebarSmartFixDistanceCenterlineGeneratorAdapter.cpp
)

target_include_directories(tsrs_application
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        "${TSRS_OCCT_INCLUDE_DIR}"
)

target_link_libraries(tsrs_application
    PUBLIC
        tsrs_app_core
        tsrs_step
)

add_library(tsrs_detail STATIC
    src/drawing/detail/DetailPackageReader.cpp
    src/drawing/detail/DetailPackageWriter.cpp
)

target_include_directories(tsrs_detail
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(tsrs_detail
    PUBLIC
        Qt6::Core
)

add_library(tsrs_ui STATIC
    src/presentation/occ/OccViewerWidget.cpp
    src/presentation/style/StepModelDisplayStyle.cpp
    src/ui/mainwindow/MainWindow.cpp
)

set_target_properties(tsrs_ui PROPERTIES AUTOMOC ON)

target_include_directories(tsrs_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        "${TSRS_OCCT_INCLUDE_DIR}"
)

target_link_libraries(tsrs_ui
    PUBLIC
        Qt6::Widgets
        tsrs_application
        ${TSRS_OCCT_VIEWER_LIBS}
)

qt_add_executable(tsrs_app
    src/app/main.cpp
)

set_target_properties(tsrs_app PROPERTIES AUTOMOC ON)

target_link_libraries(tsrs_app
    PRIVATE
        tsrs_ui
)

set_target_properties(tsrs_app PROPERTIES
    WIN32_EXECUTABLE ON
)

enable_testing()
add_subdirectory(tests)
