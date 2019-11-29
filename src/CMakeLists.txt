set(KDE_SKIP_TEST_SETTINGS ON)


# make sure QtMain is linked
cmake_policy(SET CMP0020 NEW)

# Check that the dependencies dirs exist
if (NOT APPLE) # apple uses brew
	if ("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
		if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/kf5-debug")
			message(WARNING "Run setup.sh first if you wish to use the bundled kf5; trying to find system KF5")
		endif()
	else()
		if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/kf5-release")
			message(WARNING "Run setup.sh first if you wish to use the bundled kf5; trying to find system KF5")
		endif()
	endif()
endif()


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# make sure they have pulled the submodules
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/nodeeditor/CMakeLists.txt")
	message(FATAL_ERROR "Pull the submodules first, run `git submodule update --init`")
endif()

# use the downloaded KF5 libraries
if ("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/kf5-debug)
else()
	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/kf5-release)
endif()


set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)


find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(FeatureSummary)

# find qt
find_package(Qt5 5.6.0 REQUIRED COMPONENTS Gui Script Xml Core Widgets Network PrintSupport Svg)
find_package(Qt5 5.6.0 COMPONENTS DBus)

find_package(KF5 REQUIRED COMPONENTS 
	Attica Archive TextEditor JobWidgets DBusAddons KIO Crash Sonnet SyntaxHighlighting 
	Parts GuiAddons ItemViews Config ConfigWidgets Auth Codecs Completion GlobalAccel
	Service WindowSystem CoreAddons I18n XmlGui WidgetsAddons TextWidgets IconThemes)

set(CMAKE_CXX_STANDARD 17)

if (CG_BUILD_DEBUGGER)
	add_subdirectory(debugger)
endif()

set(GUI_SRCS
	main.cpp
	mainwindow.cpp
	chigraphnodemodel.cpp
	functionview.cpp
	functiondetails.cpp
	paramlistwidget.cpp
	subprocessoutputview.cpp
	modulebrowser.cpp
	execparamlistwidget.cpp
	thememanager.cpp
	localvariables.cpp
	functioninouts.cpp
	typeselector.cpp
	centraltabview.cpp
	launchconfigurationmanager.cpp
	launchconfigurationdialog.cpp
	structedit.cpp
	moduletreemodel.cpp
	chiitemselectiondialog.cpp
	chiitemselectwidget.cpp
	newmoduledialog.cpp
	modulepropertiesdialog.cpp
)

set(GUI_RESOURCES
	chigraphgui.qrc
)

set(GUI_HEADERS
	mainwindow.hpp
	chigraphnodemodel.hpp
	functionview.hpp
	subprocessoutputview.hpp
	paramlistwidget.hpp
	functiondetails.hpp
	modulebrowser.hpp
	execparamlistwidget.hpp
	thememanager.hpp
	localvariables.hpp
	functioninouts.hpp
	typeselector.hpp
	centraltabview.hpp
	launchconfigurationmanager.hpp
	launchconfigurationdialog.hpp
	structedit.hpp
	moduletreemodel.hpp
	chiitemselectiondialog.hpp
	chiitemselectwidget.hpp
	newmoduledialog.hpp
	modulepropertiesdialog.hpp
)

add_executable(chigraphgui ${GUI_SRCS} ${GUI_HEADERS} ${GUI_RESOURCES})

# for some reason cmake doesn't pick up on me wanting to do this...idk
set_target_properties(chigraphgui PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


target_link_libraries(chigraphgui
	Qt5::Widgets 
	KF5::Completion 
	KF5::TextEditor 
	KF5::Crash 
	KF5::Service 
	KF5::CoreAddons 
	KF5::I18n 
	KF5::Auth 
	KF5::XmlGui 
	KF5::WidgetsAddons 
	KF5::TextWidgets 
	KF5::IconThemes 
	nodes
	chigraphcore
)

if (CG_BUILD_DEBUGGER)
	target_link_libraries(chigraphgui chigraphguidebugger)
endif()

if(UNIX)
	target_compile_options(chigraphgui PRIVATE "-fexceptions")
endif()

# copy dlls
if(WIN32)
	add_custom_target(copydlls ALL)
	foreach(QTLIB
		Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Script
		Qt5::Network Qt5::DBus Qt5::Svg Qt5::PrintSupport Qt5::Xml
		KF5::Attica KF5::Crash KF5::Parts KF5::KIOCore KF5::KIOWidgets
		KF5::JobWidgets KF5::TextEditor KF5::SyntaxHighlighting
		KF5::DBusAddons KF5::Archive KF5::ConfigWidgets
		KF5::GuiAddons KF5::ItemViews KF5::Codecs KF5::Auth
		KF5::Completion KF5::SonnetCore KF5::SonnetUi
		KF5::Service KF5::ConfigGui KF5::WindowSystem
		KF5::GlobalAccel KF5::CoreAddons KF5::I18n KF5::XmlGui
		KF5::WidgetsAddons KF5::TextWidgets KF5::IconThemes KF5::AuthCore
	)

		add_custom_command(
		TARGET copydlls POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:${QTLIB}>
			$<TARGET_FILE_DIR:chigraphgui>
		COMMENT "Copying ${QTLIB} from $<TARGET_FILE:${QTLIB}> to $<TARGET_FILE_DIR:chigraphgui>"
		)
		
		install(FILES $<TARGET_FILE:${QTLIB}> DESTINATION bin)
	
	endforeach()
	
	find_package(Intl REQUIRED)
	get_filename_component(INTL_DIR ${Intl_LIBRARY} DIRECTORY)
	
	find_file(INTL_DLL "intl.dll" HINTS "${INTL_DIR}/../bin") # "$<TARGET_FILE_DIR:Qt5::Widgets>" "$<TARGET_FILE_DIR:KF5::Attica>")

	# copy libintl.dll
	add_custom_command(
	TARGET copydlls POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${INTL_DLL}
		$<TARGET_FILE_DIR:chigraphgui>
	COMMENT "Copying libitnl from ${INTL_DLL} to $<TARGET_FILE_DIR:chigraphgui>"
	)
	
	# copy clang
	find_program(CLANG_EXE "clang")
	add_custom_command(
	TARGET copydlls POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CLANG_EXE}
		$<TARGET_FILE_DIR:chigraphgui>
	COMMENT "Copying clang from ${CLANG_EXE} to $<TARGET_FILE_DIR:chigraphgui>"
	)

endif()

install(TARGETS chigraphgui DESTINATION bin)

# install desktop file and icons
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../org.chigraph.chigraphgui.desktop DESTINATION share/applications)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/org.chigraph.chigraphgui.appdata.xml DESTINATION share/metadata)

foreach(size 16 32 64 128 256 512 1024)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../icons/${size}x${size}/org.chigraph.chigraphgui.png DESTINATION share/icons/hicolor/${size}x${size}/apps)
endforeach()
