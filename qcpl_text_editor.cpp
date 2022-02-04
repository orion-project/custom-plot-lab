#include "qcpl_text_editor.h"

#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"

#include <QColorDialog>
#include <QDebug>
#include <QFontComboBox>
#include <QMenu>
#include <QPainter>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QToolBar>
#include <QToolButton>

namespace QCPL {

//------------------------------------------------------------------------------
//                             TextEditorHighlighter
//------------------------------------------------------------------------------

class TextEditorHighlighter : public QSyntaxHighlighter
{
public:
    explicit TextEditorHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
    {
        _varFormat.setForeground(Qt::blue);
    }

    void addVar(const QString& name)
    {
        QString s = name;
        s.replace(QStringLiteral("{"), QStringLiteral("\\{"));
        s.replace(QStringLiteral("}"), QStringLiteral("\\}"));
        _vars << QRegularExpression(s);
    }

protected:
    void highlightBlock(const QString &text) override
    {
        foreach (const auto& var, _vars)
        {
            auto m = var.match(text);
            while (m.hasMatch())
            {
                int pos = m.capturedStart(0);
                int length = m.capturedLength(0);
                setFormat(pos, length, _varFormat);
                m = var.match(text, pos + length);
            }
        }
    }

private:
    QVector<QRegularExpression> _vars;
    QTextCharFormat _varFormat;
};

//------------------------------------------------------------------------------
//                                  TextEditor
//------------------------------------------------------------------------------

class TextEditor : public QPlainTextEdit
{
public:
    std::function<void()> requestAccept;

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        if (event->modifiers().testFlag(Qt::ControlModifier) && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return))
            requestAccept();
        else QPlainTextEdit::keyPressEvent(event);
    }
};

//------------------------------------------------------------------------------
//                                TextEditorWidget
//------------------------------------------------------------------------------

static QPixmap makeColorIcon(const QBrush &b)
{
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setPen(b.color());
    p.setBrush(b);
    p.drawRect(5, 5, 14, 14);
    return pixmap;
}

static QWidget* makeSeparator()
{
    auto w = new QWidget;
    w->setFixedWidth(Ori::Gui::layoutSpacing());
    return w;
}

TextEditorWidget::TextEditorWidget(const Options &opts) : QWidget()
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

    toolbar->addWidget(makeSeparator());

    _actnBold = toolbar->addAction(QIcon(":/qcpl_images/bold"), tr("Bold"), this, &TextEditorWidget::toggleBold);
    _actnItalic = toolbar->addAction(QIcon(":/qcpl_images/italic"), tr("Italic"), this, &TextEditorWidget::toggleItalic);
    _actnUnderline = toolbar->addAction(QIcon(":/qcpl_images/underline"), tr("Underline"), this, &TextEditorWidget::toggleUnderline);
    _actnBold->setShortcut(QKeySequence::Bold);
    _actnItalic->setShortcut(QKeySequence::Italic);
    _actnUnderline->setShortcut(QKeySequence::Underline);
    _actnBold->setCheckable(true);
    _actnItalic->setCheckable(true);
    _actnUnderline->setCheckable(true);

    toolbar->addWidget(makeSeparator());

    _actnColor = toolbar->addAction(tr("Color..."), this, &TextEditorWidget::selectColor);

    _editor = new TextEditor;
    ((TextEditor*)_editor)->requestAccept = [this]{ emit acceptRequested(); };

    if (!opts.vars.isEmpty())
    {
        auto highlighter = new TextEditorHighlighter(_editor->document());
        auto varsMenu = new QMenu(this);
        foreach (const auto& var, opts.vars)
        {
            highlighter->addVar(var.name);
            auto action = varsMenu->addAction(var.descr + '\t' + var.name, this, &TextEditorWidget::insertVar);
            action->setData(var.name);
        }
        auto varsButton = new QToolButton;
        varsButton->setIcon(QIcon(":/qcpl_images/var"));
        varsButton->setToolTip(tr("Insert Variable"));
        varsButton->setPopupMode(QToolButton::InstantPopup);
        varsButton->setMenu(varsMenu);
        toolbar->addWidget(makeSeparator());
        toolbar->addWidget(varsButton);
    }

    Ori::Layouts::LayoutV({toolbar, _editor}).setSpacing(0).setMargin(0).useFor(this);
}

void TextEditorWidget::setText(const QString& text)
{
    _editor->setPlainText(text);
}

void TextEditorWidget::setFont(const QFont& font)
{
    _editor->setFont(font);
    _comboFont->setCurrentFont(font);
    _comboSize->setCurrentIndex(_comboSize->findText(QString::number(font.pointSize())));
    _actnBold->setChecked(font.bold());
    _actnItalic->setChecked(font.italic());
    _actnUnderline->setChecked(font.underline());
}

void TextEditorWidget::setColor(const QColor& color)
{
    _color = color;
    _actnColor->setIcon(makeColorIcon(_color));
}

QString TextEditorWidget::text() const
{
    return _editor->toPlainText().trimmed();
}

QFont TextEditorWidget::font() const
{
    return _editor->font();
}

void TextEditorWidget::setFontFamily(const QString& family)
{
    auto font = _editor->font();
    font.setFamily(family);
    _editor->setFont(font);
}

void TextEditorWidget::setFontSize(const QString& size)
{
    qreal pointSize = size.toFloat();
    if (pointSize <= 0) return;
    auto font = _editor->font();
    font.setPointSize(pointSize);
    _editor->setFont(font);
}

void TextEditorWidget::selectColor()
{
    QColorDialog dlg;
    dlg.setCurrentColor(_color);
    if (dlg.exec())
        setColor(dlg.selectedColor());
}

void TextEditorWidget::toggleBold()
{
    auto font = _editor->font();
    font.setBold(_actnBold->isChecked());
    _editor->setFont(font);
}

void TextEditorWidget::toggleItalic()
{
    auto font = _editor->font();
    font.setItalic(_actnItalic->isChecked());
    _editor->setFont(font);
}

void TextEditorWidget::toggleUnderline()
{
    auto font = _editor->font();
    font.setUnderline(_actnUnderline->isChecked());
    _editor->setFont(font);
}

void TextEditorWidget::insertVar()
{
    auto action = qobject_cast<QAction*>(sender());
    _editor->textCursor().insertText(action->data().toString());
}

} // namespace QCPL
