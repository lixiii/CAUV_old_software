project (gui)

set ( FLUIDITY_PLUGIN_SRCS
    plugin.cpp
    fluiditynode.cpp
    messaging.cpp
)

set ( FLUIDITY_PLUGIN_UIS
)

set ( FLUIDITY_PLUGIN_H
    plugin.h
    fluiditynode.h
    messaging.h
)

     
qt4_wrap_ui (FLUIDITY_PLUGIN_UIS_H ${FLUIDITY_PLUGIN_UIS})
qt4_wrap_cpp (FLUIDITY_PLUGIN_MOC_H ${FLUIDITY_PLUGIN_H})

add_library( fluidityplugin MODULE
    ${FLUIDITY_PLUGIN_SRCS}
    ${FLUIDITY_PLUGIN_UIS_H}
    ${FLUIDITY_PLUGIN_MOC_H}
)
depends_on_messages(fluidityplugin)
cauv_install_plugin(fluidityplugin)
add_precompiled_header(fluidityplugin ${guis_SOURCE_DIR}/pch.h)
                                      
target_link_libraries( fluidityplugin
    gui-core
    ${QT_LIBRARIES}
)