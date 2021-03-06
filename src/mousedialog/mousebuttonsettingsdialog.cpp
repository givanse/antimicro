#include "mousebuttonsettingsdialog.h"
#include "ui_mousesettingsdialog.h"
#include <QSpinBox>
#include <QCheckBox>

#include <inputdevice.h>
#include <setjoystick.h>

MouseButtonSettingsDialog::MouseButtonSettingsDialog(JoyButton *button, QWidget *parent) :
    MouseSettingsDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);
    resize(size().width(), 450);
    //setGeometry(geometry().x(), geometry().y(), size().width(), 450);

    this->button = button;

    calculateMouseSpeedPreset();
    selectCurrentMouseModePreset();
    calculateSpringPreset();

    if (button->getSensitivity() > 0.0)
    {
        ui->sensitivityDoubleSpinBox->setValue(button->getSensitivity());
    }

    updateAccelerationCurvePresetComboBox();
    updateWindowTitleButtonName();

    if (ui->mouseModeComboBox->currentIndex() == 2)
    {
        springPreviewWidget = new SpringModeRegionPreview(ui->springWidthSpinBox->value(), ui->springHeightSpinBox->value());
    }
    else
    {
        springPreviewWidget = new SpringModeRegionPreview(0, 0);
    }

    ui->wheelHoriSpeedSpinBox->setValue(button->getWheelSpeedX());
    ui->wheelVertSpeedSpinBox->setValue(button->getWheelSpeedY());

    if (button->isRelativeSpring())
    {
        ui->relativeSpringCheckBox->setChecked(true);
    }

    double easingDuration = button->getEasingDuration();
    ui->easingDoubleSpinBox->setValue(easingDuration);

    if (button->isPartRealAxis())
    {
        ui->extraAccelerationGroupBox->setChecked(button->isExtraAccelerationEnabled());
        ui->extraAccelDoubleSpinBox->setValue(button->getExtraAccelerationMultiplier());
        ui->minMultiDoubleSpinBox->setValue(button->getStartAccelMultiplier());
        ui->minThresholdDoubleSpinBox->setValue(button->getMinAccelThreshold());
        ui->maxThresholdDoubleSpinBox->setValue(button->getMaxAccelThreshold());
        ui->accelExtraDurationDoubleSpinBox->setValue(button->getAccelExtraDuration());
    }
    else
    {
        ui->extraAccelerationGroupBox->setVisible(false);
    }

    ui->releaseSpringRadiusspinBox->setValue(button->getSpringDeadCircleMultiplier());

    changeSpringSectionStatus(ui->mouseModeComboBox->currentIndex());
    changeSettingsWidgetStatus(ui->accelerationComboBox->currentIndex());

    connect(this, SIGNAL(finished(int)), springPreviewWidget, SLOT(deleteLater()));

    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseMode(int)));
    connect(ui->accelerationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseCurve(int)));

    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigHorizontalSpeed(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigVerticalSpeed(int)));

    connect(ui->springWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringWidth(int)));
    connect(ui->springWidthSpinBox, SIGNAL(valueChanged(int)), springPreviewWidget, SLOT(setSpringWidth(int)));

    connect(ui->springHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringHeight(int)));
    connect(ui->springHeightSpinBox, SIGNAL(valueChanged(int)), springPreviewWidget, SLOT(setSpringHeight(int)));

    connect(ui->relativeSpringCheckBox, SIGNAL(clicked(bool)), this, SLOT(updateSpringRelativeStatus(bool)));

    connect(ui->sensitivityDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSensitivity(double)));

    connect(ui->wheelHoriSpeedSpinBox, SIGNAL(valueChanged(int)), button, SLOT(setWheelSpeedX(int)));
    connect(ui->wheelVertSpeedSpinBox, SIGNAL(valueChanged(int)), button, SLOT(setWheelSpeedY(int)));

    connect(ui->easingDoubleSpinBox, SIGNAL(valueChanged(double)), button, SLOT(setEasingDuration(double)));

    connect(ui->extraAccelerationGroupBox, SIGNAL(clicked(bool)), this, SLOT(updateExtraAccelerationStatus(bool)));
    connect(ui->extraAccelDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateExtraAccelerationMultiplier(double)));
    connect(ui->minMultiDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateStartMultiPercentage(double)));
    connect(ui->minThresholdDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateMinAccelThreshold(double)));
    connect(ui->maxThresholdDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateMaxAccelThreshold(double)));
    connect(ui->accelExtraDurationDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateAccelExtraDuration(double)));

    connect(ui->releaseSpringRadiusspinBox, SIGNAL(valueChanged(int)), this, SLOT(updateReleaseSpringRadius(int)));
}

