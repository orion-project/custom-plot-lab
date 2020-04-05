#ifndef QCPL_EXPORT_H
#define QCPL_EXPORT_H

#include <QString>

QT_BEGIN_NAMESPACE
class QTextStream;
QT_END_NAMESPACE

namespace QCPL {

struct GraphDataExportSettings
{
    bool csv = false;
    bool systemLocale = false;
    int numberPrecision = 6;
};

class GraphDataExporter
{
public:
    GraphDataExporter(const GraphDataExportSettings& settings);
    ~GraphDataExporter();

    void add(double v);
    void add(double x, double y);

    void toClipboard();

private:
    QString _result;
    QTextStream* _stream;
    bool _quote, _csv;
};

} // namespace QCPL

#endif // QCPL_EXPORT_H
