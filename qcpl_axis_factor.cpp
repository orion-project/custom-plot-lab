#include "qcpl_axis_factor.h"

#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"
#include "widgets/OriValueEdit.h"

#include <QComboBox>
#include <QGroupBox>
#include <QTimer>

using namespace Ori::Layouts;

namespace QCPL {

//------------------------------------------------------------------------------
//                            QCPL::FactorAxisTicker
//------------------------------------------------------------------------------

FactorAxisTicker::FactorAxisTicker(QSharedPointer<QCPAxisTicker> prevTicker) : QCPAxisTicker(), prevTicker(prevTicker)
{
    setTickStepStrategy(prevTicker->tickStepStrategy());
    setTickCount(prevTicker->tickCount());
    setTickOrigin(prevTicker->tickOrigin());
}

void FactorAxisTicker::generate(const QCPRange &range, const QLocale &locale, QChar formatChar, int precision,
                                QVector<double> &ticks, QVector<double> *subTicks, QVector<QString> *tickLabels)
{
    prevTicker->setTickStepStrategy(tickStepStrategy());
    prevTicker->setTickCount(tickCount());
    prevTicker->setTickOrigin(tickOrigin());

    QCPAxisTicker::generate(range, locale, formatChar, precision, ticks, subTicks, tickLabels);

    if (dynamic_cast<QCPAxisTickerLog*>(prevTicker.get()))
        return;

    QVector<double> factoredTicks(ticks.size());

    double f =  std::holds_alternative<int>(factor)
        ? qPow(10.0, double(std::get<int>(factor)))
        : std::get<double>(factor);
    for (int i = 0; i < ticks.size(); i++)
        factoredTicks[i] = ticks.at(i) / f;

    if (tickLabels)
        *tickLabels = createLabelVector(factoredTicks, locale, formatChar, precision);
}

//------------------------------------------------------------------------------
//                            QCPL::AxisFactorWidget
//------------------------------------------------------------------------------

AxisFactorWidget::AxisFactorWidget(QCPAxis *axis, const Props& props) : QWidget()
{
    _comboMetric = new QComboBox;
    _comboMetric->setEditable(false);
    _comboMetric->addItem(tr("Exa (×1e18)"), 18);
    _comboMetric->addItem(tr("Peta (×1e15)"), 15);
    _comboMetric->addItem(tr("Tera (×1e12)"), 12);
    _comboMetric->addItem(tr("Giga (×1e9)"), 9);
    _comboMetric->addItem(tr("Mega (×1e6)"), 6);
    _comboMetric->addItem(tr("Kilo (×1000)"), 3);
    _comboMetric->addItem(tr("Hecto (×100)"), 2);
    _comboMetric->addItem(tr("Deca (×10)"), 1);
    _comboMetric->addItem(tr("None"), 0);
    _comboMetric->addItem(tr("Deci (×0.1)"), -1);
    _comboMetric->addItem(tr("Centi (×0.01)"), -2);
    _comboMetric->addItem(tr("Milli (×0.001)"), -3);
    _comboMetric->addItem(tr("Micro (×1e-6)"), -6);
    _comboMetric->addItem(tr("Nano (×1e-9)"), -9);
    _comboMetric->addItem(tr("Pico (×1e-12)"), -12);
    _comboMetric->addItem(tr("Femto (×1e-15)"), -15);
    _comboMetric->addItem(tr("Atto (×1e-18)"), -18);

    _groupMetric = LayoutV({_comboMetric}).makeGroupBox(tr("Metric"));
    _groupMetric->setCheckable(true);

    _editorCustom = new Ori::Widgets::ValueEdit;

    _groupCustom = LayoutV({_editorCustom}).makeGroupBox(tr("Custom"));
    _groupCustom->setCheckable(true);

    auto factorTicker = dynamic_cast<FactorAxisTicker*>(axis->ticker().data());
    auto factor = factorTicker ? factorTicker->factor : AxisFactor();
    if (std::holds_alternative<double>(factor))
    {
        _comboMetric->setCurrentIndex(-1);
        _editorCustom->setValue(std::get<double>(factor));
        _groupMetric->setChecked(false);
        _groupCustom->setChecked(true);
    }
    else if (std::holds_alternative<int>(factor))
    {
        int f = std::get<int>(factor);
        Ori::Gui::setSelectedId(_comboMetric, f);
        if (_comboMetric->currentIndex() == -1)
            Ori::Gui::setSelectedId(_comboMetric, 0);
        _editorCustom->setValue(1);
        _groupMetric->setChecked(true);
        _groupCustom->setChecked(false);
    }
    else
    {
        Ori::Gui::setSelectedId(_comboMetric, 0);
        _editorCustom->setValue(1);
        _groupMetric->setChecked(true);
        _groupCustom->setChecked(false);
    }

    connect(_groupMetric, &QGroupBox::toggled, this, [this](bool on){ _groupCustom->setChecked(!on); updateFocus(); });
    connect(_groupCustom, &QGroupBox::toggled, this, [this](bool on){ _groupMetric->setChecked(!on); updateFocus(); });

    LayoutV({_groupMetric, SpaceV(), _groupCustom}).setMargin(0).useFor(this);

    QTimer::singleShot(100, this, &AxisFactorWidget::updateFocus);
}

AxisFactor AxisFactorWidget::selectedFactor() const
{
    if (_groupMetric->isChecked())
        return Ori::Gui::getSelectedId(_comboMetric, 0);
    if (_editorCustom->text().isEmpty())
        return 0;
    return _editorCustom->value();
}

void AxisFactorWidget::updateFocus()
{
    if (_groupMetric->isChecked())
        _comboMetric->setFocus();
    else {
        _editorCustom->setFocus();
        _editorCustom->selectAll();
    }
}

} // namespace QCPL
