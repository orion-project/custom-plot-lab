#ifndef QCPL_UTILS_H
#define QCPL_UTILS_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QPlainTextEdit;
QT_END_NAMESPACE

namespace QCPL {

struct TitleEditorOptions
{
    QSize iconSize;
};

class TitleEditor : public QWidget
{
    Q_OBJECT

public:
    TitleEditor(const TitleEditorOptions& opts);

    void setText(const QString& text);
    void setFont(const QFont& font);
    void setColor(const QColor& color);
    QString text() const;
    QFont font() const;
    QColor color() const { return _color; }

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
};

} // namespace QCPL

#endif // QCPL_UTILS_H
