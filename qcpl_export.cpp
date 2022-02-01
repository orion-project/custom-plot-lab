#include "qcpl_export.h"

#include <QApplication>
#include <QClipboard>
#include <QTextStream>

#include "qcpl_types.h"

namespace QCPL {

class ExporterImpl
{
public:
    ExporterImpl(const GraphDataExportSettings& settings)
    {
        _formatter = new QTextStream(&_formatted);
        _formatter->setRealNumberNotation(QTextStream::SmartNotation);
        _formatter->setRealNumberPrecision(settings.numberPrecision);
        _formatter->setLocale(settings.systemLocale ? QLocale::system() : QLocale::c());

        _quote = settings.csv && _formatter->locale().decimalPoint() == ',';
        _csv = settings.csv;
    }

    virtual ~ExporterImpl()
    {
        delete _formatter;
    };

    virtual void add(const QString& v) = 0;
    virtual void add(const QString& x, const QString& y) = 0;
    virtual QString result() const = 0;

    QString format(const double& v)
    {
        _formatted.clear();
        *_formatter << v;
        return _formatted;
    }

protected:
    bool _quote, _csv;

    void addValue(QTextStream* stream, const QString& v)
    {
        if (_quote)
            *stream << '"' << v << '"';
        else *stream << v;
    }

    void addSeparator(QTextStream* stream)
    {
        *stream << (_csv ? ',' : '\t');
    }

    void addNewline(QTextStream* stream)
    {
        *stream << '\n';
    }
private:
    QString _formatted;
    QTextStream *_formatter;
};

namespace {

class ColumnExporter : public ExporterImpl
{
public:
    ColumnExporter(const GraphDataExportSettings& settings) : ExporterImpl(settings)
    {
        _stream = new QTextStream(&_result);
    }

    ~ColumnExporter()
    {
        delete _stream;
    }

    void add(const QString& v) override
    {
        addValue(_stream, v);
        addNewline(_stream);
    }

    void add(const QString& x, const QString& y) override
    {
        addValue(_stream, x);
        addSeparator(_stream);
        addValue(_stream, y);
        addNewline(_stream);
    }

    QString result() const override
    {
        return _result;
    }

private:
    QString _result;
    QTextStream *_stream;
};

class RowExporter : public ExporterImpl
{
public:
    RowExporter(const GraphDataExportSettings& settings) : ExporterImpl(settings)
    {
        _streamX = new QTextStream(&_resultX);
        _streamY = new QTextStream(&_resultY);
    }

    ~RowExporter()
    {
        delete _streamX;
        delete _streamY;
    }

    void add(const QString& v) override
    {
        addValue(_streamX, v);
        addSeparator(_streamX);
    }

    void add(const QString& x, const QString& y) override
    {
        addValue(_streamX, x);
        addSeparator(_streamX);

        addValue(_streamY, y);
        addSeparator(_streamY);
    }

    QString result() const override
    {
        QString rx = _resultX.trimmed();
        QString ry = _resultY.trimmed();
        if (rx.endsWith(',')) rx = rx.left(rx.length()-1);
        if (ry.endsWith(',')) ry = ry.left(ry.length()-1);
        if (ry.isEmpty())
            return rx + '\n';
        return rx + '\n' + ry + '\n';
    }

private:
    QString _resultX, _resultY;
    QTextStream *_streamX, *_streamY;
};

} // namespace

//------------------------------------------------------------------------------
//                             GraphDataExporter
//------------------------------------------------------------------------------

GraphDataExporter::GraphDataExporter(const GraphDataExportSettings& settings)
{
    if (settings.transposed)
        _impl = new RowExporter(settings);
    else
        _impl = new ColumnExporter(settings);
    _merge = settings.mergePoints;
}

GraphDataExporter::~GraphDataExporter()
{
    delete _impl;
}

void GraphDataExporter::add(const double &v)
{
    auto str = _impl->format(v);

    if (_merge)
    {
        if (_prev == str)
            return;
        _prev = str;
    }

    _impl->add(str);
}

void GraphDataExporter::add(const double &x, const double &y)
{
    auto strX = _impl->format(x);
    auto strY = _impl->format(y);

    if (_merge)
    {
        if (_prevX == strX && _prevY == strY)
            return;
        _prevX = strX;
        _prevY = strY;
    }

    _impl->add(strX, strY);
}

void GraphDataExporter::add(const QVector<double>& vs)
{
    foreach (const auto& v, vs)
        add(v);
}

void GraphDataExporter::toClipboard()
{
    qApp->clipboard()->setText(_impl->result());
}

} // namespace QCPL
