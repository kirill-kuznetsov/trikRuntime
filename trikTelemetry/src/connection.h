/* Copyright 2014 - 2015 CyberTech Labs Ltd.
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

#include <trikNetwork/connection.h>
#include <trikControl/brickInterface.h>
#include <trikNetwork/gamepadInterface.h>

namespace trikTelemetry {

/// Connection class accepts requests for sensors configuration and current sensor values. Uses a brick
/// passed to it as a constructor parameter to respond to those requests.
///
/// Accepted commands:
///     data - sends data from sensors to a client
///     ports - sends current ports configuration to a client
class Connection : public trikNetwork::Connection
{
	Q_OBJECT

public:
	/// Constructor.
	/// @param brick - a Brick used to respond to clients.
	/// @param gamepad - gamepad object used to report state of Android gamepad.
	explicit Connection(trikControl::BrickInterface &brick, trikNetwork::GamepadInterface &gamepad);

private:
	void processData(QByteArray const &data) override;

	static QString serializeVector(QVector<int> const &vector);

	bool isButtonPressed(QString const &buttonName);

	/// A Brick which is used by Connections to respond to clients' requests
	trikControl::BrickInterface &mBrick;

	/// A Gamepad object which is used by Connections to respond to requests about gamepad state.
	trikNetwork::GamepadInterface &mGamepad;
};

}
