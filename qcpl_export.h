#ifndef QCPL_EXPORT_H
#define QCPL_EXPORT_H

#include <QVector>

namespace QCPL {

struct GraphDataExportSettings
{
    bool csv = false;
    bool systemLocale = false;
    bool transposed = false;
    int numberPrecision = 6;
    bool mergePoints = false;
};

class GraphDataExporter
{
public:
    GraphDataExporter(const GraphDataExportSettings& settings);
    ~GraphDataExporter();

    void add(const double& v);
    void add(const double& x, const double& y);
    void add(const QVector<double>& vs);

    void toClipboard();

private:
    class ExporterImpl* _impl;
    QString _prev, _prevX, _prevY;
    bool _merge;
};

} // namespace QCPL

#endif // QCPL_EXPORT_H
