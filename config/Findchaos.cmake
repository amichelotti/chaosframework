IF(CMAKE_INSTALL_PREFIX)
  set(PREFIX ${CMAKE_INSTALL_PREFIX})
ELSE()
  set(PREFIX ${CMAKE_CURRENT_LIST_DIR})
ENDIF()

execute_process(COMMAND "grep CHAOS_ENABLE_C11:BOOL=OFF ${PREFIX}/CMakeConfiguration.txt"   
   RESULT_VARIABLE retcode)

 IF(CMAKE_CXX_COMPILER_VERSION)
   if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
     message(FATAL_ERROR "Host GCC ${CMAKE_CXX_COMPILER_VERSION} version must be at least 4.8!")
   endif()   
ENDIF()

 IF(NOT CHAOS_TARGET)
  ADD_DEFINITIONS(-DCHAOS -fPIC)
ELSE()
  ADD_DEFINITIONS(-DCHAOS -fPIC -DFORCE_BOOST_SHPOINTER)
  message( STATUS "Forcing to use BOOST SHPOINTER")
ENDIF()

if(NOT "${retcode}" STREQUAL "0")

  ADD_DEFINITIONS(-std=c++11)

ELSE()
  message(STATUS "Force Static C98 compatibility compilation")
  ADD_DEFINITIONS(-DFORCE_BOOST_SHPOINTER)
  ADD_DEFINITIONS(-std=c++98)
ENDIF()
if(CMAKE_BUILD_TYPE MATCHES PROFILE)
   MESSAGE(STATUS "ENABLING PROFILE on ${PROJECT_NAME}")
   SET(GCC_COVERAGE_COMPILE_FLAGS "-O0 -g -fprofile-arcs -ftest-coverage")
   SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
   SET(GCC_COVERAGE_LINK_LIB    "gcov")

   SET(GCC_COVERAGE_LINK_FLAGS    "-fprofile-arcs -ftest-coverage -coverage")
   ADD_DEFINITIONS(${GCC_COVERAGE_COMPILE_FLAGS})

ENDIF()

if((CMAKE_BUILD_TYPE MATCHES Release) OR (CMAKE_BUILD_TYPE MATCHES RELEASE))
  message(STATUS "ENABLING RELEASE on ${PROJECT_NAME}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
ENDIF()

if((CMAKE_BUILD_TYPE MATCHES RelWithDebInfo) OR (CMAKE_BUILD_TYPE MATCHES RELWITHDEBINFO))
message(STATUS "ENABLING RELWITHDEBINFO on ${PROJECT_NAME}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O3")
ENDIF()


if((CMAKE_BUILD_TYPE MATCHES MinSizeRel) OR (CMAKE_BUILD_TYPE MATCHES MINSIZEREL))
message(STATUS "ENABLING MINSIZEREL on ${PROJECT_NAME}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
ENDIF()

set(chaos_INCLUDE_DIRS ${PREFIX}/include)
FILE(GLOB boost_libs ${PREFIX}/lib/libboost*.a)

set(chaos_LIBRARIES chaos_cutoolkit chaos_common)
set(chaos_client_LIBRARIES chaos_metadata_service_client)
SET(CMAKE_INSTALL_RPATH "${PREFIX}/lib")
link_directories(${PREFIX}/lib ${PREFIX}/lib64)
include_directories(${chaos_INCLUDE_DIRS})
SET(FrameworkLib ${chaos_LIBRARIES} ${boost_libs} pthread dl ${GCC_COVERAGE_LINK_LIB})
IF (WIN32)
include(${PREFIX}/CMakeMacroUtilsWin.txt)
ELSE()
include(${PREFIX}/CMakeMacroUtils.txt)
ENDIF(WIN32)
IF(CHAOS_STATIC)
  message(STATUS "Enabling Static compilation")
  SET(BUILD_FORCE_STATIC 1)
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  ADD_DEFINITIONS(-DCHAOS_STATIC)
  SET(CHAOS_LINKER_FLAGS "-static")
  SET(CMAKE_EXE_LINKER_FLAGS "-static")
  SET(BUILD_SHARED_LIBRARIES OFF)
  SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
  SET(CMAKE_SHARED_LIBRARY_LINK_CC_FLAGS)
  set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic
  set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
#    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_SEARCH_END_STATIC 1)
ELSE()
  SET(BUILD_SHARED_LIBRARIES ON)
ENDIF()

#if(CMAKE_BUILD_TYPE MATCHES PROFILE)
#   MESSAGE( STATUS "ENABLING PROFILE on ${PROJECT_NAME}")
#   ADD_DEFINITIONS(-g -fprofile-arcs -ftest-coverage)
#   set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
# ENDIF()

IF ( CHAOS_SANITIZER)
  IF(DEFINED PROJECT_NAME)
      SET(FrameworkLib asan ${FrameworkLib})

    IF( (${CHAOS_SANITIZER} STREQUAL ${PROJECT_NAME}) OR (${CHAOS_SANITIZER} STREQUAL "ALL"))
      message(STATUS "ENABLING SANITIZER FOR PROJECT ${PROJECT_NAME}")
      ADD_DEFINITIONS(-fsanitize=address)
    

      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    ENDIF()

  ENDIF()
ENDIF()
IF(CHAOS_PROMETHEUS)
  message(STATUS "ENABLING Prometheus on ${PROJECT_NAME}")
  SET(FrameworkLib ${FrameworkLib} prometheus-cpp-core prometheus-cpp-pull)
  ADD_DEFINITIONS(-DCHAOS_PROMETHEUS)
ENDIF(CHAOS_PROMETHEUS)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
