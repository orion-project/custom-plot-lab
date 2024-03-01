#include "qcpl_export.h"

#include "helpers/OriLayouts.h"
#include "helpers/OriDialogs.h"
#include "helpers/OriWidgets.h"

#include "qcustomplot/qcustomplot.h"

#include <QApplication>
#include <QClipboard>
#include <QCheckBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextStream>

using namespace Ori::Layouts;

namespace QCPL {

//------------------------------------------------------------------------------
//                             BaseGraphDataExporter
//------------------------------------------------------------------------------

BaseGraphDataExporter::BaseGraphDataExporter(const GraphDataExportSettings& settings, bool noResult)
{
    _formatter = new QTextStream(&_formatted);
    _formatter->setRealNumberNotation(QTextStream::SmartNotation);
    _formatter->setRealNumberPrecision(settings.numberPrecision);
    _formatter->setLocale(settings.systemLocale ? QLocale::system() : QLocale::c());

    _quote = settings.csv && _formatter->locale().decimalPoint() == ',';
    _csv = settings.csv;

    if (!noResult)
        _stream = new QTextStream(&_result);
}

BaseGraphDataExporter::~BaseGraphDataExporter()
{
    delete _formatter;
    if (_stream)
        delete _stream;
}

QString BaseGraphDataExporter::format(const double& v)
{
    _formatted.clear();
    *_formatter << v;
    return _formatted;
}

void BaseGraphDataExporter::addValue(QTextStream* stream, const QString& v)
{
    if (_quote)
        *stream << '"' << v << '"';
    else *stream << v;
}

void BaseGraphDataExporter::addSeparator(QTextStream* stream)
{
    *stream << (_csv ? ',' : '\t');
}

void BaseGraphDataExporter::addNewline(QTextStream* stream)
{
    *stream << '\n';
}

void BaseGraphDataExporter::toClipboard()
{
    qApp->clipboard()->setText(result());
}

//------------------------------------------------------------------------------
//                               ExporterImpl
//------------------------------------------------------------------------------

class ExporterImpl : public BaseGraphDataExporter
{
public:
    ExporterImpl(const GraphDataExportSettings& settings, bool noResult) : BaseGraphDataExporter(settings, noResult) {}
    virtual void add(const QString& v) = 0;
    virtual void add(const QString& x, const QString& y) = 0;
};


namespace {

class ColumnExporter : public ExporterImpl
{
public:
    ColumnExporter(const GraphDataExportSettings& settings) : ExporterImpl(settings, false) {}

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
};


class RowExporter : public ExporterImpl
{
public:
    RowExporter(const GraphDataExportSettings& settings) : ExporterImpl(settings, true)
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
    _impl->toClipboard();
}

//------------------------------------------------------------------------------
//                              exportImageDlg
//------------------------------------------------------------------------------

QJsonObject ExportToImageProps::toJson() const
{
    return QJsonObject({
        { "fileName", fileName },
        { "width", width },
        { "height", height },
        { "proportional", proportional },
        { "scalePixels", scalePixels },
    });
}

void ExportToImageProps::fromJson(const QJsonObject& obj)
{
    fileName = obj["fileName"].toString();
    width = obj["width"].toInt();
    height = obj["height"].toInt();
    proportional = obj["proportional"].toBool(true);
    scalePixels = obj["scalePixels"].toBool(false);
}

class ExportImageDlg
{
    Q_DECLARE_TR_FUNCTIONS(ExportImageDlg)

public:
    ExportImageDlg(QCustomPlot* plot, ExportToImageProps& props) : plot(plot) {
        edFile = new QLineEdit;
        edFile->setText(props.fileName);
        edFile->connect(edFile, &QLineEdit::editingFinished, edFile, [this]{ updateFileStatus(); });

        labFileStatus = new QLabel;

        auto butSelectFile = new QPushButton(tr("Select..."));
        butSelectFile->connect(butSelectFile, &QPushButton::clicked, butSelectFile, [this]{ selectTargetFile(); });

        seWidth = Ori::Gui::spinBox(100, 10000, props.width > 0 ? props.width : plot->width());
        seHeight = Ori::Gui::spinBox(100, 10000, props.height > 0 ? props.height : plot->height());
        seWidth->connect(seWidth, QOverload<int>::of(&QSpinBox::valueChanged), seWidth, [this](int w){ widthChanged(w); });
        seHeight->connect(seHeight, QOverload<int>::of(&QSpinBox::valueChanged), seHeight, [this](int h){ heightChanged(h); });
        aspect = seWidth->value() / double(seHeight->value());

        auto sizeLayout = new QFormLayout;
        sizeLayout->addRow(tr("Widht"), seWidth);
        sizeLayout->addRow(tr("Height"), seHeight);

        cbProportional = new QCheckBox(tr("Proportional"));
        cbProportional->setChecked(props.proportional);

        cbScalePixels = new QCheckBox(tr("Scale as bitmap"));
        cbScalePixels->setChecked(props.scalePixels);

        auto butResetSize = new QPushButton("  " +  tr("Set size as on screen") + "  ");
        butResetSize->connect(butResetSize, &QPushButton::clicked, butResetSize, [this]{ resetImageSize(); });

        updateFileStatus();

        content = content = LayoutV({
            LayoutV({
                edFile,
                LayoutH({labFileStatus, SpaceH(2), Stretch(), butSelectFile}),
            }).makeGroupBox(tr("Target file")),
            LayoutV({
                LayoutH({
                    sizeLayout,
                    Stretch(),
                    SpaceH(4),
                    LayoutV({
                        cbProportional,
                        cbScalePixels,
                    }),
                }),
                SpaceV(),
                LayoutH({
                    butResetSize,
                    Stretch(),
                })
            }).makeGroupBox(tr("Image size")),
        }).setMargin(0).makeWidgetAuto();
    }

