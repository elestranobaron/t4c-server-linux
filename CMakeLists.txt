cmake_minimum_required(VERSION 3.10)
project(T4CServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -Wno-overloaded-virtual -Wno-mismatched-new-delete -Wno-unused-but-set-variable -Wno-ignored-qualifiers -fcheck-new -fpermissive -DT4C_LINUX_BUILD")

find_library(MYSQL_LIBRARIES NAMES mariadb mysqlclient)
find_path(MYSQL_INCLUDE_DIR NAMES mysql.h mariadb/mysql.h
          HINTS /usr/include /usr/include/mysql /usr/include/mariadb)
if(MYSQL_LIBRARIES AND MYSQL_INCLUDE_DIR)
    include_directories(${MYSQL_INCLUDE_DIR})
endif()

find_package(SDL3 REQUIRED CONFIG)
find_package(OpenSSL REQUIRED)
find_package(ODBC REQUIRED)
find_package(Threads REQUIRED)

set(EXE_DIR "${CMAKE_SOURCE_DIR}/Exe Server")

include_directories(
    ${SDL3_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${ODBC_INCLUDE_DIRS}
    "${EXE_DIR}"
    "${EXE_DIR}/Wda"
    "${EXE_DIR}/Wda/Npc"
    "${EXE_DIR}/Skills"
    "${EXE_DIR}/SpellEffects"
    "${EXE_DIR}/TinyXML"
    "${EXE_DIR}/World Objects"
    "${EXE_DIR}/DevTools"
    "${EXE_DIR}/MD5"
    "${EXE_DIR}/NMWda"
    "${EXE_DIR}/Zlib"
    "${EXE_DIR}/crypt"
    ${CMAKE_SOURCE_DIR}/Crypto
    "${CMAKE_SOURCE_DIR}/Dll Gameops"
    "${CMAKE_SOURCE_DIR}/Dll Npcs"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Addon"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Arakas"
    "${CMAKE_SOURCE_DIR}/Dll Npcs RavensDust"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Remort"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Stoneheim"
    "${CMAKE_SOURCE_DIR}/Dll Npcs WindHowl"
)

file(GLOB_RECURSE SOURCES
    "${EXE_DIR}/*.cpp"
    "${EXE_DIR}/*.CPP"
    "${CMAKE_SOURCE_DIR}/Crypto/*.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Gameops/*.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Gameops/*.CPP"
    "${CMAKE_SOURCE_DIR}/Dll Npcs/*.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs/*.CPP"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Addon/*.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Addon/*.CPP"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Arakas/*.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Arakas/*.CPP"
    "${CMAKE_SOURCE_DIR}/Dll Npcs RavensDust/*.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs RavensDust/*.CPP"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Remort/*.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Remort/*.CPP"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Stoneheim/*.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Stoneheim/*.CPP"
    "${CMAKE_SOURCE_DIR}/Dll Npcs WindHowl/*.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs WindHowl/*.CPP"
)

list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/build/.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/Release/.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/Debug/.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/System-(Win32|Linux)\\.[cC][pP][pP]$")
list(FILTER SOURCES EXCLUDE REGEX ".*/T4CLogProcess\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/DevTools/bsccLogger\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/WebServer/.*\\.[cC][pP][pP]$")
list(FILTER SOURCES EXCLUDE REGEX ".*/CommCenter\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/PacketManager_linux\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/EXPFLTR\\.CPP$")
list(FILTER SOURCES EXCLUDE REGEX ".*/C_SERVIC\\.CPP$")
list(FILTER SOURCES EXCLUDE REGEX ".*/C_Servic\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/MonsterStatSetup\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX "NPC Thread__eeee\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX "NPC Thread_AvantFix\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/sysop-backup\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/StdAfx\\.cpp$")

set(_mss_paths
    "${CMAKE_SOURCE_DIR}/Dll Gameops/MonsterStatSetup.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs/MonsterStatSetup.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Addon/MonsterStatSetup.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Arakas/MonsterStatSetup.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs RavensDust/MonsterStatSetup.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Remort/MonsterStatSetup.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs Stoneheim/MonsterStatSetup.cpp"
    "${CMAKE_SOURCE_DIR}/Dll Npcs WindHowl/MonsterStatSetup.cpp"
)
set(_mss_syms GameOps NPCs NPCAddOn Arakas RavensDust Remort Stoneheim WindHowl)
foreach(_mss_path IN LISTS _mss_paths)
    list(APPEND SOURCES "${_mss_path}")
endforeach()
list(APPEND SOURCES "${EXE_DIR}/MonsterStatSetup_dispatch.cpp")
list(APPEND SOURCES "${EXE_DIR}/EXPFLTR_linux.cpp")

add_executable(T4CServer ${SOURCES})
target_compile_definitions(T4CServer PRIVATE TIXML_USE_STL NO_DAO_SUPPORT T4C_LINUX_BUILD)

set(T4C_NPC_REGION_DIRS "Dll Gameops" "Dll Npcs" "Dll Npcs Addon" "Dll Npcs Arakas" "Dll Npcs RavensDust" "Dll Npcs Remort" "Dll Npcs Stoneheim" "Dll Npcs WindHowl")
set(T4C_NPC_REGION_SYMS GameOps NPCs NPCAddOn Arakas RavensDust Remort Stoneheim WindHowl)
set(_ri 0)
foreach(_dir IN LISTS T4C_NPC_REGION_DIRS)
    list(GET T4C_NPC_REGION_SYMS ${_ri} _sym)
    file(GLOB _reg_src "${CMAKE_SOURCE_DIR}/${_dir}/*.cpp" "${CMAKE_SOURCE_DIR}/${_dir}/*.CPP")
    set(_rh "${EXE_DIR}/linux_npc_region/${_sym}_rename.h")
    set_source_files_properties(${_reg_src} PROPERTIES
        COMPILE_DEFINITIONS "T4C_CREATE_NPC_FUNC_NAME=CreateNPC_${_sym}"
        COMPILE_OPTIONS "-include;${_rh}")
    math(EXPR _ri "${_ri}+1")
endforeach()
set(_mss_idx 0)
foreach(_mss_path IN LISTS _mss_paths)
    list(GET _mss_syms ${_mss_idx} _mss_sym)
    set_property(SOURCE "${_mss_path}" APPEND PROPERTY COMPILE_DEFINITIONS
        "T4C_MONSTER_STAT_SETUP_FUNC_NAME=MonsterStatSetup_${_mss_sym}")
    math(EXPR _mss_idx "${_mss_idx}+1")
endforeach()

find_package(ZLIB)
target_link_libraries(T4CServer PRIVATE SDL3::SDL3 OpenSSL::SSL OpenSSL::Crypto ODBC::ODBC Threads::Threads)
if(ZLIB_FOUND)
    target_link_libraries(T4CServer PRIVATE ZLIB::ZLIB)
else()
    target_link_libraries(T4CServer PRIVATE z)
endif()
if(MYSQL_LIBRARIES)
    target_link_libraries(T4CServer PRIVATE ${MYSQL_LIBRARIES})
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(T4CServer PRIVATE dl)
endif()

set(CMAKE_BUILD_TYPE Debug)
