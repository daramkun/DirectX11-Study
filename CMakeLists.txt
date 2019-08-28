cmake_minimum_required (VERSION 3.10)

project (
	DirectX11-Study
	VERSION 1.0.0
	DESCRIPTION "Personal DirectX 11 Study Project."
	LANGUAGES C CXX
)

set (INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Binary directory")

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

set (LINKAGE_STATIC_LIBS DirectX11_Study_Framework d3d11.lib dxgi.lib d3dcompiler.lib windowscodecs.lib mf.lib mfplat.lib mfuuid.lib mfreadwrite.lib XInput.lib Xaudio2.lib Shlwapi.lib)
set (INCLUDE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/framework/include)

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

install (
	TARGETS
		"${DAY1_CH1_PROJECTNAME}"
		"${DAY1_CH2_PROJECTNAME}"
		"${DAY1_CH3_PROJECTNAME}"
		"${DAY1_CH4_PROJECTNAME}"

		"${DAY2_CH1_PROJECTNAME}"
		"${DAY2_CH2_PROJECTNAME}"
		"${DAY2_CH3_PROJECTNAME}"
		"${DAY2_CH4_PROJECTNAME}"
	RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)
install (
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_INSTALL_PREFIX}
)