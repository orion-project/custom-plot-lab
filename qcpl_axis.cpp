#include "qcpl_axis.h"

#include "qcpl_plot.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriLayouts.h"

#include <QButtonGroup>
#include <QGridLayout>
#include <QFrame>
#include <QRadioButton>
#include <QPushButton>

using namespace Ori::Layouts;

namespace QCPL {

//------------------------------------------------------------------------------
//                                QCPL::Axis
//------------------------------------------------------------------------------

Axis::Axis(QCPAxisRect *parent, AxisType type) : QCPAxis(parent, type)
{
}

void Axis::draw(QCPPainter *painter)
{
    if (_highlight)
    {
        QRect axisRect = this->axisRect()->rect();
        int offset = this->offset();
        auto type = this->axisType();

        QPen pen;
        pen.setWidth(mBasePen.width() + 6);
        pen.setStyle(Qt::SolidLine);
        pen.setColor(QColor(0, 240, 255, 120));
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);

      // The same code for drawing axis as in QCPAxisPainterPrivate::draw
      QPoint origin;
      switch (type)
      {
        case QCPAxis::atLeft:   origin = axisRect.bottomLeft() +QPoint(-offset, 0); break;
        case QCPAxis::atRight:  origin = axisRect.bottomRight()+QPoint(+offset, 0); break;
        case QCPAxis::atTop:    origin = axisRect.topLeft()    +QPoint(0, -offset); break;
        case QCPAxis::atBottom: origin = axisRect.bottomLeft() +QPoint(0, +offset); break;
      }

      double xCor = 0, yCor = 0; // paint system correction, for pixel exact matches (affects baselines and ticks of top/right axes)
      switch (type)
      {
        case QCPAxis::atTop: yCor = -1; break;
        case QCPAxis::atRight: xCor = 1; break;
        default: break;
      }
      // draw baseline:
      QLineF baseLine;
      painter->setPen(pen);
      if (QCPAxis::orientation(type) == Qt::Horizontal)
        baseLine.setPoints(origin+QPointF(xCor, yCor), origin+QPointF(axisRect.width()+xCor, yCor));
      else
        baseLine.setPoints(origin+QPointF(xCor, yCor), origin+QPointF(xCor, -axisRect.height()+yCor));
      painter->drawLine(baseLine);
    }
    QCPAxis::draw(painter);
}

//------------------------------------------------------------------------------
//                              QCPL::chooseAxes
//------------------------------------------------------------------------------

static void highlightAxes(Plot* plot, const AxisPair& pair)
{
    foreach (auto axis, plot->axisRect()->axes())
        if (auto a = dynamic_cast<Axis*>(axis); a)
            a->setHightlight(a == pair.x || a == pair.y);
    plot->replot();
}

class AxesChooser : public QWidget
{
public:
    AxesChooser(Plot* plot, const AxisPair& chosenAxes) : QWidget(), plot(plot), chosenAxes(chosenAxes)
    {
        auto frame = new QFrame;
        frame->setFrameStyle(QFrame::Plain);
        frame->setFrameShape(QFrame::Box);
        frame->setAutoFillBackground(true);
        frame->setBackgroundRole(QPalette::Base);

        xFlags = new QButtonGroup(this);
        yFlags = new QButtonGroup(this);

        auto layout = new QGridLayout(this);
        layout->addWidget(frame, 1, 1);
        layout->addLayout(fillAxes(QCPAxis::atBottom, xFlags), 2, 1);
        layout->addLayout(fillAxes(QCPAxis::atLeft, singleAxis ? xFlags : yFlags), 1, 0);
        layout->addLayout(fillAxes(QCPAxis::atTop, xFlags), 0, 1);
        layout->addLayout(fillAxes(QCPAxis::atRight, singleAxis ? xFlags : yFlags), 1, 2);

        if (hasHiddenAxes) {
            auto iconLabel = new QLabel;
            iconLabel->setFixedWidth(24);
            hiddenMessage = new QLabel,
            hiddenMessage->setWordWrap(true);
            iconLabel->setPixmap(QIcon(":/qcpl_images/eye_closed").pixmap(24));
            layout->setRowMinimumHeight(3, 16);
            layout->addLayout(LayoutH({iconLabel, hiddenMessage}).boxLayout(), 4, 0, 1, 3);
        }
    }

    QVBoxLayout* fillAxes(QCPAxis::AxisType axisType, QButtonGroup* group)
    {
        auto layout = new QVBoxLayout;
        if (axisType == QCPAxis::atTop)
            layout->setDirection(QBoxLayout::BottomToTop);
        foreach (auto axis, plot->axisRect()->axes(axisType))
        {
            auto flag = new QPushButton(plot->axisIdent(axis));
            flag->setCheckable(true);
            flag->setChecked(group == xFlags ? axis == chosenAxes.x : axis == chosenAxes.y);
            flag->connect(flag, &QPushButton::pressed, this, [this, axis]{
                if (axis->orientation() == Qt::Horizontal) {
                    chosenAxes.x = axis;
                    if (singleAxis)
                        chosenAxes.y = nullptr;
                }
                else {
                    chosenAxes.y = axis;
                    if (singleAxis)
                        chosenAxes.x = nullptr;
                }
                highlightAxes(plot, chosenAxes);
            });
            if (!axis->visible()) {
                hasHiddenAxes = true;
                flag->setIcon(QIcon(":/qcpl_images/eye_closed"));
            }
            group->addButton(flag);
            layout->addWidget(flag);
        }
        return layout;
    }

    Plot* plot;
    QButtonGroup *xFlags, *yFlags;
    XYPair<QCPAxis*> chosenAxes;
    bool singleAxis = false;
    bool hasHiddenAxes = false;
    QLabel *hiddenMessage = nullptr;
};

AxisPair chooseAxes(Plot* plot, const AxisPair& chosenAxes)
{
    highlightAxes(plot, chosenAxes);
    AxesChooser w(plot, chosenAxes);
    if (w.hiddenMessage)
        w.hiddenMessage->setText(qApp->translate("AxesChooser",
            "Some axes are invisible, they will be revealed being chosen"));
    bool ok = Ori::Dlg::Dialog(&w, false)
        .withTitle(qApp->translate("AxesChooser", "Choose Axes"))
        .withSkipContentMargins()
        .withButtonsSeparator()
        .exec();
    highlightAxes(plot, {});
    return ok ? w.chosenAxes : chosenAxes;
}

QCPAxis* chooseAxis(Plot* plot)
{
    highlightAxes(plot, {});
    AxesChooser w(plot, {});
    w.singleAxis = true;
    if (w.hiddenMessage)
        w.hiddenMessage->setText(qApp->translate("AxesChooser", "Some axes are invisible"));
    bool ok = Ori::Dlg::Dialog(&w, false)
        .withTitle(qApp->translate("AxesChooser", "Choose an Axis"))
        .withSkipContentMargins()
        .withButtonsSeparator()
        .exec();
    highlightAxes(plot, {});
    return ok ? (w.chosenAxes.x ? w.chosenAxes.x : w.chosenAxes.y) : nullptr;
}

} // namespace QCPL
