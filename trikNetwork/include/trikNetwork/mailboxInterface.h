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

#include <QtCore/QObject>
#include <QtNetwork/QHostAddress>
#include <QtXml/QDomElement>

#include "declSpec.h"

namespace trikNetwork {

/// Allows communication between robots. Each robot has its hull number (assigned in "Settings->Comm settings" in GUI),
/// and can connect to other robot (called "leader") by IP, if they are in the same subnet (it is also can be done
/// in "Settings->Comm settings", or from script by connect() method). Then robot can send messages to every robot
/// that is connected to a leader (or to a robot that is connected to a robot that is connected to a leader, and so on).
/// Recipients are identified by their hull numbers, if two or more robots in a network have the same hull number,
/// they will all receive a message.
class TRIKNETWORK_EXPORT MailboxInterface : public QObject
{
	Q_OBJECT

public:
	/// Sets hull number of this robot and sends a message to update hull number to all known robots.
	virtual void setHullNumber(int myHullNumber) = 0;

	/// Returns IP of a leader robot, or empty QHostAddress if we are not connected.
	virtual QHostAddress serverIp() const = 0;

	/// Returns our IP address, or empty QHostAddress if we are not connected.
	virtual QHostAddress myIp() const = 0;

	/// Stops waiting for messages, clears message queue.
	virtual void reset() = 0;

	/// Returns true if mailbox is enabled in current configuration.
	virtual bool isEnabled() = 0;

public slots:
	/// Connects to robot by IP and port.
	virtual void connect(QString const &ip, int port) = 0;

	/// Connects to robot by IP and uses port of local mailbox server as a port on remote robot.
	virtual void connect(QString const &ip) = 0;

	/// Sends message to a robot with given hull number.
	virtual void send(int hullNumber, QString const &message) = 0;

	/// Sends message to all known robots.
	virtual void send(QString const &message) = 0;

	/// Returns true if there are incoming messages. Returns immediately.
	virtual bool hasMessages() = 0;

	/// Receives and returns one incoming message. If there is already a message in a queue, returns immediately,
	/// otherwise blocks until a message is received. Note that if receive() and handler for newMessage() is used
	/// simultaneously, message will be delivered twice --- first for receive(), then to handler (or handlers).
	virtual QString receive() = 0;

	/// Returns hull number of this robot.
	virtual int myHullNumber() const = 0;

signals:
	/// Emitted when new message is received from a robot with given hull number. Note that if receive() and
	/// handler for newMessage() is used simultaneously, message will be delivered twice --- first for receive(), then
	/// to handler (or handlers).
	void newMessage(int sender, QString const &message);
};

}
