/* Copyright 2014 CyberTech Labs Ltd.
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

#include <QtCore/qglobal.h>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	#include <QtGui/QMessageBox>
	#include <QtGui/QPushButton>
#else
	#include <QtWidgets/QMessageBox>
	#include <QtWidgets/QPushButton>
#endif

#include <QtCore/QString>

namespace trikGui {

/// Message box that asks user if he/she wants to open or delete a file.
class FileManagerMessageBox : public QMessageBox
{
	Q_OBJECT
public:
	enum class FileState {
		None,
		Open,
		Delete
	};

	/// Constructor.
	/// @param parent - parent of this widget in terms of Qt parent-child widget relations.
	explicit FileManagerMessageBox(QWidget *parent = 0);

	~FileManagerMessageBox() override;

	/// Shows a widget as modal dialog.
	void showMessage();

	/// Returns user selection after dialog is closed.
	FileState userAnswer() const;

protected:
	void keyPressEvent(QKeyEvent *event) override;

private:
	void init();
	void changeDefaultButton();

	QPushButton *mOpenButton;  // Has ownership
	QPushButton *mDeleteButton;  // Has ownership
	bool mEscStatus = false;
};

}

