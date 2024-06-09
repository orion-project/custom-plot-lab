#include "qcpl_cursor_panel.h"
#include "qcpl_cursor.h"

#include "widgets/OriValueEdit.h"
#include "helpers/OriDialogs.h"

#include <QAction>
#include <QMenu>
#include <QToolBar>

#define TEXT_SIZE 13
#define TEXT_MARGIN_TOP 8
#define TEXT_MARGIN_BOTTOM 5

namespace QCPL {

CursorPanel::CursorPanel(Cursor *cursor): _cursor(cursor)
{
    QFont font = this->font();
    font.setPixelSize(TEXT_SIZE);
    setFont(font);

    setStyleSheet("margin-left: 3px; margin-right: 1px");

    verticalScrollBar()->setVisible(false);
    horizontalScrollBar()->setVisible(false);
    setOpenLinks(false);
    connect(this, SIGNAL(anchorClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

    connect(_cursor, SIGNAL(positionChanged()), this, SLOT(cursorPositionCanged()));

    createActions();
}

void CursorPanel::createActions()
{
    actnShowCursor = new QAction(this);
    actnShowCursor->setCheckable(true);
    actnShowCursor->setText(tr("Cursor", "Plot action"));
    actnShowCursor->setChecked(true);
    connect(actnShowCursor, SIGNAL(toggled(bool)), _cursor, SLOT(setVisible(bool)));

    actnCursorVert = new QAction(this);
    actnCursorHorz = new QAction(this);
    actnCursorBoth = new QAction(this);
    actnCursorVert->setCheckable(true);
    actnCursorHorz->setCheckable(true);
    actnCursorBoth->setCheckable(true);
    actnCursorVert->setText(tr("Vertical Line", "Plot action"));
    actnCursorHorz->setText(tr("Horizontal Line", "Plot action"));
    actnCursorBoth->setText(tr("Both Lines", "Plot action"));
    connect(actnCursorVert, SIGNAL(triggered()), this, SLOT(setCursorShape()));
    connect(actnCursorHorz, SIGNAL(triggered()), this, SLOT(setCursorShape()));
    connect(actnCursorBoth, SIGNAL(triggered()), this, SLOT(setCursorShape()));
    actnCursorBoth->setChecked(true);

    actnCursorFollow = new QAction(this);
    actnCursorFollow->setText(tr("Follow Mouse", "Plot action"));
    actnCursorFollow->setCheckable(true);
    actnCursorFollow->setShortcut(Qt::Key_F7);
    actnCursorFollow->setIcon(QIcon(":/qcpl_images/plot_tracing"));
    connect(actnCursorFollow, SIGNAL(toggled(bool)), _cursor, SLOT(setFollowMouse(bool)));

    actnCursorSetX = new QAction(this);
    actnCursorSetX->setText(tr("Set Cursor X-position", "Plot action"));
    actnCursorSetX->setShortcut(Qt::Key_X);
    connect(actnCursorSetX, SIGNAL(triggered()), this, SLOT(setCursorX()));
    addAction(actnCursorSetX); // activates shortcut

    actnCursorSetY = new QAction(this);
    actnCursorSetY->setText(tr("Set Cursor Y-position", "Plot action"));
    actnCursorSetY->setShortcut(Qt::Key_Y);
    connect(actnCursorSetY, SIGNAL(triggered()), this, SLOT(setCursorY()));
    addAction(actnCursorSetY); // activates shortcut
}

void CursorPanel::placeIn(class QToolBar* toolbar)
{
    QActionGroup *group = new QActionGroup(this);
    group->addAction(actnCursorVert);
    group->addAction(actnCursorHorz);
    group->addAction(actnCursorBoth);

    QMenu *menu = new QMenu(this);
    menu->addActions(group->actions());

    QToolButton *button = new QToolButton;
    button->setStyleSheet("padding-top: 3px; padding-bottom: 3px; "
                          "padding-left: 12px; padding-right: 12px");
    button->setDefaultAction(actnShowCursor);
    button->setPopupMode(QToolButton::MenuButtonPopup);
    button->setMenu(menu);
    button->setToolTip("");

    toolbar->addWidget(button);
    toolbar->addWidget(this);
    toolbar->addAction(actnCursorFollow);
}

void CursorPanel::fillMenu(QMenu* menu)
{
    menu->addAction(actnShowCursor);
    menu->addSeparator();
    menu->addAction(actnCursorVert);
    menu->addAction(actnCursorHorz);
    menu->addAction(actnCursorBoth);
    menu->addSeparator();
    menu->addAction(actnCursorFollow);
}

QSize CursorPanel::sizeHint() const
{
    return QSize(9999, TEXT_SIZE + TEXT_MARGIN_TOP + TEXT_MARGIN_BOTTOM);
}

QSize CursorPanel::minimumSizeHint() const
{
    return QSize(100, TEXT_SIZE + TEXT_MARGIN_TOP + TEXT_MARGIN_BOTTOM);
}

void CursorPanel::linkClicked(const QUrl& url)
{
    if (url.scheme() == "do")
    {
        QString cmd = url.host();
        if (cmd == "set_x")
            setCursorX();
        else if (cmd == "set_y")
            setCursorY();
        else
            emit customCommandInvoked(cmd);
    }
}

void CursorPanel::cursorPositionCanged()
{
    if (_autoUpdateInfo)
        update();
    if (!_cursor->followMouse()) // cursor was fixed by double click
        actnCursorFollow->setChecked(false);
}

QString CursorPanel::formatLink(const QString& cmd, const QString& val)
{
    return QStringLiteral("<a href='do://%1'>%2</a>").arg(cmd, val);
}

QString CursorPanel::formatLinkX(const QString& x)
{
    return QStringLiteral("X = <a href='do://set_x'>%1</a>").arg(x);
}

QString CursorPanel::formatLinkY(const QString& y)
{
    return QStringLiteral("Y = <a href='do://set_y'>%1</a>").arg(y);
}

QString CursorPanel::formatCursorInfo() const
{
    auto point = _cursor->position();
    return QStringLiteral("%1; %2").arg(
        formatLinkX(QString::number(point.x(), 'g', _numberPrecision)),
        formatLinkY(QString::number(point.y(), 'g', _numberPrecision)));
}

void CursorPanel::update()
{
    setText(formatCursorInfo());
}

void CursorPanel::update(const QString& info)
{
    auto cursorInfo = formatCursorInfo();
    if (!info.isEmpty()) cursorInfo += "; " + info;
    setText(cursorInfo);
}

void CursorPanel::setCursorX()
{
    // TODO: add an ability to set custom format settings
    Ori::Widgets::ValueEdit editor(_cursor->position().x());
    if (Ori::Dlg::Dialog(&editor, false)
            .withHorizontalPrompt("<b>X:</b>")
            .withTitle(tr("Cursor Position"))
            .withActiveWidget(&editor)
            .exec())
        _cursor->setPositionX(editor.value());
}

void CursorPanel::setCursorY()
{
    // TODO: add an ability to set custom format settings
    Ori::Widgets::ValueEdit editor(_cursor->position().y());
    if (Ori::Dlg::Dialog(&editor, false)
            .withHorizontalPrompt("<b>Y:</b>")
            .withTitle(tr("Cursor Position"))
            .withActiveWidget(&editor)
            .exec())
        _cursor->setPositionY(editor.value());
}

void CursorPanel::setCursorShape()
{
    if (actnCursorVert->isChecked())
        _cursor->setShape(Cursor::VerticalLine);
    else if (actnCursorHorz->isChecked())
        _cursor->setShape(Cursor::HorizontalLine);
    else
        _cursor->setShape(Cursor::CrossLines);
}

CursorPanel::Mode CursorPanel::mode() const
{
    if (actnCursorVert->isChecked())
        return Vertical;
    else if (actnCursorHorz->isChecked())
        return Horizontal;
    return Both;
}

void CursorPanel::setMode(CursorPanel::Mode mode)
{
    switch (mode)
    {
    case Both:
        _cursor->setShape(Cursor::CrossLines);
        actnCursorBoth->setChecked(true);
        break;
    case Vertical:
        _cursor->setShape(Cursor::VerticalLine);
        actnCursorVert->setChecked(true);
        break;
    case Horizontal:
        _cursor->setShape(Cursor::HorizontalLine);
        actnCursorHorz->setChecked(true);
        break;
    }
}

bool CursorPanel::enabled() const
{
    return _cursor->visible();
}

void CursorPanel::setEnabled(bool on)
{
    _cursor->setVisible(on);
    actnShowCursor->setChecked(on);
}

void CursorPanel::setNumberPrecision(int value, bool update)
{
    _numberPrecision = value;
    if (update) this->update();
}

} // namespace QCPL
