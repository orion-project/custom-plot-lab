#include "qcpl_format_title.h"

#include "qcpl_format.h"
#include "qcpl_format_editors.h"
#include "qcpl_text_editor.h"
#include "qcpl_plot.h"

#include "helpers/OriLayouts.h"
#include "widgets/OriLabels.h"

#include <QAction>
#include <QDebug>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpinBox>

using namespace Ori::Layouts;

namespace QCPL {

TitleFormatWidget::TitleFormatWidget(QCPTextElement* title, const TitleFormatDlgProps& props) : QWidget(), _title(title)
{
    _onSaveDefault = props.onSaveDefault;

    TextEditorWidget::Options textOpts;
    textOpts.showAlignment = true;
    _textProps = new TextEditorWidget(textOpts);

    _visible = new QCheckBox(tr("Visible"));
    _saveDefault = new QCheckBox(tr("Save as default format"));
    _saveDefault->setVisible(bool(_onSaveDefault));

    MarginsEditorWidget::Options marginOpts;
    marginOpts.layoutInLine = true;
    _margins = new MarginsEditorWidget(tr("Margins"), marginOpts);

    auto separator = new Ori::Widgets::LabelSeparator;
    separator->flat = true;

    LayoutV({
        _visible,
        _saveDefault,
        separator,
        LayoutV({_textProps}).makeGroupBox(tr("Text")),
        _margins,
        Stretch(),
    }).setMargin(0).useFor(this);

    _visible->setChecked(title->visible());
    _textProps->setText(title->text());
    _textProps->setFont(title->font());
    _textProps->setColor(title->textColor());
    _textProps->setTextFlags(title->textFlags());
    _margins->setValue(title->margins());
}

void TitleFormatWidget::apply()
{
    _title->setText(_textProps->text());
    _title->setFont(_textProps->font());
    _title->setSelectedFont(_textProps->font());
    _title->setTextColor(_textProps->color());
    _title->setVisible(_visible->isChecked());
    _title->setTextFlags(_textProps->textFlags());
    _title->setMargins(_margins->value());
    auto plot = qobject_cast<Plot*>(_title->parentPlot());
    if (plot) plot->updateTitleVisibility();
    if (_onSaveDefault and _saveDefault->isChecked())
        _onSaveDefault();
}
} // namespace QCPL
