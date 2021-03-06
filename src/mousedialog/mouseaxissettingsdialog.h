#ifndef MOUSEAXISSETTINGSDIALOG_H
#define MOUSEAXISSETTINGSDIALOG_H

#include "mousesettingsdialog.h"
#include "springmoderegionpreview.h"

#include <joyaxis.h>


class MouseAxisSettingsDialog : public MouseSettingsDialog
{
    Q_OBJECT
public:
    explicit MouseAxisSettingsDialog(JoyAxis *axis, QWidget *parent = 0);

protected:
    void selectCurrentMouseModePreset();
    void calculateSpringPreset();
    void calculateMouseSpeedPreset();
    //void selectSmoothingPreset();
    void calculateWheelSpeedPreset();
    void updateWindowTitleAxisName();

    void calculateExtraAccelrationStatus();
    void calculateExtraAccelerationMultiplier();
    void calculateStartAccelerationMultiplier();
    void calculateMinAccelerationThreshold();
    void calculateMaxAccelerationThreshold();
    void calculateAccelExtraDuration();
    void calculateReleaseSpringRadius();

    JoyAxis *axis;
    SpringModeRegionPreview *springPreviewWidget;

signals:
    
public slots:
    void changeMouseMode(int index);
    void changeMouseCurve(int index);
    void updateConfigHorizontalSpeed(int value);
    void updateConfigVerticalSpeed(int value);
    void updateSpringWidth(int value);
    void updateSpringHeight(int value);
    void updateSensitivity(double value);
    void updateAccelerationCurvePresetComboBox();
    //void updateSmoothingSetting(bool clicked);

    void updateWheelSpeedHorizontalSpeed(int value);
    void updateWheelSpeedVerticalSpeed(int value);
    void updateSpringRelativeStatus(bool value);
    void updateExtraAccelerationStatus(bool checked);
    void updateExtraAccelerationMultiplier(double value);

private slots:
    void updateStartMultiPercentage(double value);
    void updateMinAccelThreshold(double value);
    void updateMaxAccelThreshold(double value);
    void updateAccelExtraDuration(double value);
    void updateReleaseSpringRadius(int value);
};

#endif // MOUSEAXISSETTINGSDIALOG_H
