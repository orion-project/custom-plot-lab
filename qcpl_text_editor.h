#ifndef QCPL_UTILS_H
#define QCPL_UTILS_H

#include <QWidget>

#include "qcpl_types.h"

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QComboBox;
class QFontComboBox;
class QPlainTextEdit;
class QToolButton;
QT_END_NAMESPACE

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
        bool showAlignment = false;
    };

    TextEditorWidget(const Options& opts);

    void setText(const QString& text);
    void setFont(const QFont& font);
    void setColor(const QColor& color);
    void setTextFlags(int flags);
    QString text() const;
    QFont font() const;
    QColor color() const { return _color; }
    int textFlags() const;

    QPlainTextEdit* editor() const { return _editor; }

signals:
    void acceptRequested();

private slots:
    void setFontFamily(const QString& family);
    void setFontSize(const QString& size);
    void setTextAlign(QAction *a);

private:
    QPlainTextEdit *_editor;
    QColor _color;
    QAction *_actnBold, *_actnItalic, *_actnUnderline, *_actnColor,
        *_actionAlignLeft, *_actionAlignCenter, *_actionAlignRight, *_actionAlignJustify;
    QActionGroup *_actionsAlignment = nullptr;
    QFontComboBox *_comboFont;
    QComboBox *_comboSize;
    QToolButton *_alignButton;
    int _textFlags = 0;

    void toggleBold();
    void toggleItalic();
    void toggleUnderline();
    void selectColor();
    void insertVar();
    void alignmentChanged(Qt::Alignment a);
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

#endif // QCPL_UTILS_H
