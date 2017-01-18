find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# Set your files and resources here
set( Srcs DisplayHelper.cpp)
set( Hdrs DisplayHelper.h)

#set( MOC_Hdrs QmitkDicomExternalDataWidget.h QmitkDicomLocalStorageWidget.h DicomModule.h)

#set( UIs QmitkDicomLocalStorageWidgetControls.ui QmitkDicomExternalDataWidgetControls.ui DicomModule.ui)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED QUIET)
qt5_wrap_ui(UI_Srcs ${UIs})

# CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
add_library(DisplayCore STATIC ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})
qt5_use_modules(DisplayCore Core Gui Widgets Sql opengl Xml UiTools XmlPatterns Concurrent)
target_link_libraries(DisplayCore ${QT_LIBRARIES} ${VTK_LIBRARIES})
