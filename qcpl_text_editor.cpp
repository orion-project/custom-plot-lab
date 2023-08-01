#include "qcpl_text_editor.h"

#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"
#include "widgets/OriMenuToolButton.h"

#include <QApplication>
#include <QColorDialog>
#include <QDebug>
#include <QFontComboBox>
#include <QFontDialog>
#include <QLabel>
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
        s.replace(QStringLiteral("("), QStringLiteral("\\("));
        s.replace(QStringLiteral(")"), QStringLiteral("\\)"));
        s.replace(QStringLiteral("["), QStringLiteral("\\["));
        s.replace(QStringLiteral("]"), QStringLiteral("\\]"));
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
    QToolButton *varsButton = nullptr;
    int preferredWidth = 0;

    // TODO: default text should be processed separately from vars
    void setVars(const QString& defaultText, const QVector<TextVariable>& vars)
    {
        auto highlighter = new TextEditorHighlighter(document());
        auto varsMenu = new QMenu(this);
        if (!defaultText.isEmpty())
        {
            varsMenu->addAction(tr("Reset to default"), [this, &defaultText]{ setPlainText(defaultText); });
            varsMenu->addSeparator();
        }
        foreach (const auto& var, vars)
        {
            highlighter->addVar(var.name);
            auto action = varsMenu->addAction(var.descr + '\t' + var.name, this, &TextEditor::insertVar);
            action->setData(var.name);
        }
        varsButton = new QToolButton;
        varsButton->setIcon(QIcon(":/qcpl_images/var"));
        varsButton->setToolTip(tr("Insert Variable"));
        varsButton->setPopupMode(QToolButton::InstantPopup);
        varsButton->setMenu(varsMenu);
    }

    QSize sizeHint() const override
    {
        auto sz = QPlainTextEdit::sizeHint();
        return {
            preferredWidth > 0 ? preferredWidth : sz.width()*2,
            sz.height()/3
        };
    }


protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        if (event->modifiers().testFlag(Qt::ControlModifier) && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return))
            requestAccept();
        else QPlainTextEdit::keyPressEvent(event);
    }


private:
    void insertVar()
    {
        auto action = qobject_cast<QAction*>(sender());
        textCursor().insertText(action->data().toString());
    }
};

//------------------------------------------------------------------------------
//                                TextEditorWidget
//------------------------------------------------------------------------------

static QPixmap makeColorIcon(QMap<QString, QPixmap>& icons, const QString& baseIcon, const QBrush &b)
{
    auto n = b.style() == Qt::NoBrush ? QStringLiteral("empty") : b.color().name();
    if (icons.contains(n))
        return icons[n];
    auto pixmap = QIcon(baseIcon).pixmap(24, 24);
    QPainter p(&pixmap);
    p.setPen(b.color());
    p.setBrush(b);
    p.drawRect(0, 18, 23, 5);
    icons[n] = pixmap;
    return pixmap;
}

static QPixmap makeTextColorIcon(const QBrush &b)
{
    static QMap<QString, QPixmap> icons;
    return makeColorIcon(icons, QStringLiteral(":/qcpl_images/text_color"), b);
}

static QPixmap makeBackColorIcon(const QBrush &b)
{
    static QMap<QString, QPixmap> icons;
    return makeColorIcon(icons, QStringLiteral(":/qcpl_images/back_color"), b);
}

