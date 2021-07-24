project(RapidOCR)
cmake_minimum_required(VERSION 3.16)


include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/prepro)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ocr)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)

if(WIN32)
	
	if( DEFINED SUPPORT_DML)
		 message("defined DML - " ${SUPPORT_DML})
		add_definitions(-DUSE_DML)
		link_libraries(directml.lib d3d12.lib DXGI )
	endif()
	

	if( DEFINED SUPPORT_CUDA)
		 message("defined DML - " ${SUPPORT_CUDA})
		add_definitions(-DUSE_CUDA)
		
	endif()


	IF (CMAKE_BUILD_TYPE MATCHES "Release")
		
		set(OpenCV_STATIC On)

		if(CMAKE_CL_64)
			set(OpenCV_DIR E:/thirdpart/opencv/windows-x64) 
		else()
			set(OpenCV_DIR E:/thirdpart/opencv/windows-x86) 
		endif()
	
		set(SDK_ROOT E:/thirdpart/lib4win/)
	else()

		set(SDK_ROOT E:/thirdpart/lib4win/)
		if(CMAKE_CL_64)
		set(OpenCV_DIR E:/thirdpart/opencv/build) 
		else()
		set(OpenCV_STATIC On)
		set(OpenCV_DIR E:/thirdpart/opencv/windows-x86) 
		endif()
		message(Debug Mode)

	endif()

	     
	
    set(INC_ROOT ${SDK_ROOT}/include)
	set(LIB_ROOT ${SDK_ROOT}/lib)
	set(RES_SRC sources/RpUniOCR.rc)
	if(CMAKE_CL_64)
		link_directories(${LIB_ROOT}/x64)
		message( Library x64)
	else()
		link_directories(${LIB_ROOT})
	endif()



	if(DEFINED DEBUG_MODE)
		
		STRING(REPLACE "-O2" "-Od" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})

	endif()
elseif(UNIX)


	set(OpenCV_STATIC ON)
	set(RES_SRC )
	set(SDK_ROOT /opt/rpcds/)
	set(INC_ROOT ${SDK_ROOT}/include)
	
	if(EXISTS ${SDK_ROOT}/lib/cmake/opencv4/)
	   	set(OpenCV_DIR ${SDK_ROOT}/lib/cmake/opencv4/) 
	else()
	  	set(OpenCV_DIR ${SDK_ROOT}/lib64/cmake/opencv4/) 
	endif()

	set(LIB_ROOT ${SDK_ROOT}/lib)
	
endif()

include_directories(${INC_ROOT})

include_directories(${INC_ROOT}/onnxruntime)

link_directories(${LIB_ROOT})

FIND_PACKAGE(OpenCV REQUIRED)

if(OPENCV_FOUND)
	message("OpenCV FOUND")
	include_directories(${OpenCV_INCLUDE_DIRS})
	message(${OpenCV_INCLUDE_DIRS})
	link_directories(${OpenCV_LIBRARY_DIRS})
	link_libraries(${OpenCV_LIBS})
		
endif()



set(PREPOSTPRO  prepro/postprocess_op.cpp prepro/preprocess_op.cpp  prepro/clipper.cpp prepro/utility.cpp)

SET(OCRSRC ocr/rpocrdet.cpp ocr/rpocrrec.cpp ocr/rpuniocrobj.cpp  ocr/rpocrconst.cpp)
set(LIBSRC    "sources/rpuniocr_api.cpp"   ${PREPOSTPRO}  ${OCRSRC})

add_library(RpUniOCR SHARED ${LIBSRC}  ${RES_SRC})

target_link_libraries(RpUniOCR  INTERFACE freeimage onnxruntime rpimgutil sheenbidi)

IF(WIN32)

target_compile_definitions(RpUniOCR PRIVATE  _RPUNIOCR_EXPORT_LIB=1)
target_compile_definitions(RpUniOCR PRIVATE  CV_CXX11=1)


target_precompile_headers(RpUniOCR PRIVATE include/precomp.h)

set_target_properties(RpUniOCR PROPERTIES 
    DISABLE_PRECOMPILE_HEADERS OFF
)

endif()

add_executable(rpuniocr_test sources/uniocr_test.cpp  common/getopt.cpp )

target_link_libraries(rpuniocr_test RpUniOCR sheenbidi)


if(WIN32)
	
	#set(CMAKE_INSTALL_DIR ${SDK_ROOT})
	if(CMAKE_CL_64)
		set(LIBDIR ${CMAKE_INSTALL_DIR}/lib/x64)
		set(BINDIR ${CMAKE_INSTALL_DIR}/bin/x64)
	else()
		set(LIBDIR ${CMAKE_INSTALL_DIR}/lib/)
		set(BINDIR ${CMAKE_INSTALL_DIR}/bin/)
	endif()


else()
set(CMAKE_INSTALL_DIR /opt/rpcds/)
set(LIBDIR ${CMAKE_INSTALL_DIR}/lib)
set(BINDIR ${CMAKE_INSTALL_DIR}/bin)
endif()

#set(CMAKE_DEBUG_POSTFIX "_d")

#set_target_properties(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "_d")
# install files.
INSTALL(TARGETS RpUniOCR
       DESTINATION  ${LIBDIR}
      
)

INSTALL(TARGETS rpuniocr_test
       DESTINATION  ${BINDIR}
      
)

install(FILES include/rpuniocr_api.h
		DESTINATION ${CMAKE_INSTALL_DIR}/include
)