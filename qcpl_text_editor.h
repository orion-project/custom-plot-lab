#ifndef QCPL_TEXT_EDITOR_H
#define QCPL_TEXT_EDITOR_H

#include <QWidget>

#include "qcpl_types.h"

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QPlainTextEdit;
class QToolBar;
class QToolButton;
QT_END_NAMESPACE

namespace Ori {
namespace Widgets {
class MenuToolButton;
}}

namespace QCPL {

/// Full text edit with font and color selectors
class TextEditorWidget : public QWidget
{
    Q_OBJECT

public:
    struct Options
    {
        QSize iconSize;
        QVector<TextVariable> vars;
        /// Show text alignment drop-down
        bool showAlignment = false;
        /// Show background color selector
        bool showBackColor = false;
        /// Use color with alpha-channel for text
        bool colorAlphaText = false;
        /// Use color with alpha-channel for background
        bool colorAlphaBack = false;
        /// Arrange buttons in two narrower panels instead of one long
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

    QPlainTextEdit* editor() const { return _editor; }

signals:
    void acceptRequested();

private slots:
    void setFontFamily(const QString& family);
    void setFontSize(const QString& size);

private:
    Options _opts;
    QPlainTextEdit *_editor;
    QColor _color, _backColor;
    QAction *_actnBold, *_actnItalic, *_actnUnderline, *_actnColor;
    QAction *_actnBackColor = nullptr;
    Ori::Widgets::MenuToolButton* _btnAlign = nullptr;
    QFontComboBox *_comboFont;
    QComboBox *_comboSize;
    QToolBar *_toolbar1, *_toolbar2;
    int _textFlags = 0;

    void selectFont();
    void toggleBold();
    void toggleItalic();
    void toggleUnderline();
    void selectColor();
    void selectBackColor();
    void insertVar();
};

/// Simplified text editor having only text field and variables button
class TextEditorWidgetV2 : public QWidget
{
    Q_OBJECT

public:
    struct Options
    {
        QString defaultText;
        QVector<TextVariable> vars;
    };

    TextEditorWidgetV2(const Options& opts);

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