    void updateFileStatus() {
        QString fn = edFile->text().trimmed();
        if (fn.isEmpty()) {
            labFileStatus->setText(tr("File not selected"));
            return;
        }
        QFileInfo fi(fn);
        edFile->setText(fi.absoluteFilePath());
        if (fi.exists()) {
            labFileStatus->setText("<font color=orange>" + tr("File exists, will be overwritten") + "</font>");
            selectedDir = fi.dir().absolutePath();
        } else {
            auto dir = fi.dir();
            if (dir.exists()) {
                selectedDir = dir.absolutePath();
                labFileStatus->setText("<font color=green>" + tr("File not found, will be created") + "</font>");
            }
            else
                labFileStatus->setText("<font color=red>" + tr("Target directory does not exist") + "</font>");
        }
    }

    bool selectTargetFile() {
        const QStringList filters = {
            tr("PNG Images (*.png)"),
            tr("JPG Images (*.jpg *.jpeg)"),
        };
        const QStringList filterExts = { "png", "jpg" };
        Q_ASSERT(filters.size() == filterExts.size());
        QFileDialog dlg(qApp->activeModalWidget(), tr("Select a file name"), selectedDir);
        dlg.setNameFilters(filters);
        QString selectedFile = edFile->text().trimmed();
        if (!selectedFile.isEmpty()) {
            dlg.selectFile(selectedFile);
            QString ext = QFileInfo(selectedFile).suffix().toLower();
            for (const auto& f : filters) {
                if (f.contains(ext)) {
                    dlg.selectNameFilter(f);
                    break;
                }
            }
        }
        if (!dlg.exec())
            return false;
        selectedFile = dlg.selectedFiles().at(0);
        if (QFileInfo(selectedFile).suffix().isEmpty()) {
            QString selectedFilter = dlg.selectedNameFilter();
            for (int i = 0; i < filters.size(); i++)
                if (selectedFilter == filters.at(i)) {
                    selectedFile += "." + filterExts.at(i);
                    break;
                }
        }
        edFile->setText(selectedFile);
        updateFileStatus();
        return true;
    }

    void resetImageSize() {
        skipSizeChange = true;
        seWidth->setValue(plot->width());
        seHeight->setValue(plot->height());
        skipSizeChange = false;
    }

    void widthChanged(int w) {
        if (skipSizeChange) return;
        skipSizeChange = true;
        if (cbProportional->isChecked())
            seHeight->setValue(w / aspect);
        else aspect = w / seHeight->value();
        skipSizeChange = false;
    }

    void heightChanged(int h) {
        if (skipSizeChange) return;
        skipSizeChange = true;
        if (cbProportional->isChecked())
            seWidth->setValue(h * aspect);
        else aspect = seWidth->value() / double(h);
        skipSizeChange = false;
    }

    bool exec() {
        return Ori::Dlg::Dialog(content)
            .withVerification([this]{
                if (!QFileInfo(edFile->text()).dir().exists())
                    return tr("Target directory does not exist");
                return QString();
            })
            .withContentToButtonsSpacingFactor(2)
            .withPersistenceId("exportImageDlg")
            .withTitle(tr("Export Image"))
            .exec();
    }

    void fillProps(ExportToImageProps& props) {
        props.fileName = edFile->text().trimmed();
        props.width = seWidth->value();
        props.height = seHeight->value();
        props.proportional = cbProportional->isChecked();
        props.scalePixels = cbScalePixels->isChecked();
    }

    QCustomPlot *plot;
    QLineEdit *edFile;
    QLabel *labFileStatus;
    QSpinBox *seWidth, *seHeight;
    QCheckBox *cbProportional, *cbScalePixels;
    QSharedPointer<QWidget> content;
    double aspect = 1;
    bool skipSizeChange = false;
    QString selectedDir;
};

bool exportImageDlg(QCustomPlot* plot, ExportToImageProps& props)
{
    ExportImageDlg dlg(plot, props);
    if (!dlg.exec())
        return false;
    dlg.fillProps(props);
    if (props.fileName.isEmpty())
    {
        if (!dlg.selectTargetFile())
            return false;
        dlg.fillProps(props);
    }
    bool saved = true;
    if (!props.scalePixels)
    {
        QImage image(props.width, props.height, QImage::Format_RGB32);
        QCPPainter painter(&image);
        plot->toPainter(&painter, props.width, props.height);
        saved = image.save(props.fileName);
    }
    else
    {
        int w = plot->width();
        int h = plot->height();
        QImage image(w, h, QImage::Format_RGB32);
        QCPPainter painter(&image);
        plot->toPainter(&painter);
        if (props.width != w || props.height != h)
        {
            // Don't reassign the previous image variable, make a new one.
            // Otherwise there could be error:
            // QPaintDevice: Cannot destroy paint device that is being painted
            QImage image1 = image.scaled(props.width, props.height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            saved = image1.save(props.fileName);
        }
        else saved = image.save(props.fileName);
    }
    if (!saved)
        Ori::Dlg::error(qApp->translate("ExportImageDlg", "Failed to save image"));
    return true;
}

} // namespace QCPL
