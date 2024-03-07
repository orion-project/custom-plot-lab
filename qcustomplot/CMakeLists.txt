set(QCP_NAME qcustomplot)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QCP_SOURCES
    qcustomplot.cpp
)

add_library(${QCP_NAME} STATIC
    ${QCP_SOURCES}
)

target_link_libraries(${QCP_NAME} PRIVATE
    Qt::Widgets
    Qt::PrintSupport
)
