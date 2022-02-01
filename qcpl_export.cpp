#include "qcpl_export.h"

#include <QApplication>
#include <QClipboard>
#include <QTextStream>

#include "qcpl_types.h"

namespace QCPL {

class ExporterImpl
{
public:
    virtual ~ExporterImpl() {};
    virtual void add(const double& v) = 0;
    virtual void add(const double& x, const double& y) = 0;
    virtual void add(const QVector<double>& v) = 0;
    virtual QString result() const = 0;
protected:
    bool _quote, _csv;

    void addValue(QTextStream* stream, const double& v)
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
};

namespace {

QTextStream* makeStream(QString* target, const GraphDataExportSettings& settings)
{
    auto stream = new QTextStream(target);
    stream->setRealNumberNotation(QTextStream::SmartNotation);
    stream->setRealNumberPrecision(settings.numberPrecision);
    stream->setLocale(settings.systemLocale ? QLocale::system() : QLocale::c());
    return stream;
}

class ColumnExporter : public ExporterImpl
{
public:
    ColumnExporter(const GraphDataExportSettings& settings)
    {
        _stream = makeStream(&_result, settings);
        _quote = settings.csv && _stream->locale().decimalPoint() == ',';
        _csv = settings.csv;
    }

    ~ColumnExporter()
    {
        delete _stream;
    }

    void add(const double& v) override
    {
        addValue(_stream, v);
        addNewline(_stream);
    }

    void add(const double& x, const double& y) override
    {
        addValue(_stream, x);
        addSeparator(_stream);
        addValue(_stream, y);
        addNewline(_stream);
    }

    void add(const QVector<double>& v) override
    {
        int sz = v.size();
        for (int i = 0; i < sz; i++)
        {
            addValue(_stream, v.at(i));
            addNewline(_stream);
        }
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
    RowExporter(const GraphDataExportSettings& settings)
    {
        _streamX = makeStream(&_resultX, settings);
        _streamY = makeStream(&_resultY, settings);
        _quote = settings.csv && _streamX->locale().decimalPoint() == ',';
        _csv = settings.csv;
    }

    ~RowExporter()
    {
        delete _streamX;
        delete _streamY;
    }

    void add(const double& v) override
    {
        addValue(_streamX, v);
        addSeparator(_streamX);
    }

    void add(const double& x, const double& y) override
    {
        addValue(_streamX, x);
        addSeparator(_streamX);

        addValue(_streamY, y);
        addSeparator(_streamY);
    }

    void add(const QVector<double>& v) override
    {
        int sz = v.size();
        for (int i = 0; i < sz; i++)
        {
            addValue(_streamX, v.at(i));
            if (i < sz-1)
                addSeparator(_streamX);
        }
        addNewline(_streamX);
    }

    QString result() const override
    {
        QString rx = _resultX.trimmed();
        QString ry = _resultY.trimmed();
        if (rx.endsWith(',')) rx = rx.left(rx.length()-1);
        if (ry.endsWith(',')) ry = ry.left(ry.length()-1);
        return ry.isEmpty() ? rx : (rx + '\n' + ry + '\n');
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
}

GraphDataExporter::~GraphDataExporter()
{
    delete _impl;
}

void GraphDataExporter::add(const double &v)
{
    _impl->add(v);
}

void GraphDataExporter::add(const double &x, const double &y)
{
    _impl->add(x, y);
}

void GraphDataExporter::add(const QVector<double>& v)
{
    _impl->add(v);
}

void GraphDataExporter::toClipboard()
{
    qApp->clipboard()->setText(_impl->result());
}

} // namespace QCPL
