cmake_minimum_required(VERSION 3.20)
project(coco VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Enable Qt Auto-tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 2. Find Packages
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(OpenCASCADE REQUIRED)
find_package(VTK REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    FiltersSources
    RenderingCore
    RenderingOpenGL2
    GUISupportQt
)

# 3. Define the Executable (Qt6 Style)
# MANUAL_FINALIZATION is recommended for complex projects to handle deployment/plugins
qt_add_executable(${PROJECT_NAME} 
    MANUAL_FINALIZATION
    src/main.cpp
)

# 4. Source Files (Crucial: Included the .ui file here)
target_sources(${PROJECT_NAME} PRIVATE 
    src/app/Application.cpp 
    src/app/ViewportController.cpp
    src/ui/MainWindow.cpp 
    src/ui/OcctWidget.cpp 
    src/core/HullModel.cpp
    src/render/OcctViewport.cpp
    src/render/VisualPoint.cpp
    
    src/ui/MainWindow.ui
    src/ui/images.qrc
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

# 5. Platform Specifics
if(APPLE)
    # Objective-C++ support for Cocoa window handles
    set_source_files_properties(
        src/main.cpp
        src/render/OcctViewport.cpp
        PROPERTIES COMPILE_FLAGS "-x objective-c++"
    )
    
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.coco.hullapp"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework AppKit" "-framework OpenGL")
endif()

if(WIN32)
    # Hide the console window on Windows
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# 6. General Linkage
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Widgets
    Qt6::OpenGLWidgets
    
    # OCCT Libraries
    TKernel TKMath TKG2d TKG3d TKGeomBase
    TKBRep TKTopAlgo TKPrim TKService TKV3d TKOpenGl
    
    # VTK Libraries (explicit list avoids over-linking/auto-init issues)
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::FiltersSources
    VTK::RenderingCore
    VTK::RenderingOpenGL2
    VTK::GUISupportQt
)

# 7. Include Paths
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${OpenCASCADE_INCLUDE_DIR}
    ${VTK_INCLUDE_DIRS}
)

# 8. VTK Initialization
vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES
        VTK::CommonCore
        VTK::CommonDataModel
        VTK::FiltersSources
        VTK::RenderingCore
        VTK::RenderingOpenGL2
        VTK::GUISupportQt
)

# 9. Windows Deployment (Post-Build)
if(WIN32 AND TARGET ${PROJECT_NAME})
    find_program(
        WINDEPLOYQT_EXE
        NAMES windeployqt.exe windeployqt
        PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/Qt6/bin"
              "${Qt6_DIR}/../../../bin"
        NO_DEFAULT_PATH)

    if(WINDEPLOYQT_EXE)
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND
                $<$<CONFIG:Release>:${WINDEPLOYQT_EXE}>
                $<$<CONFIG:Release>:--no-compiler-runtime>
                $<$<CONFIG:Release>:--verbose> $<$<CONFIG:Release>:0>
                $<$<CONFIG:Release>:$<TARGET_FILE:${PROJECT_NAME}>>
            COMMENT "Running windeployqt for Release build..."
            VERBATIM)
    endif()
endif()

# 10. Finalize (Qt 6 Best Practice)
qt_finalize_executable(${PROJECT_NAME})
