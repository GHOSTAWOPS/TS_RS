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

find_package(OpenCASCADE CONFIG REQUIRED)

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

add_library(tsrs_app_core STATIC
    src/core/AppBuildInfo.cpp
    src/rebarsmart/space_list/SpaceListParser.cpp
    src/rebarsmart/distribute/PriorityCountDistributor.cpp
    src/rebarsmart/distribute/PrioritySpaceDistributor.cpp
    src/rebarsmart/distribute/PriorityListDistributor.cpp
    src/rebarsmart/zone/GuideSurfaceOffsetCalculator.cpp
    src/rebarsmart/zone/GuideCurveZoneCalculator.cpp
    src/rebarsmart/generators/FixDistanceGenerator.cpp
)

target_include_directories(tsrs_app_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(tsrs_app_core PUBLIC cxx_std_17)

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

enable_testing()
add_subdirectory(tests)
