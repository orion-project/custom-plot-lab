#ifndef QCPL_FORMAT_PLOT_H
#define QCPL_FORMAT_PLOT_H

#include "qcpl_types.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

namespace QCPL {

class Plot;

class PlotFormatWidget : public QWidget
{
    Q_OBJECT

public:
    PlotFormatWidget(Plot* plot, const PlotFormatSettings& format);

public slots:
    void apply();

private:
    Plot *_plot;
    PlotFormatSettings _format;
};

} // namespace QCPL

#endif // QCPL_FORMAT_PLOT_H
