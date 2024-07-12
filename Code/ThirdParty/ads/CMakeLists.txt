ns_cmake_init()

ns_requires(NS_3RDPARTY_ADS_SUPPORT)

ns_requires_qt()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ns_create_target(LIBRARY ${PROJECT_NAME} NO_NS_PREFIX)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

ns_link_target_qt(TARGET ${PROJECT_NAME} COMPONENTS Core Gui Widgets Svg)

if (NS_COMPILE_ENGINE_AS_DLL)
  target_compile_definitions(${PROJECT_NAME} PRIVATE ADS_SHARED_EXPORT)
else()
  target_compile_definitions(${PROJECT_NAME} PUBLIC ADS_STATIC)
endif()

set (MOC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/linux/FloatingWidgetTitleBar.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DockAreaTabBar.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DockAreaTitleBar.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DockAreaTitleBar_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DockAreaWidget.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DockContainerWidget.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DockFocusController.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DockManager.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DockOverlay.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DockSplitter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DockWidget.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DockWidgetTab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ElidingLabel.h
  ${CMAKE_CURRENT_SOURCE_DIR}/FloatingDockContainer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/FloatingDragPreview.h
  )
ns_qt_wrap_target_moc_files(${PROJECT_NAME} "${MOC_FILES}")

if(NS_CMAKE_COMPILER_MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /wd4458 /wd4456 /wd4702)
endif()

if(NS_CMAKE_PLATFORM_LINUX)
  target_link_libraries(${PROJECT_NAME} PRIVATE xcb Qt6::GuiPrivate)
  target_compile_definitions(${PROJECT_NAME} PUBLIC Q_OS_LINUX)
endif()
