
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
    itkAdcMapFilter.hxx 
    itkComputedDwiFilter.hxx 
    itkDwiIVIMFilter.hxx
    itkComputedEadcFilter.hxx
    itkDwiIVIMFilter2.hxx
    itkDisplayOptimizer.hxx
    itkMaskVectorImageFilter.hxx
    itkGetDiffusionImageFilter.hxx
    itkTensor.hxx
	DiffusionCore.cpp
    )

set( Hdrs 
     itkAdcMapFilter.h 
     itkComputedDwiFilter.h     
     itkDwiIVIMCostFunction.h
     itkDwiIVIMFilter.h
     itkComputedEadcFilter.h
     itkDwiIVIMCostFunction2.h
     itkDwiIVIMFilter2.h
     itkDisplayOptimizer.h
     itkMaskVectorImageFilter.h
     itkTensor.h
     itkGetDiffusionImageFilter.h
   )

set( MOC_Hdrs DiffusionCore.h)
set( UIs DiffusionCore.ui)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  "../Utility"
)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)


find_package(Qt5 COMPONENTS Widgets REQUIRED QUIET)
qt5_wrap_ui(UI_Srcs ${UIs})
add_library(DiffusionCore STATIC ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs} )
qt5_use_modules(DiffusionCore Core Gui Widgets Sql Xml UiTools XmlPatterns Concurrent)
target_link_libraries(DiffusionCore ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${Glue} Utility CTKWidgets)
# if(VTK_LIBRARIES)
  # target_link_libraries(DiffusionCore ${VTK_LIBRARIES})
# else()
  # target_link_libraries(DiffusionCore vtkHybrid vtkWidgets)
# endif()


