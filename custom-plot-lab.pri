INCLUDEPATH += $$PWD

macx: DEFINES += _LIBCPP_DISABLE_AVAILABILITY

QT += printsupport

SOURCES += \
    $$PWD/qcpl_axis.cpp \
    $$PWD/qcpl_axis_factor.cpp \
    $$PWD/qcpl_cursor.cpp \
    $$PWD/qcpl_cursor_panel.cpp \
    $$PWD/qcpl_export.cpp \
    $$PWD/qcpl_format_editors.cpp \
    $$PWD/qcpl_format_graph.cpp \
    $$PWD/qcpl_format_legend.cpp \
    $$PWD/qcpl_format_title.cpp \
    $$PWD/qcpl_io_json.cpp \
    $$PWD/qcpl_text_editor.cpp \
    $$PWD/qcpl_plot.cpp \
    $$PWD/qcpl_colors.cpp \
    $$PWD/qcpl_graph.cpp \
    $$PWD/qcpl_types.cpp \
    $$PWD/qcpl_graph_grid.cpp \
    $$PWD/qcpl_utils.cpp \
    $$PWD/qcpl_format.cpp \
    $$PWD/qcpl_format_axis.cpp \
    $$PWD/qcpl_format_plot.cpp

HEADERS  += \
    $$PWD/qcpl_axis.h \
    $$PWD/qcpl_axis_factor.h \
    $$PWD/qcpl_cursor.h \
    $$PWD/qcpl_cursor_panel.h \
    $$PWD/qcpl_export.h \
    $$PWD/qcpl_format_editors.h \
    $$PWD/qcpl_format_graph.h \
    $$PWD/qcpl_format_legend.h \
    $$PWD/qcpl_format_title.h \
    $$PWD/qcpl_io_json.h \
    $$PWD/qcpl_text_editor.h \
    $$PWD/qcpl_plot.h \
    $$PWD/qcpl_colors.h \
    $$PWD/qcpl_graph.h \
    $$PWD/qcpl_types.h \
    $$PWD/qcpl_graph_grid.h \
    $$PWD/qcpl_utils.h \
    $$PWD/qcpl_format.h \
    $$PWD/qcpl_format_axis.h \
    $$PWD/qcpl_format_plot.h

qcustomplotlab_shared {
    # Tell the qcustomplot header that it will be used as library:
    DEFINES += QCUSTOMPLOT_USE_LIBRARY
    win32:QCPLIB = qcustomplot2
    else: QCPLIB = qcustomplot
    LIBS += -L$$PWD/qcustomplot/release -l$$QCPLIB
} else {
    SOURCES += $$PWD/qcustomplot/qcustomplot.cpp
    HEADERS += $$PWD/qcustomplot/qcustomplot.h
}

RESOURCES += \
    $$PWD/qcpl_images.qrc