TextEditorWidget::TextEditorWidget(const Options &opts) : QWidget()
{
    _opts = opts;

    auto p = sizePolicy();
    p.setVerticalStretch(255);
    setSizePolicy(p);

    _toolbar1 = new QToolBar;
    if (!opts.iconSize.isEmpty())
        _toolbar1->setIconSize(opts.iconSize);
    _toolbar1->setContentsMargins(0, 0, 0, 0);

    _toolbar2 = new QToolBar;
    if (!opts.iconSize.isEmpty())
        _toolbar2->setIconSize(opts.iconSize);
    _toolbar2->setContentsMargins(0, 0, 0, 0);
    _toolbar2->setVisible(opts.narrow);

    auto toolbar = [&opts, this](){
        return opts.narrow ? _toolbar2 : _toolbar1;
    };

    _comboFont = new QFontComboBox;
    _comboFont->setMaxVisibleItems(16);
    connect(_comboFont, SIGNAL(currentTextChanged(QString)), this, SLOT(setFontFamily(QString)));
    _toolbar1->addWidget(_comboFont);
    _toolbar1->addWidget(new QLabel(" "));
    if (opts.narrow)
    {
        QFontMetrics fm(_comboFont->font());
        auto r = fm.boundingRect("Time New Roman");
        _comboFont->setFixedWidth(r.width() * 1.5);
    }

    _comboSize = new QComboBox;
    _comboSize->setEditable(true);
    _comboSize->setMaxVisibleItems(24);
    QList<int> fontSizes = QFontDatabase::standardSizes();
    foreach(int size, fontSizes) _comboSize->addItem(QString::number(size));
    connect(_comboSize, SIGNAL(currentTextChanged(QString)), this, SLOT(setFontSize(QString)));
    _toolbar1->addWidget(_comboSize);
    if (!opts.narrow)
        _toolbar1->addWidget(new QLabel(" "));

    _actnBold = toolbar()->addAction(QIcon(":/qcpl_images/bold"), tr("Bold"), this, &TextEditorWidget::toggleBold);
    _actnItalic = toolbar()->addAction(QIcon(":/qcpl_images/italic"), tr("Italic"), this, &TextEditorWidget::toggleItalic);
    _actnUnderline = toolbar()->addAction(QIcon(":/qcpl_images/underline"), tr("Underline"), this, &TextEditorWidget::toggleUnderline);
    _actnBold->setShortcut(QKeySequence::Bold);
    _actnItalic->setShortcut(QKeySequence::Italic);
    _actnUnderline->setShortcut(QKeySequence::Underline);
    _actnBold->setCheckable(true);
    _actnItalic->setCheckable(true);
    _actnUnderline->setCheckable(true);
    if (opts.narrow)
        _toolbar2->addSeparator();

    _actnColor = toolbar()->addAction(tr("Color..."), this, &TextEditorWidget::selectColor);
    if (opts.showBackColor)
        _actnBackColor = toolbar()->addAction(tr("Back Color..."), this, &TextEditorWidget::selectBackColor);
    if (opts.narrow)
        _toolbar2->addSeparator();

    toolbar()->addAction(QIcon(":/qcpl_images/font_dlg"), tr("Select font via dialog"), this, &TextEditorWidget::selectFont);

    if (opts.showAlignment)
    {
        _btnAlign = new Ori::Widgets::MenuToolButton;
        _btnAlign->addAction(Qt::AlignLeft, tr("Left"), ":/qcpl_images/align_left");
        _btnAlign->addAction(Qt::AlignHCenter, tr("Center"), ":/qcpl_images/align_center");
        _btnAlign->addAction(Qt::AlignRight, tr("Right"), ":/qcpl_images/align_right");
        _btnAlign->addAction(Qt::AlignJustify, tr("Justify"), ":/qcpl_images/align_just");
        toolbar()->addWidget(_btnAlign);
    }

    auto editor = new TextEditor;
    editor->requestAccept = [this]{ emit acceptRequested(); };
    if (opts.narrow)
    {
        editor->preferredWidth = 100;
        auto p = editor->sizePolicy();
        p.setHorizontalPolicy(QSizePolicy::Minimum);
        editor->setSizePolicy(p);
    }

    if (!opts.vars.isEmpty())
    {
        editor->setVars(QString(), opts.vars);
        _toolbar1->addWidget(editor->varsButton);
    }

    _editor = editor;
    Ori::Layouts::LayoutV({_toolbar1, _toolbar2, _editor}).setSpacing(0).setMargin(0).useFor(this);
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
    _actnColor->setIcon(makeTextColorIcon(_color));
}

void TextEditorWidget::setBackColor(const QColor& color)
{
    if (!_actnBackColor) return;
    _backColor = color;
    _actnBackColor->setIcon(makeBackColorIcon(_backColor));
}

void TextEditorWidget::setTextFlags(int flags)
{
    _textFlags = flags;
    if (_btnAlign)
        _btnAlign->setSelectedFlags(flags);
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
    dlg.setOption(QColorDialog::ShowAlphaChannel, _opts.colorAlphaText);
    dlg.setCurrentColor(_color);
    if (dlg.exec())
        setColor(dlg.selectedColor());
}

void TextEditorWidget::selectBackColor()
{
    QColorDialog dlg;
    dlg.setOption(QColorDialog::ShowAlphaChannel, _opts.colorAlphaBack);
    dlg.setCurrentColor(_backColor);
    if (dlg.exec())
        setBackColor(dlg.selectedColor());
}

int TextEditorWidget::textFlags() const
{
    return _btnAlign ? _btnAlign->selectedFlags(_textFlags) : _textFlags;
}

void TextEditorWidget::selectFont()
{
    bool ok;
    QFont f = QFontDialog::getFont(&ok, _editor->font(), this);
    if (!ok) return;
    setFont(f);
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

void TextEditorWidget::addAction(QAction *actn, bool secondToolbar)
{
    (secondToolbar ? _toolbar2 : _toolbar1)->addAction(actn);
}

//------------------------------------------------------------------------------
//                            TextEditorWidgetV2
//------------------------------------------------------------------------------

TextEditorWidgetV2::TextEditorWidgetV2(const Options &opts) : QWidget()
{
    auto p = sizePolicy();
    p.setVerticalStretch(255);
    setSizePolicy(p);

    auto editor = new TextEditor;
    Ori::Gui::setFontMonospace(editor);
    editor->requestAccept = [this]{ emit acceptRequested(); };

    if (!opts.vars.isEmpty())
    {
        editor->setVars(opts.defaultText, opts.vars);
        // TODO: get sizes from style?
        editor->varsButton->setIconSize({24, 24});
        editor->varsButton->resize(32, 32);
    }

    _editor = editor;
    Ori::Layouts::LayoutV({_editor}).setSpacing(0).setMargin(0).useFor(this);

    if (editor->varsButton)
        editor->varsButton->setParent(this);
}

void TextEditorWidgetV2::setText(const QString& text)
{
    _editor->setPlainText(text);
}

QString TextEditorWidgetV2::text() const
{
    return _editor->toPlainText().trimmed();
}

void TextEditorWidgetV2::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    auto button = ((TextEditor*)_editor)->varsButton;
    if (button)
    {
        auto r = _editor->geometry();
        auto m = qApp->style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing);
        button->move(r.right() - button->width() - m, r.top() + m);
    }
}

} // namespace QCPL
