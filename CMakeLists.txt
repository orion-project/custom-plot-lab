set(QCPL_NAME custom-plot-lab)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QCPL_SOURCES
    qcpl_axis.cpp
    qcpl_axis_factor.cpp
    qcpl_cursor.cpp
    qcpl_cursor_panel.cpp
    qcpl_export.cpp
    qcpl_format_editors.cpp
    qcpl_format_graph.cpp
    qcpl_format_legend.cpp
    qcpl_format_title.cpp
    qcpl_io_json.cpp
    qcpl_text_editor.cpp
    qcpl_plot.cpp
    qcpl_colors.cpp
    qcpl_graph.cpp
    qcpl_types.cpp
    qcpl_graph_grid.cpp
    qcpl_utils.cpp
    qcpl_format.cpp
    qcpl_format_axis.cpp
    qcpl_format_plot.cpp
)

add_library(${QCPL_NAME} STATIC
    qcpl_images.qrc
    ${QCPL_SOURCES}
)

add_subdirectory(qcustomplot)

target_link_libraries(${QCPL_NAME} PRIVATE
    orion
    qcustomplot
    Qt::Widgets
)

target_include_directories(${QCPL_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(${QCPL_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
