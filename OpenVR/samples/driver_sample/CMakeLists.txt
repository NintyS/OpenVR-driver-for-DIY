cmake_minimum_required(VERSION 3.24)

project(driver_sample)

include_directories(../../headers)
include_directories(/usr/local/include)

set(TARGET_NAME driver_sample)

find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_library(${TARGET_NAME} MODULE
  basics.cpp
  basics.h
  driverlog.cpp
  driverlog.h
  driver_sample.cpp
  csampledevicedriver.cpp
  csampledevicedriver.h
  cserverdriver_sample.cpp
  cserverdriver_sample.h
  csamplecontrollerdriver.cpp
  csamplecontrollerdriver.h
  cwatchdogdriver_sample.cpp
  cwatchdogdriver_sample.h
  struct.h)

SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES PREFIX "")

target_link_libraries(${TARGET_NAME}
  ${OPENVR_LIBRARIES}
  ${CMAKE_DL_LIBS}
  -lsfml-network
  -lsfml-system
  -lsfml-window
  opencv_core opencv_imgproc opencv_video opencv_imgcodecs ${EXTERNAL_LIBS}
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARY}
  -lX11
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PREFIX "win")
else()
    set(PREFIX "linux")
endif()

if(CMAKE_SIZEOF_VOID_P STREQUAL "8")
    set(ARCH 64)
else()
    set(ARCH 64)
endif()

install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION drivers/sample/bin/${PREFIX}${ARCH}
    LIBRARY DESTINATION drivers/sample/bin/${PREFIX}${ARCH}
)

install(FILES default.vrsettings DESTINATION drivers/sample/resources/settings)
install(FILES default.vrdrivermanifest DESTINATION drivers/sample)
