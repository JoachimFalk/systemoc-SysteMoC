CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

FIND_PACKAGE(PkgConfig REQUIRED)

PROJECT(main CXX)

PKG_CHECK_MODULES(SYSTEMOC REQUIRED libsystemoc>=1.0)

UNSET(SYSTEMOC_STATIC_LIBRARIES CACHE)
UNSET(SYSTEMOC_STATIC_LDFLAGS_OTHER CACHE)
UNSET(SYSTEMOC_STATIC_LIBRARY_DIRS CACHE)
FOREACH(item ${SYSTEMOC_STATIC_LDFLAGS})
  IF(${item} MATCHES "^/.*/lib([^/]*)\\.(la|a|so)$")
    STRING(REGEX REPLACE "^(/.*)/lib([^/]*)\\.(la|a|so)$" "\\1" var_libdir ${item})
    STRING(REGEX REPLACE "^(/.*)/lib([^/]*)\\.(la|a|so)$" "\\2" var_lib    ${item})
    LIST(APPEND SYSTEMOC_STATIC_LIBRARY_DIRS ${var_libdir} ${var_libdir}/.libs)
    LIST(APPEND SYSTEMOC_STATIC_LIBRARIES    ${var_lib})
    UNSET(var_libdir)
    UNSET(var_lib)
  ELSEIF(${item} MATCHES "^-L[ \t]*(.*)$")
    STRING(REGEX REPLACE "^-L[ \t]*(.*)$" "\\1" var_libdir ${item})
    LIST(APPEND SYSTEMOC_STATIC_LIBRARY_DIRS ${var_libdir})
    UNSET(var_libdir)
  ELSEIF(${item} MATCHES "^-l[ \t]*(.*)$")
    STRING(REGEX REPLACE "^-l[ \t]*(.*)$" "\\1" var_lib ${item})
    LIST(APPEND SYSTEMOC_STATIC_LIBRARIES ${var_lib})
    UNSET(var_lib)
  ELSE()
    LIST(APPEND SYSTEMOC_STATIC_LDFLAGS_OTHER ${item})
  ENDIF()
ENDFOREACH()

#MESSAGE(STATUS "SYSTEMOC_STATIC_LIBRARIES:     \"${SYSTEMOC_STATIC_LIBRARIES}\"")
#MESSAGE(STATUS "SYSTEMOC_STATIC_LDFLAGS_OTHER: \"${SYSTEMOC_STATIC_LDFLAGS_OTHER}\"")
#MESSAGE(STATUS "SYSTEMOC_STATIC_LIBRARY_DIRS:  \"${SYSTEMOC_STATIC_LIBRARY_DIRS}\"")

ADD_LIBRARY(SysteMoC INTERFACE IMPORTED)
SET_TARGET_PROPERTIES(SysteMoC PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${SYSTEMOC_INCLUDE_DIRS}"
  INTERFACE_COMPILE_OPTIONS     "${SYSTEMOC_CFLAGS_OTHER}"
  INTERFACE_LINK_LIBRARIES      "${SYSTEMOC_STATIC_LIBRARIES};${SYSTEMOC_STATIC_LDFLAGS_OTHER}"
# INTERFACE_LINK_DIRECTORIES    "${SYSTEMOC_STATIC_LIBRARY_DIRS}" requires cmake >= 3.13
)

LINK_DIRECTORIES(${SYSTEMOC_STATIC_LIBRARY_DIRS})

ADD_EXECUTABLE(data-flow
  data-flow/cpp/data-flow.cpp)
TARGET_LINK_LIBRARIES(data-flow SysteMoC)

ADD_EXECUTABLE(guards
  guards/cpp/guards.cpp)
TARGET_LINK_LIBRARIES(guards SysteMoC)

ADD_EXECUTABLE(hello-systemoc
  hello-systemoc/cpp/hello-systemoc.cpp)
TARGET_LINK_LIBRARIES(hello-systemoc SysteMoC)

ADD_EXECUTABLE(mini-filter
  mini-filter/cpp/mini-filter.cpp)
TARGET_LINK_LIBRARIES(mini-filter SysteMoC)

ADD_EXECUTABLE(ping
  ping/cpp/ping.cpp)
TARGET_LINK_LIBRARIES(ping SysteMoC)

ADD_EXECUTABLE(rates
  rates/cpp/rates.cpp)
TARGET_LINK_LIBRARIES(rates SysteMoC)

ADD_EXECUTABLE(source-sink
  source-sink/cpp/source-sink.cpp)
TARGET_LINK_LIBRARIES(source-sink SysteMoC)
