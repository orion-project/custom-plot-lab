INCLUDEPATH += $$PWD

QT += printsupport

SOURCES += \
    $$PWD/qcpl_cursor.cpp \
    $$PWD/qcpl_cursor_panel.cpp \
    $$PWD/qcpl_export.cpp \
    $$PWD/qcustomplot/qcustomplot.cpp \
    $$PWD/qcpl_plot.cpp \
    $$PWD/qcpl_colors.cpp \
    $$PWD/qcpl_graph.cpp \
    $$PWD/qcpl_types.cpp \
    $$PWD/qcpl_graph_grid.cpp \
    $$PWD/qcpl_utils.cpp \
    $$PWD/qcpl_format.cpp \
    $$PWD/qcpl_title_editor.cpp

HEADERS  += \
    $$PWD/qcpl_cursor.h \
    $$PWD/qcpl_cursor_panel.h \
    $$PWD/qcpl_export.h \
    $$PWD/qcustomplot/qcustomplot.h \
    $$PWD/qcpl_plot.h \
    $$PWD/qcpl_colors.h \
    $$PWD/qcpl_graph.h \
    $$PWD/qcpl_types.h \
    $$PWD/qcpl_graph_grid.h \
    $$PWD/qcpl_utils.h \
    $$PWD/qcpl_format.h \
    $$PWD/qcpl_title_editor.h

RESOURCES += \
    $$PWD/qcpl_images.qrc
