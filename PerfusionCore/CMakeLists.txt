
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()
if(POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW) # CMake 3.0
endif()
if(POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW) # CMake 3.0
endif()
if(POLICY CMP0053)
  cmake_policy(SET CMP0053 NEW) # CMake 3.1
endif()

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
find_package(CTK REQUIRED)
include(${CTK_USE_FILE})

set( Srcs 
PerfusionCore.cpp
perfusionAlgorithm.cpp
    )

set( Hdrs 
itkPerfusionMapFilter.h
itkPerfusionMapFilter.hxx
perfusionAlgorithm.h
   )

set( MOC_Hdrs PerfusionCore.h)
set( UIs  PerfusionCore.ui)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  "../Utility"
)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)


find_package(Qt5 COMPONENTS Widgets REQUIRED QUIET)
qt5_wrap_ui(UI_Srcs ${UIs})
add_library(PerfusionCore STATIC ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs} )
qt5_use_modules(PerfusionCore Core Gui Widgets Sql Xml UiTools XmlPatterns Concurrent)
target_link_libraries(PerfusionCore ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${Glue} Utility CTKWidgets)
# if(VTK_LIBRARIES)
  # target_link_libraries(PerfusionCore ${VTK_LIBRARIES})
# else()
  # target_link_libraries(PerfusionCore vtkHybrid vtkWidgets)
# endif()


