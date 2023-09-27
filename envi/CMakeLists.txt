cmake_minimum_required(VERSION 3.24)

include(CMakeDependentOption)
include(FetchContent)
include(ExternalProject)

# proj

set(vProjName "SnakIter")
set(vProjVnum "0.1")
set(vProjDesc "Snake-Eater")
set(vProjPath "${CMAKE_SOURCE_DIR}/..")

project(${vProjName} VERSION ${vProjVnum} DESCRIPTION ${vProjDesc})

# lang

enable_language(CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# code

set(vCodePath "${vProjPath}/code")
set(vCodeFile "${vCodePath}/${vProjName}.cpp")

# head

set(vHeadPath "${vProjPath}/code")
set(vHeadFile "${vHeadPath}/${vProjName}.hpp")

# data

set(vDataPath "${vProjPath}/data")

# defs

set(vDefsName "d${vProjName}_")

# make

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "type of build" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)

set(vMakePath "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${vMakePath}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${vMakePath}")

##lib

set(vTlibName "t${vProjName}Lib")

add_library(${vTlibName} INTERFACE)
target_compile_definitions(${vTlibName} INTERFACE ${vDefsName}MakeTlib
  ${vDefsName}ProjName="${vProjName}"
  ${vDefsName}ProjVnum="${vProjVnum}"
  ${vDefsName}ProjDesc="${vProjDesc}"
  ${vDefsName}ProjPath="${vProjPath}"
  ${vDefsName}DataPath="${vDataPath}"
  ) # target_compile_definitions

target_precompile_headers(${vTlibName} INTERFACE ${vHeadFile})

FetchContent_Declare(fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
  GIT_TAG "10.0.0"
  OVERRIDE_FIND_PACKAGE)
FetchContent_MakeAvailable(fmt)
find_package(fmt 10.0.0 REQUIRED)
target_link_libraries(${vTlibName} INTERFACE fmt::fmt)

##exe

set(vTexeName "t${vProjName}Exe")
set(vTexePath "${vMakePath}")
set(vTexeFile ${vTexePath}/${vTexeName})

option(oMakeTexe "build the project executable" TRUE)
if(oMakeTexe)

  add_executable(${vTexeName})
  target_compile_definitions(${vTexeName} PRIVATE ${vDefsName}MakeTexe)
  target_link_libraries(${vTexeName} PRIVATE ${vTlibName})

  target_sources(${vTexeName} PRIVATE ${vCodeFile})

endif(oMakeTexe)

# test

cmake_dependent_option(oMakeTest "setup and build ctest units" TRUE oMakeTexe FALSE)
if(oMakeTest)

  include(CTest)
  enable_testing()

  target_compile_definitions(${vTexeName} PRIVATE ${vDefsName}MakeTest)

  function(fTest vName vText)
    add_test(${vName} COMMAND ${vTexeFile} "test" "${vName}")
    if(vText)
      set_tests_properties(${vName} PROPERTIES PASS_REGULAR_EXPRESSION ${vText})
    endif(vText)
  endfunction(fTest)

  set(vTestText "")
  list(APPEND vTestText "HelloWorld")
  fTest("Hello" ${vTestText})

endif(oMakeTest)
