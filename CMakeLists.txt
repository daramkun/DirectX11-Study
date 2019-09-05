cmake_minimum_required (VERSION 3.10)

project (
	DirectX11-Study
	VERSION 1.0.0
	DESCRIPTION "Personal DirectX 11 Study Project."
	LANGUAGES C CXX
)

set (INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Binary directory")

if (NOT MSVC)
	message (FATAL_ERROR "Target platform must Windows.")
endif ()

set (FRAMEWORK_FILES framework/src/framework.cpp)
add_library (
	DirectX11_Study_Framework
	STATIC
	${FRAMEWORK_FILES}
)
target_include_directories (
	DirectX11_Study_Framework
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/framework/include
)

############################################################
# Framework
############################################################
set (LINKAGE_STATIC_LIBS DirectX11_Study_Framework d3d11.lib dxgi.lib d3dcompiler.lib windowscodecs.lib mf.lib mfplat.lib mfuuid.lib mfreadwrite.lib XInput.lib Xaudio2.lib Shlwapi.lib)
set (INCLUDE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/framework/include)

############################################################
# Day 1
############################################################
set (DAY1_CH1_PROJECTNAME DirectX11_Study_Day1_Chapter1)
set (DAY1_CH1_FILES "src/common.manifest" "src/Day1/1. Create Device with Legacy/Main.cpp")
add_executable (${DAY1_CH1_PROJECTNAME} WIN32 ${DAY1_CH1_FILES})
target_include_directories (${DAY1_CH1_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY1_CH1_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

set (DAY1_CH2_PROJECTNAME DirectX11_Study_Day1_Chapter2)
set (DAY1_CH2_FILES "src/common.manifest" "src/Day1/2. Create Device with Brandnew/Main.cpp")
add_executable (${DAY1_CH2_PROJECTNAME} WIN32 ${DAY1_CH2_FILES})
target_include_directories (${DAY1_CH2_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY1_CH2_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

set (DAY1_CH3_PROJECTNAME DirectX11_Study_Day1_Chapter3)
set (DAY1_CH3_FILES "src/common.manifest" "src/Day1/3. Create Render Target View/Main.cpp")
add_executable (${DAY1_CH3_PROJECTNAME} WIN32 ${DAY1_CH3_FILES})
target_include_directories (${DAY1_CH3_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY1_CH3_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

set (DAY1_CH4_PROJECTNAME DirectX11_Study_Day1_Chapter4)
set (DAY1_CH4_FILES "src/common.manifest" "src/Day1/4. Rendering Triangle/Main.cpp")
add_executable (${DAY1_CH4_PROJECTNAME} WIN32 ${DAY1_CH4_FILES})
target_include_directories (${DAY1_CH4_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY1_CH4_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

############################################################
# Day 2
############################################################
set (DAY2_CH1_PROJECTNAME DirectX11_Study_Day2_Chapter1)
set (DAY2_CH1_FILES "src/common.manifest" "src/Day2/1. Colored Triangle/Main.cpp")
add_executable (${DAY2_CH1_PROJECTNAME} WIN32 ${DAY2_CH1_FILES})
target_include_directories (${DAY2_CH1_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY2_CH1_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

set (DAY2_CH2_PROJECTNAME DirectX11_Study_Day2_Chapter2)
set (DAY2_CH2_FILES "src/common.manifest" "src/Day2/2. Transform/Main.cpp")
add_executable (${DAY2_CH2_PROJECTNAME} WIN32 ${DAY2_CH2_FILES})
target_include_directories (${DAY2_CH2_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY2_CH2_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

set (DAY2_CH3_PROJECTNAME DirectX11_Study_Day2_Chapter3)
set (DAY2_CH3_FILES "src/common.manifest" "src/Day2/3. Box/Main.cpp")
add_executable (${DAY2_CH3_PROJECTNAME} WIN32 ${DAY2_CH3_FILES})
target_include_directories (${DAY2_CH3_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY2_CH3_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

set (DAY2_CH4_PROJECTNAME DirectX11_Study_Day2_Chapter4)
set (DAY2_CH4_FILES "src/common.manifest" "src/Day2/4. Depth Buffer/Main.cpp")
add_executable (${DAY2_CH4_PROJECTNAME} WIN32 ${DAY2_CH4_FILES})
target_include_directories (${DAY2_CH4_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY2_CH4_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

############################################################
# Day 3
############################################################
set (DAY3_CH1_PROJECTNAME DirectX11_Study_Day3_Chapter1)
set (DAY3_CH1_FILES "src/common.manifest" "src/Day3/1. Texturing/Main.cpp")
add_executable (${DAY3_CH1_PROJECTNAME} WIN32 ${DAY3_CH1_FILES})
target_include_directories (${DAY3_CH1_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY3_CH1_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

set (DAY3_CH2_PROJECTNAME DirectX11_Study_Day3_Chapter2)
set (DAY3_CH2_FILES "src/common.manifest" "src/Day3/2. Multi Texturing/Main.cpp")
add_executable (${DAY3_CH2_PROJECTNAME} WIN32 ${DAY3_CH2_FILES})
target_include_directories (${DAY3_CH2_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY3_CH2_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

set (DAY3_CH3_PROJECTNAME DirectX11_Study_Day3_Chapter3)
set (DAY3_CH3_FILES "src/common.manifest" "src/Day3/3. Draw Vertices without Vertex Buffer/Main.cpp")
add_executable (${DAY3_CH3_PROJECTNAME} WIN32 ${DAY3_CH3_FILES})
target_include_directories (${DAY3_CH3_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY3_CH3_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

############################################################
# Day 4
############################################################
set (DAY4_CH1_PROJECTNAME DirectX11_Study_Day4_Chapter1)
set (DAY4_CH1_FILES "src/common.manifest" "src/Day4/1. Blending/Main.cpp")
add_executable (${DAY4_CH1_PROJECTNAME} WIN32 ${DAY4_CH1_FILES})
target_include_directories (${DAY4_CH1_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY4_CH1_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

set (DAY4_CH2_PROJECTNAME DirectX11_Study_Day4_Chapter2)
set (DAY4_CH2_FILES "src/common.manifest" "src/Day4/2. Forward Lighting/Main.cpp")
add_executable (${DAY4_CH2_PROJECTNAME} WIN32 ${DAY4_CH2_FILES})
target_include_directories (${DAY4_CH2_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY4_CH2_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

############################################################
# Day 5
############################################################
set (DAY5_CH1_PROJECTNAME DirectX11_Study_Day5_Chapter1)
set (DAY5_CH1_FILES "src/common.manifest" "src/Day5/1. Mirror with Stencil/Main.cpp")
add_executable (${DAY5_CH1_PROJECTNAME} WIN32 ${DAY5_CH1_FILES})
target_include_directories (${DAY5_CH1_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY5_CH1_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

set (DAY5_CH2_PROJECTNAME DirectX11_Study_Day5_Chapter2)
set (DAY5_CH2_FILES "src/common.manifest" "src/Day5/2. Render Target/Main.cpp")
add_executable (${DAY5_CH2_PROJECTNAME} WIN32 ${DAY5_CH2_FILES})
target_include_directories (${DAY5_CH2_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY5_CH2_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

############################################################
# Day 6
############################################################
set (DAY6_CH1_PROJECTNAME DirectX11_Study_Day6_Chapter1)
set (DAY6_CH1_FILES "src/common.manifest" "src/Day6/1. Multi Render Target/Main.cpp")
add_executable (${DAY6_CH1_PROJECTNAME} WIN32 ${DAY6_CH1_FILES})
target_include_directories (${DAY6_CH1_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY6_CH1_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

set (DAY6_CH2_PROJECTNAME DirectX11_Study_Day6_Chapter2)
set (DAY6_CH2_FILES "src/common.manifest" "src/Day6/2. Multi Render Target 2/Main.cpp")
add_executable (${DAY6_CH2_PROJECTNAME} WIN32 ${DAY6_CH2_FILES})
target_include_directories (${DAY6_CH2_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY6_CH2_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

set (DAY6_CH3_PROJECTNAME DirectX11_Study_Day6_Chapter3)
set (DAY6_CH3_FILES "src/common.manifest" "src/Day6/3. Deferred Lighting/Main.cpp")
add_executable (${DAY6_CH3_PROJECTNAME} WIN32 ${DAY6_CH3_FILES})
target_include_directories (${DAY6_CH3_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY6_CH3_PROJECTNAME} ${LINKAGE_STATIC_LIBS})

############################################################
# Day 7
############################################################
set (DAY7_CH1_PROJECTNAME DirectX11_Study_Day7_Chapter1)
set (DAY7_CH1_FILES "src/common.manifest" "src/Day7/1. CPU Texture Blur/Main.cpp")
add_executable (${DAY7_CH1_PROJECTNAME} WIN32 ${DAY7_CH1_FILES})
target_include_directories (${DAY7_CH1_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY7_CH1_PROJECTNAME} ${LINKAGE_STATIC_LIBS})
target_compile_options (${DAY7_CH1_PROJECTNAME} PUBLIC "/openmp")

set (DAY7_CH2_PROJECTNAME DirectX11_Study_Day7_Chapter2)
set (DAY7_CH2_FILES "src/common.manifest" "src/Day7/2. DirectCompute Texture Blur/Main.cpp")
add_executable (${DAY7_CH2_PROJECTNAME} WIN32 ${DAY7_CH2_FILES})
target_include_directories (${DAY7_CH2_PROJECTNAME} PUBLIC ${INCLUDE_HEADERS})
target_link_libraries (${DAY7_CH2_PROJECTNAME} ${LINKAGE_STATIC_LIBS})