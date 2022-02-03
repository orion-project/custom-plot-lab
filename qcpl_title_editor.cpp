#include "qcpl_title_editor.h"

#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"

#include <QColorDialog>
#include <QDebug>
#include <QFontComboBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QToolBar>

namespace QCPL {

static QPixmap makeColorIcon(const QBrush &b)
{
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setPen(b.color());
    p.setBrush(b);
    p.drawRect(4, 4, 16, 16);
    return pixmap;
}

static QWidget* makeToolbarSeparator()
{
    auto w = new QWidget;
    w->setFixedWidth(Ori::Gui::layoutSpacing());
    return w;
}

TitleEditor::TitleEditor(const TitleEditorOptions &opts) : QWidget()
{
    auto toolbar = new QToolBar;
    if (!opts.iconSize.isEmpty())
        toolbar->setIconSize(opts.iconSize);

    _comboFont = new QFontComboBox;
    _comboFont->setMaxVisibleItems(16);
    connect(_comboFont, SIGNAL(activated(QString)), this, SLOT(setFontFamily(QString)));
    toolbar->addWidget(_comboFont);

    _comboSize = new QComboBox;
    _comboSize->setEditable(true);
    _comboSize->setMaxVisibleItems(24);
    QList<int> fontSizes = QFontDatabase::standardSizes();
    foreach(int size, fontSizes) _comboSize->addItem(QString::number(size));
    connect(_comboSize, SIGNAL(activated(QString)), this, SLOT(setFontSize(QString)));
    toolbar->addWidget(_comboSize);

    toolbar->addWidget(makeToolbarSeparator());

    _actnBold = toolbar->addAction(QIcon(":/qcpl_images/bold"), tr("Bold"), this, &TitleEditor::toggleBold);
    _actnItalic = toolbar->addAction(QIcon(":/qcpl_images/italic"), tr("Italic"), this, &TitleEditor::toggleItalic);
    _actnUnderline = toolbar->addAction(QIcon(":/qcpl_images/underline"), tr("Underline"), this, &TitleEditor::toggleUnderline);
    _actnBold->setCheckable(true);
    _actnItalic->setCheckable(true);
    _actnUnderline->setCheckable(true);

    toolbar->addWidget(makeToolbarSeparator());

    _actnColor = toolbar->addAction(tr("Color..."), this, &TitleEditor::selectColor);

    _editor = new QPlainTextEdit;

    Ori::Layouts::LayoutV({toolbar, _editor}).setSpacing(0).setMargin(0).useFor(this);
}

void TitleEditor::setText(const QString& text)
{
    _editor->setPlainText(text);
}

void TitleEditor::setFont(const QFont& font)
{
    _editor->setFont(font);
    _comboFont->setCurrentFont(font);
    _comboSize->setCurrentIndex(_comboSize->findText(QString::number(font.pointSize())));
    _actnBold->setChecked(font.bold());
    _actnItalic->setChecked(font.italic());
    _actnUnderline->setChecked(font.underline());
}

void TitleEditor::setColor(const QColor& color)
{
    _color = color;
    _actnColor->setIcon(makeColorIcon(_color));
}

QString TitleEditor::text() const
{
    return _editor->toPlainText().trimmed();
}

QFont TitleEditor::font() const
{
    return _editor->font();
}

void TitleEditor::setFontFamily(const QString& family)
{
    auto font = _editor->font();
    font.setFamily(family);
    _editor->setFont(font);
}

void TitleEditor::setFontSize(const QString& size)
{
    qreal pointSize = size.toFloat();
    if (pointSize <= 0) return;
    auto font = _editor->font();
    font.setPointSize(pointSize);
    _editor->setFont(font);
}

void TitleEditor::selectColor()
{
    QColorDialog dlg;
    dlg.setCurrentColor(_color);
    if (dlg.exec())
        setColor(dlg.selectedColor());
}

void TitleEditor::toggleBold()
{
    auto font = _editor->font();
    font.setBold(_actnBold->isChecked());
    _editor->setFont(font);
}

void TitleEditor::toggleItalic()
{
    auto font = _editor->font();
    font.setItalic(_actnItalic->isChecked());
    _editor->setFont(font);
}

void TitleEditor::toggleUnderline()
{
    auto font = _editor->font();
    font.setUnderline(_actnUnderline->isChecked());
    _editor->setFont(font);
}

} // namespace QCPL