void MouseButtonSettingsDialog::changeMouseMode(int index)
{
    if (index == 1)
    {
        button->setMouseMode(JoyButton::MouseCursor);
        if (springPreviewWidget->isVisible())
        {
            springPreviewWidget->hide();
        }
    }
    else if (index == 2)
    {
        button->setMouseMode(JoyButton::MouseSpring);
        if (!springPreviewWidget->isVisible())
        {
            springPreviewWidget->setSpringWidth(ui->springWidthSpinBox->value());
            springPreviewWidget->setSpringHeight(ui->springHeightSpinBox->value());
        }

        if (button->isPartRealAxis())
        {
            button->setExtraAccelerationStatus(false);
        }
    }
}

void MouseButtonSettingsDialog::changeMouseCurve(int index)
{
    JoyButton::JoyMouseCurve temp = MouseSettingsDialog::getMouseCurveForIndex(index);
    button->setMouseCurve(temp);
}

void MouseButtonSettingsDialog::updateConfigHorizontalSpeed(int value)
{
    button->setMouseSpeedX(value);
}

void MouseButtonSettingsDialog::updateConfigVerticalSpeed(int value)
{
    button->setMouseSpeedY(value);
}

void MouseButtonSettingsDialog::updateSpringWidth(int value)
{
    button->setSpringWidth(value);
}

void MouseButtonSettingsDialog::updateSpringHeight(int value)
{
    button->setSpringHeight(value);
}

void MouseButtonSettingsDialog::selectCurrentMouseModePreset()
{
    JoyButton::JoyMouseMovementMode mode = button->getMouseMode();
    if (mode == JoyButton::MouseCursor)
    {
        ui->mouseModeComboBox->setCurrentIndex(1);
    }
    else if (mode == JoyButton::MouseSpring)
    {
        ui->mouseModeComboBox->setCurrentIndex(2);
    }
}

void MouseButtonSettingsDialog::calculateSpringPreset()
{
    int tempWidth = button->getSpringWidth();
    int tempHeight = button->getSpringHeight();

    if (tempWidth > 0)
    {
        ui->springWidthSpinBox->setValue(tempWidth);
    }

    if (tempHeight > 0)
    {
        ui->springHeightSpinBox->setValue(tempHeight);
    }
}

void MouseButtonSettingsDialog::calculateMouseSpeedPreset()
{
    int tempMouseSpeedX = button->getMouseSpeedX();
    int tempMouseSpeedY = button->getMouseSpeedY();

    ui->horizontalSpinBox->setValue(tempMouseSpeedX);
    ui->verticalSpinBox->setValue(tempMouseSpeedY);
}

void MouseButtonSettingsDialog::updateSensitivity(double value)
{
    button->setSensitivity(value);
}

void MouseButtonSettingsDialog::updateAccelerationCurvePresetComboBox()
{
    JoyButton::JoyMouseCurve temp = button->getMouseCurve();
    MouseSettingsDialog::updateAccelerationCurvePresetComboBox(temp);
}

/*void MouseButtonSettingsDialog::updateSmoothingSetting(bool clicked)
{
    button->setSmoothing(clicked);
}
*/

/*void MouseButtonSettingsDialog::selectSmoothingPreset()
{
    bool smoothing = button->isSmoothingEnabled();
    if (smoothing)
    {
        ui->smoothingCheckBox->setChecked(true);
    }
    else
    {
        ui->smoothingCheckBox->setChecked(false);
    }
}
*/

void MouseButtonSettingsDialog::updateSpringRelativeStatus(bool value)
{
    button->setSpringRelativeStatus(value);
}

void MouseButtonSettingsDialog::updateWindowTitleButtonName()
{
    QString temp;
    temp.append(tr("Mouse Settings - ")).append(button->getPartialName(false, true));


    if (button->getParentSet()->getIndex() != 0)
    {
        unsigned int setIndex = button->getParentSet()->getRealIndex();
        temp.append(" [").append(tr("Set %1").arg(setIndex));

        QString setName = button->getParentSet()->getName();
        if (!setName.isEmpty())
        {
            temp.append(": ").append(setName);
        }

        temp.append("]");
    }

    setWindowTitle(temp);
}

void MouseButtonSettingsDialog::updateExtraAccelerationStatus(bool checked)
{
    button->setExtraAccelerationStatus(checked);
}

void MouseButtonSettingsDialog::updateExtraAccelerationMultiplier(double value)
{
    button->setExtraAccelerationMultiplier(value);
}

void MouseButtonSettingsDialog::updateStartMultiPercentage(double value)
{
    button->setStartAccelMultiplier(value);
}

void MouseButtonSettingsDialog::updateMinAccelThreshold(double value)
{
    button->setMinAccelThreshold(value);
}

void MouseButtonSettingsDialog::updateMaxAccelThreshold(double value)
{
    button->setMaxAccelThreshold(value);
}

void MouseButtonSettingsDialog::updateAccelExtraDuration(double value)
{
    button->setAccelExtraDuration(value);
}

void MouseButtonSettingsDialog::updateReleaseSpringRadius(int value)
{
    button->setSpringDeadCircleMultiplier(value);
}
