#include "qcpl_format.h"

#include "qcpl_format_axis.h"
#include "qcpl_format_graph.h"
#include "qcpl_format_legend.h"
#include "qcpl_format_plot.h"
#include "qcpl_io_json.h"
#include "qcpl_plot.h"
#include "qcpl_text_editor.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriWidgets.h"
#include "tools/OriSettings.h"
#include "widgets/OriValueEdit.h"

namespace QCPL {

bool axisTitleDlg(QCPAxis* axis, const AxisTitleDlgProps& props)
{
    auto style = qApp->style();

    TextEditorWidget::Options opts;
    opts.iconSize = props.iconSize;
    if (props.formatter)
        opts.vars = props.formatter->vars();
    TextEditorWidget editor(opts);
    if (props.formatter)
        editor.setText(props.formatter->text());
    else
        editor.setText(axis->label());
    editor.setFont(axis->labelFont());
    editor.setColor(axis->labelColor());
    editor.setContentsMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin)/2, 0,
                              style->pixelMetric(QStyle::PM_LayoutRightMargin)/2, 0);

    if (Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title)
            .withSkipContentMargins()
            .withContentToButtonsSpacingFactor(2)
            .withPersistenceId("axis-title")
            .withAcceptSignal(SIGNAL(acceptRequested()))
            .withActiveWidget(editor.editor())
            .exec())
    {
        if (props.formatter)
        {
            props.formatter->setText(editor.text());
            props.formatter->format();
        }
        else
            axis->setLabel(editor.text());
        axis->setLabelFont(editor.font());
        axis->setLabelColor(editor.color());
        axis->setSelectedLabelFont(editor.font());
        return true;
    }
    return false;
}

bool axisTitleDlgV2(QCPAxis* axis, const AxisTitleDlgPropsV2& props)
{
    auto style = qApp->style();

    TextEditorWidgetV2::Options opts;
    opts.defaultText = props.defaultTitle;
    if (props.formatter)
        opts.vars = props.formatter->vars();
    TextEditorWidgetV2 editor(opts);
    if (props.formatter)
        editor.setText(props.formatter->text());
    else
        editor.setText(axis->label());
    editor.setContentsMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin)/2,
                              style->pixelMetric(QStyle::PM_LayoutTopMargin)/2,
                              style->pixelMetric(QStyle::PM_LayoutRightMargin)/2, 0);

    if (Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title)
            .withSkipContentMargins()
            .withContentToButtonsSpacingFactor(2)
            .withPersistenceId("axis-title-v2")
            .withAcceptSignal(SIGNAL(acceptRequested()))
            .withActiveWidget(editor.editor())
            .exec())
    {
        if (props.formatter)
        {
            props.formatter->setText(editor.text());
            props.formatter->format();
        }
        else
            axis->setLabel(editor.text());
        return true;
    }
    return false;
}

bool axisLimitsDlg(QCPRange& range, const AxisLimitsDlgProps& props)
{
    auto editorMin = new Ori::Widgets::ValueEdit;
    auto editorMax = new Ori::Widgets::ValueEdit;
    Ori::Gui::adjustFont(editorMin);
    Ori::Gui::adjustFont(editorMax);
    editorMin->setNumberPrecision(props.precision);
    editorMax->setNumberPrecision(props.precision);
    editorMin->setValue(range.lower);
    editorMax->setValue(range.upper);
    editorMin->selectAll();

    QWidget w;

    auto layout = new QFormLayout(&w);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addRow(new QLabel(props.unit.isEmpty() ? QString("Min") : QString("Min (%1)").arg(props.unit)), editorMin);
    layout->addRow(new QLabel(props.unit.isEmpty() ? QString("Max") : QString("Max (%1)").arg(props.unit)), editorMax);

    if (Ori::Dlg::Dialog(&w, false)
            .withTitle(props.title)
            .withContentToButtonsSpacingFactor(3)
            .exec())
    {
        range.lower = editorMin->value();
        range.upper = editorMax->value();
        range.normalize();
        return true;
    }
    return false;
}

bool axisFormatDlg(QCPAxis* axis, const AxisFormatDlgProps& props)
{
    AxisFormatWidget editor(axis);

    return Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title)
            .withOnApply([&editor, &props]{ editor.apply(); props.plot->replot(); })
            .withPersistenceId("axis-format")
            .connectOkToContentApply()
            .exec();
}

