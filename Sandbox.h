#ifndef PLOT_WINDOW_H
#define PLOT_WINDOW_H

#include <QMainWindow>

#include "qcpl_plot.h"

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
    QString recentFormatFile;

    void addRandomSample();
    void savePlotFormat();
    void loadPlotFormat();
    void loadDefaultFormat();
    void pasteLegendFormat();
    void pasteTitleFormat();
};

#endif // PLOT_WINDOW_H
