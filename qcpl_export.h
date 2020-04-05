#ifndef QCPL_EXPORT_H
#define QCPL_EXPORT_H

#include <QVector>

QT_BEGIN_NAMESPACE
class QTextStream;
QT_END_NAMESPACE

namespace QCPL {

class ValueFormatter;

struct GraphDataExportSettings
{
    bool csv = false;
    const ValueFormatter *formatter = nullptr;
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
    GraphDataExportSettings _settings;
    QString _result;
    QTextStream* _stream;
    const ValueFormatter *_formatter;
    bool _quote;
};

} // namespace QCPL

#endif // QCPL_EXPORT_H
