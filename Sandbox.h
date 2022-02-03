#ifndef PLOT_WINDOW_H
#define PLOT_WINDOW_H

#include <QMainWindow>

#include "qcpl_plot.h"
#include "qcpl_colors.h"

class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    PlotWindow(QWidget *parent = nullptr);
    ~PlotWindow();

private:
    QCPL::Plot *_plot;

    void addRandomGraph();
};

#endif // PLOT_WINDOW_H
