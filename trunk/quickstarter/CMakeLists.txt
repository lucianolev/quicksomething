# Project Needs a name ofcourse
project(quickstarter)
 
# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
find_package(Plasma REQUIRED)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )
 
# We add our source code here
set(quickstarter_SRCS quickstarter.cpp popupdialog.cpp resizedialog.cpp applicationmodel.cpp itemview.cpp
		      settings.cpp)
 
# Now make sure all files get to the right place
kde4_add_ui_files(quickstarter_SRCS quickstarterConfig.ui)
kde4_add_plugin(plasma_applet_quickstarter ${quickstarter_SRCS})
target_link_libraries(plasma_applet_quickstarter 
                      ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS} kfile)
 
install(TARGETS plasma_applet_quickstarter
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-quickstarter.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})