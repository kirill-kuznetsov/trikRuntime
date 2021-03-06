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

#include <QtCore/QString>
#include <QtCore/QStack>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	#include <QtGui/QWidget>
	#include <QtGui/QVBoxLayout>
	#include <QtGui/QHBoxLayout>
	#include <QtGui/QStackedLayout>
#else
	#include <QtWidgets/QWidget>
	#include <QtWidgets/QVBoxLayout>
	#include <QtWidgets/QHBoxLayout>
	#include <QtWidgets/QStackedLayout>
#endif

#include <trikKernel/lazyMainWidget.h>
#include "controller.h"
#include "batteryIndicator.h"
#include "startWidget.h"
#include "runningWidget.h"

namespace trikGui {

/// TrikGui backround widget which is a parent for other trikGui widgets.
/// It consists of a status bar and a place for one of main widgets which is
/// used for conversation with user.
class BackgroundWidget : public QWidget
{
	Q_OBJECT

public:
	/// Constructor.
	/// @param configPath - path to a directory with config files (config.xml and so on). It is expected to be
	///        ending with "/".
	/// @param startDirPath - path to the directory from which the application was executed (it is expected to be
	///        ending with "/").
	/// @param parent - parent of this widget in terms of Qt parent-child widget relations.
	explicit BackgroundWidget(QString const &configPath, QString const &startDirPath, QWidget *parent = 0);

public slots:
	/// Add a widget to main widgets layout and show it.
	/// @param widget - reference to the widget.
	void addMainWidget(trikKernel::MainWidget &widget);

	/// Add a RunningWidget to main widgets layout and show it.
	/// @param widget - reference to the widget.
	void addRunningWidget(trikKernel::MainWidget &widget);

	/// Add a GraphicsWidget to main widgets layout and show RunningWidget.
	/// @param widget - reference to the widget.
	void addLazyWidget(trikKernel::LazyMainWidget &widget);

private slots:
	void renewFocus();

	/// Updates all widgets to remove clutter.
	void refresh();

	/// Show a widget which is contained in main widgets layout.
	/// @param widget - reference to the widget.
	void showMainWidget(trikKernel::MainWidget &widget);

	/// Show a RunningWidget which is contained in main widgets layout.
	/// @param widget - reference to the widget.
	void showRunningWidget(const QString &fileName, int scriptId);

	void hideRunningWidget(int scriptId);

	void showError(const QString &error, int scriptId);

	void hideGraphicsWidget();

	void hideScriptWidgets();

	void updateStack(int removedWidget);

private:
	/// Remove widget margins.
	/// @param widget - reference to the widget.
	void resetWidgetLayout(trikKernel::MainWidget &widget);

	Controller mController;
	QVBoxLayout mMainLayout;
	QHBoxLayout mStatusBarLayout;
	QStackedLayout mMainWidgetsLayout;
	BatteryIndicator mBatteryIndicator;
	StartWidget mStartWidget;
	RunningWidget mRunningWidget;

	QStack<int> mMainWidgetIndex;
};

}
