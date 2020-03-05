INCLUDEPATH += $$PWD

QT += printsupport

SOURCES += \
    $$PWD/qcpl_cursor.cpp \
    $$PWD/qcpl_cursor_panel.cpp \
    $$PWD/qcustomplot/qcustomplot.cpp \
    $$PWD/qcpl_plot.cpp \
    $$PWD/qcpl_colors.cpp \
    $$PWD/qcpl_graph.cpp \
    $$PWD/qcpl_types.cpp \
    $$PWD/qcpl_graph_grid.cpp

HEADERS  += \
    $$PWD/qcpl_cursor.h \
    $$PWD/qcpl_cursor_panel.h \
    $$PWD/qcustomplot/qcustomplot.h \
    $$PWD/qcpl_plot.h \
    $$PWD/qcpl_colors.h \
    $$PWD/qcpl_graph.h \
    $$PWD/qcpl_types.h \
    $$PWD/qcpl_graph_grid.h

RESOURCES += \
    $$PWD/qcpl_images.qrc
