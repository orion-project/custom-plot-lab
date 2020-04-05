#include "qcpl_export.h"

#include <QApplication>
#include <QClipboard>
#include <QTextStream>

#include "qcpl_types.h"

namespace QCPL {

GraphDataExporter::GraphDataExporter(const GraphDataExportSettings& settings): _settings(settings)
{
    _formatter = settings.formatter ? settings.formatter : getDefaultValueFormatter();
    _quote = settings.csv && _formatter->isDecimalComma();
    _stream = new QTextStream(&_result);
}

GraphDataExporter::~GraphDataExporter()
{
    delete _stream;
}

void GraphDataExporter::add(double v)
{
    if (_quote)
        *_stream << '"' << _formatter->format(v) << '"';
    else *_stream << _formatter->format(v);
    *_stream << '\n';
}

void GraphDataExporter::add(double x, double y)
{
    if (_quote)
        *_stream << '"' << _formatter->format(x) << '"';
    else *_stream << _formatter->format(x);

    *_stream << (_settings.csv ? ',' : '\t');

    if (_quote)
        *_stream << '"' << _formatter->format(y) << '"';
    else *_stream << _formatter->format(y);

    *_stream << '\n';
}

void GraphDataExporter::toClipboard()
{
    qApp->clipboard()->setText(_result);
}

} // namespace QCPL