bool titleFormatDlg(QCPTextElement* title, const TitleFormatDlgProps& props)
{
    auto style = qApp->style();

    TextEditorWidget::Options opts;
    opts.showAlignment = true;

    TextEditorWidget editor(opts);
    editor.setText(title->text());
    editor.setFont(title->font());
    editor.setColor(title->textColor());
    editor.setTextFlags(title->textFlags());
    editor.setContentsMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin)/2, 0,
                              style->pixelMetric(QStyle::PM_LayoutRightMargin)/2, 0);

    if (Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title)
            .withSkipContentMargins()
            .withContentToButtonsSpacingFactor(2)
            .withPersistenceId("plot-title")
            .withAcceptSignal(SIGNAL(acceptRequested()))
            .withActiveWidget(editor.editor())
            .exec())
    {
        title->setText(editor.text());
        title->setFont(editor.font());
        title->setSelectedFont(editor.font());
        title->setTextColor(editor.color());
        title->setTextFlags(editor.textFlags());
        // TODO: cursor line offsets when alignment changes
        return true;
    }
    return false;
}

bool legendFormatDlg(QCPLegend* legend, const LegendFormatDlgProps& props)
{
    LegendFormatWidget editor(legend, props);

    return Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title)
            .withOnApply([&editor, legend]{ editor.apply(); legend->parentPlot()->replot(); })
            .withContentToButtonsSpacingFactor(3)
            .connectOkToContentApply()
            .exec();
}

bool plotFormatDlg(Plot* plot, const PlotFormatDlgProps &props)
{
    PlotFormatWidget editor(plot, {});

    return Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title.isEmpty() ? "Plot Format" : props.title)
            .withOnApply([&editor, plot]{ editor.apply(); plot->replot(); })
            .connectOkToContentApply()
            .exec();
}

bool graphFormatDlg(QCPGraph* graph, const GraphFormatDlgProps& props)
{
    GraphFormatWidget editor(graph);

    return Ori::Dlg::Dialog(&editor, false)
            .withTitle(props.title)
            .withOnApply([&editor, &props]{ editor.apply(); props.plot->replot(); })
            .connectOkToContentApply()
            .exec();
}

//------------------------------------------------------------------------------
//                            QCPL::TextProcessor
//------------------------------------------------------------------------------

void TextProcessor::addVar(const QString& name, TextVarGetter getter)
{
    _vars.append({name, getter});
}

QString TextProcessor::process(const QString& text) const
{
    QString str(text);
    foreach (const auto& var, _vars)
    {
        int pos = str.indexOf(var.name);
        if (pos >= 0)
        {
            QString value = var.getter();
            while (pos >= 0)
            {
                str.replace(pos, var.name.length(), value);
                pos = str.indexOf(var.name, pos + value.length());
            }
        }
    }
    return str;
}

//------------------------------------------------------------------------------
//                           QCPL::TextFormatterBase
//------------------------------------------------------------------------------

void TextFormatterBase::addVar(const QString& name, const QString& descr, TextVarGetter getter)
{
    _vars.append({name, descr});
    _processor.addVar(name, getter);
}

//------------------------------------------------------------------------------
//                          QCPL::AxisTitleFormatter
//------------------------------------------------------------------------------

AxisTitleFormatter::AxisTitleFormatter(QCPAxis* axis): _axis(axis)
{
}

void AxisTitleFormatter::format()
{
    _axis->setLabel(_processor.process(_text).trimmed());
}

//------------------------------------------------------------------------------
//                          QCPL::FormatStorageIni
//------------------------------------------------------------------------------

static QString jsonToStr(const QJsonObject& obj)
{
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

static QJsonObject varToJson(const QVariant& data)
{
    QString str = data.toString();
    if (str.isEmpty()) return QJsonObject();
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
    return doc.isNull() ? QJsonObject() : doc.object();
}

void FormatStorageIni::load(QCustomPlot* plot)
{
    Ori::Settings s;
    s.beginGroup("DefaultPlotFormat");
    QCPL::readLegend(varToJson(s.value("legend")), plot->legend);
}

void FormatStorageIni::saveLegend(QCPLegend* legend)
{
    Ori::Settings s;
    s.beginGroup("DefaultPlotFormat");
    s.setValue("legend", jsonToStr(QCPL::writeLegend(legend)));
}

//------------------------------------------------------------------------------

QString loadFormatFromFile(const QString& fileName, QCustomPlot* plot)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "Unable to open file for reading: " + file.errorString();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (doc.isNull())
        return "Unable to parse json file: " + error.errorString();

    readPlot(doc.object(), plot);
    return QString();
}

QString saveFormatToFile(const QString& fileName, QCustomPlot* plot)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return "Unable to open file for writing: " + file.errorString();
    QTextStream(&file) << QJsonDocument(writePlot(plot)).toJson();
    return QString();
}

} // namespace QCPL
