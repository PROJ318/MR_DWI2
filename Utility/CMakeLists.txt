

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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(Qt5 COMPONENTS Widgets REQUIRED QUIET)

# Set your files and resources here
set( Srcs displayport.cpp DicomHelper.cpp)

set( Hdrs displayport.h DicomHelper.h )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
add_library(Utility STATIC ${Srcs} ${Hdrs} )
qt5_use_modules(Utility Core Gui Widgets )
target_link_libraries(Utility ${QT_LIBRARIES} ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${Glue})
