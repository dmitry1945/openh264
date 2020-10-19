cmake_minimum_required ( VERSION 2.6 )
# Укажите здесь имя вашего проекта.
# Учитываете установку переменных 
# <project_name>_BINARY_DIR и  <project_name>_SOURCE_DIR
# в значение имени каталога с проектом.
project ( decH264TcpRx )


message(PROJECT_SOURCE_DIR - "${PROJECT_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 14)

set ( SRC_DIR ${PROJECT_SOURCE_DIR}/src )
set ( SLAM_SRC_DIR ${PROJECT_SOURCE_DIR}/../../SFM_SLAM/src)
set ( SLAM_INC_DIR ${PROJECT_SOURCE_DIR}/../../SFM_SLAM)
set ( NAV_SRC_DIR ${SLAM_INC_DIR}/navigation)

set ( TCP_SRC_DIR ${PROJECT_SOURCE_DIR}/../modules/tcp)
set ( TCP_INC_DIR ${PROJECT_SOURCE_DIR}/../modules/tcp)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set ( APP_NAME decH264TcpRx )
SET(SLAM_ROOT "~/work/slam/libslam")
SET(SLAM_THIRDPARTY "${SLAM_ROOT}/thirdparty")
SET(H264_ROOT "~/work/video/openh264")

message("==========>   SRC_DIR - ${SRC_DIR}")
message("==========>   SLAM_SRC_DIR - ${SLAM_SRC_DIR}")
message("==========>   SLAM_INC_DIR - ${SLAM_INC_DIR}")
message("==========>   SLAM_ROOT - ${SLAM_ROOT}")
message("==========>   SLAM_THIRDPARTY - ${SLAM_THIRDPARTY}")


LIST(APPEND CMAKE_MODULE_PATH
    "${SLAM_ROOT}/tools"
    "${SLAM_ROOT}/tools/share/cmake")

if (!CMAKE_BUILD_TYPE)
	set( CMAKE_BUILD_TYPE RELEASE )
endif()

message("Build type - ${CMAKE_BUILD_TYPE}")

# ______________________________________________________________________________
# Eigen3
SET(Eigen_DIR "${SLAM_THIRDPARTY}/eigen")
SET(Eigen_ROOT "${SLAM_THIRDPARTY}/eigen")
FIND_PACKAGE(Eigen REQUIRED)

# ______________________________________________________________________________
# g2o
SET(G2O_ROOT "${SLAM_THIRDPARTY}/g2o")
message("SLAM_THIRDPARTY - ${SLAM_THIRDPARTY}")
message("SLAM_ROOT - ${SLAM_ROOT}/tools")

FIND_PACKAGE(g2o REQUIRED)

find_package( OpenCV 4 REQUIRED )
find_library( OpenCV 4 REQUIRED )

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
	message(">------------   OPenMP found!  -------------------<")
endif()


include_directories (${PROJECT_SOURCE_DIR}/src)
include_directories (${SLAM_INC_DIR})
include_directories (${SLAM_INC_DIR}/include)
include_directories (${SLAM_SRC_DIR}/sfm_core)
include_directories (${SLAM_SRC_DIR}/Emulator3D)
include_directories (${NAV_SRC_DIR}/mat)
include_directories (${NAV_SRC_DIR}/ahrs)
include_directories (${NAV_SRC_DIR}/file_nav)
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${SLAM_THIRDPARTY}/dbow3/install/include )
include_directories( ${SLAM_THIRDPARTY}/g2o)
include_directories( ${SLAM_THIRDPARTY}/eigen)
include_directories( ${SLAM_THIRDPARTY}/eigen/Eigen)

include_directories( ${SLAM_THIRDPARTY}/eigen/Eigen)

#include H264:
include_directories( ${H264_ROOT}/codec/common/inc )
include_directories( ${H264_ROOT}/codec/processing/interface)
include_directories( ${H264_ROOT}/codec/console/common/inc)
include_directories( ${H264_ROOT}/codec/api/svc )
include_directories( ${H264_ROOT}/codec/encoder/core/inc )
include_directories( ${H264_ROOT}/codec/decoder/core/inc )

# TCP
include_directories (${TCP_INC_DIR})



message("==========>   SLAM_THIRDPARTY - ${SLAM_THIRDPARTY}")
message("==========>   OpenCV_INCLUDE_DIRS - ${OpenCV_INCLUDE_DIRS}")

LINK_DIRECTORIES("${SLAM_ROOT}/thirdparty/dbow3/install/lib")
SET(DBoW3_LIB_DIR "${SLAM_ROOT}//thirdparty/dbow3/install/lib")
LINK_DIRECTORIES("${SLAM_ROOT}/thirdparty/g2o/build")
LINK_DIRECTORIES("${SLAM_ROOT}/thirdparty/g2o/bin")

list(APPEND SRC ${SRC_DIR}/decH264TcpRx_main.cpp
				${TCP_SRC_DIR}/rxtx_packet.cpp
				${TCP_SRC_DIR}/TCP_DataRecv.cpp
				${TCP_SRC_DIR}/TCPServer.cpp
				${TCP_SRC_DIR}/TCPClient.cpp
)
add_executable ( ${APP_NAME} ${SRC} )

message(OpenCV_LIBS - "${OpenCV_LIBS}")

target_link_libraries( ${APP_NAME}
#				rockchip_mpp
#				utils				
				v4l2 
				jpeg
#				slam
				pthread
				DBoW3
				g2o_core
				g2o_stuff
				g2o_types_sim3
				g2o_types_sba
				openh264
				${OpenCV_LIBS}
)
