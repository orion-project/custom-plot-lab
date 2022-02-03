#include "qcpl_format.h"
#include "qcpl_plot.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriWidgets.h"
#include "widgets/OriValueEdit.h"

#include <QPlainTextEdit>

namespace QCPL {

bool axisTitleDlg(QCPAxis* axis, const QString& title)
{
    QPlainTextEdit editor;
    editor.setPlainText(axis->label());

    if (Ori::Dlg::Dialog(&editor, false)
            .withTitle(title)
            .withContentToButtonsSpacingFactor(3)
            .exec())
    {
        axis->setLabel(editor.toPlainText().trimmed());
        return true;
    }
    return false;
}

bool axisLimitsDlg(QCPRange& range, const QString &title, const AxisLimitsDlgProps& props)
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
    layout->setMargin(0);
    layout->addRow(new QLabel(props.unit.isEmpty() ? QString("Min") : QString("Min (%1)").arg(props.unit)), editorMin);
    layout->addRow(new QLabel(props.unit.isEmpty() ? QString("Max") : QString("Max (%1)").arg(props.unit)), editorMax);

    if (Ori::Dlg::Dialog(&w, false)
            .withTitle(title)
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

} // namespace QCPL
