#ifndef QCPL_TEXT_EDITOR_H
#define QCPL_TEXT_EDITOR_H

#include <QWidget>

#include "qcpl_types.h"

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QToolBar;
class QToolButton;
QT_END_NAMESPACE

namespace Ori {
namespace Widgets {
class MenuToolButton;
class BaseColorButton;
}}

namespace QCPL {

/// Full text edit with font and color selectors
class TextEditorWidget : public QWidget
{
    Q_OBJECT

public:
    struct Options
    {
        /// If non zero, it will override the default icon size for text toolbar.
        QSize iconSize;

        /// A list of variables that can be inserted into text via special menu.
        /// Or one just can type variable names and they will be highlighted in the text
        /// as having special meannig.
        QVector<TextVariable> vars;

        /// If provided, a command added to vars menu to reset the editor text to this value.
        QString defaultText;

        bool readOnly = false;

        /// Use line editor instead of text editor.
        /// If provided, @ref multiLineEditor() returns null.
        /// Vars menu doesn't work in this mode.
        bool singleLine = false;

        /// Show text alignment drop-down
        bool showAlignment = false;

        /// Show background color selector button in teh toolbar
        bool showBackColor = false;

        /// Use color with alpha-channel for text
        bool colorAlphaText = false;

        /// Use color with alpha-channel for background
        bool colorAlphaBack = false;

        /// Arrange buttons in two narrower tool bars instead of one longer
        bool narrow = false;
    };

    TextEditorWidget(const Options& opts);

    void setText(const QString& text);
    void setFont(const QFont& font);
    void setColor(const QColor& color);
    void setBackColor(const QColor& color);
    void setTextFlags(int flags);
    QString text() const;
    QFont font() const;
    QColor color() const { return _color; }
    QColor backColor() const { return _backColor; }
    int textFlags() const;

    void addAction(QAction *actn, bool secondToolbar = false);

    QPlainTextEdit* multiLineEditor() const { return _textEditor; }
    QLineEdit* singleLineEditor() const { return _lineEditor; }

signals:
    void acceptRequested();

private slots:
    void setFontFamily(const QString& family);
    void setFontSize(const QString& size);

private:
    Options _opts;
    QPlainTextEdit *_textEditor = nullptr;
    QLineEdit *_lineEditor = nullptr;
    QColor _color, _backColor;
    QAction *_actnBold, *_actnItalic, *_actnUnderline;
    Ori::Widgets::MenuToolButton* _btnAlign = nullptr;
    Ori::Widgets::BaseColorButton *_btnTextColor, *_btnBackColor = nullptr;
    QFontComboBox *_comboFont;
    QComboBox *_comboSize;
    QToolBar *_toolbar1, *_toolbar2;
    int _textFlags = 0;

    QWidget* editorWidget();
    QFont currentFont() const;
    void setCurrentFont(const QFont& font);

    void selectFont();
    void toggleBold();
    void toggleItalic();
    void toggleUnderline();
    void insertVar();
};

/// Simplified text editor having only text field and variables button
class TextOnlyEditorWidget : public QWidget
{
    Q_OBJECT

public:
    struct Options
    {
        /// If provided, a command added to vars menu to reset editot text to this value
        QString defaultText;

        /// A list of variables that can be inserted into text via special menu.
        /// Or one just can type variable names and they will be highlighted in the text
        /// as having special meannig
        QVector<TextVariable> vars;
    };

    TextOnlyEditorWidget(const Options& opts);

    void setText(const QString& text);
    QString text() const;

    QPlainTextEdit* editor() const { return _editor; }

signals:
    void acceptRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QPlainTextEdit *_editor;

    void insertVar();
};

} // namespace QCPL

#endif // QCPL_TEXT_EDITOR_H
