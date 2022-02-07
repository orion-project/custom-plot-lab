#ifndef QCPL_UTILS_H
#define QCPL_UTILS_H

#include <QWidget>

#include "qcpl_types.h"

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QPlainTextEdit;
QT_END_NAMESPACE

namespace QCPL {

class TextEditorWidget : public QWidget
{
    Q_OBJECT

public:
    struct Options
    {
        QSize iconSize;
        QVector<TextVariable> vars;
    };

    TextEditorWidget(const Options& opts);

    void setText(const QString& text);
    void setFont(const QFont& font);
    void setColor(const QColor& color);
    QString text() const;
    QFont font() const;
    QColor color() const { return _color; }

    QPlainTextEdit* editor() const { return _editor; }

signals:
    void acceptRequested();

private slots:
    void setFontFamily(const QString& family);
    void setFontSize(const QString& size);

private:
    QPlainTextEdit *_editor;
    QColor _color;
    QAction *_actnBold, *_actnItalic, *_actnUnderline, *_actnColor;
    QFontComboBox *_comboFont;
    QComboBox *_comboSize;

    void toggleBold();
    void toggleItalic();
    void toggleUnderline();
    void selectColor();
    void insertVar();
};

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
