/* Copyright 2015 CyberTech Labs Ltd.
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

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>

class QDomElement;

namespace trikKernel {

/// Generic configuration helper, parses configuration XML files and presents configuration as a set of attributes
/// with their values for each configurable device. Respects configuration overriding rules:
/// Model config > device type config > device-port pair config > device class config
class Configurer
{
public:
	/// Constructor.
	/// @param systemConfig - file name (with path) of system config, absolute or relative to current directory.
	/// @param modelConfig - file name (with path) of model config, absolute or relative to current directory.
	Configurer(QString const &systemConfig, QString const &modelConfig);

	/// Returns value of given attribute of given device.
	QString attributeByDevice(QString const &deviceType, QString const &attributeName) const;

	/// Returns value of given attribute of a device on given port.
	QString attributeByPort(QString const &port, QString const &attributeName) const;

	/// Returns true if device is enabled in current configuration (either explicitly enabled in model configuration
	/// or can not be disabled at all).
	bool isEnabled(QString const deviceName) const;

	/// Ports configured in model config.
	QStringList ports() const;

	/// Returns class of a device configured on given port.
	QString deviceClass(QString const &port) const;

	/// Returns init scripts defined in config files, first from system config then from model config.
	QStringList initScripts() const;

private:
	struct Device {
		QString name;
		QHash<QString, QString> attributes;
		QHash<QString, QHash<QString, QString>> portSpecificAttributes;
		bool isOptional = false;
	};

	struct DeviceType {
		QString name;
		QString deviceClass;
		QHash<QString, QString> attributes;
	};

	struct ModelConfigurationElement {
		QString port;
		QString deviceType;
		QHash<QString, QString> attributes;
	};

	struct AdditionalModelConfigurationElement {
		QString deviceType;
		QHash<QString, QString> attributes;
	};

	void parseDeviceClasses(QDomElement const &element);
	void parseDevicePorts(QDomElement const &element);
	void parseDeviceTypes(QDomElement const &element);
	void parseInitScript(QDomElement const &element);
	void parseAdditionalConfigurations(QDomElement const &element);
	void parseModelConfig(QDomElement const &element);

	QStringList mInitScripts;

	/// Maps device class name to its configuration.
	QHash<QString, Device> mDevices;

	/// Maps device type name to its configuration.
	QHash<QString, DeviceType> mDeviceTypes;

	/// Maps device class name to its configuration.
	QHash<QString, Device> mAdditionalConfiguration;

	/// Maps port name to configuration of device on that port.
	QHash<QString, ModelConfigurationElement> mModelConfiguration;

	/// Maps device type name to its configuration.
	QHash<QString, AdditionalModelConfigurationElement> mAdditionalModelConfiguration;
};

}
