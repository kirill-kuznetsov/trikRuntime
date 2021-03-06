/* Copyright 2013 - 2015 Roman Kurbatov, Yurii Litvinov and CyberTech Labs Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#pragma once

#include <QtCore/QFile>

#include "pwmCaptureInterface.h"
#include "deviceState.h"

namespace trikKernel {
class Configurer;
}

namespace trikControl {

/// Implementation of PWM capture for real robot.
class PwmCapture : public PwmCaptureInterface
{
	Q_OBJECT

public:
	/// Constructor.
	/// @param port - port on which this sensor is configured.
	/// @param configurer - configurer object containing preparsed XML files with sensor parameters.
	PwmCapture(QString const &port, trikKernel::Configurer const &configurer);

	~PwmCapture() override;

	Status status() const override;

public slots:
	/// Returns three readings of PWM signal frequency.
	QVector<int> frequency() override;

	/// Returns PWM signal duty.
	int duty() override;

private:
	QFile mFrequencyFile;
	QFile mDutyFile;
	DeviceState mState;
};

}
