#ifndef PLOT_WINDOW_H
#define PLOT_WINDOW_H

#include <QMainWindow>

#include "qcpl_plot.h"
#include "qcpl_utils.h"
#include "qcpl_format.h"

class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    PlotWindow(QWidget *parent = nullptr);
    ~PlotWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QCPL::Plot *_plot;

    void addRandomSample();
};

#endif // PLOT_WINDOW_H
