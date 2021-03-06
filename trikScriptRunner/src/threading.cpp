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

#include "threading.h"

#include "scriptEngineWorker.h"
#include "src/utils.h"
#include "src/scriptThread.h"

#include <QsLog.h>

using namespace trikScriptRunner;

Threading::Threading(ScriptEngineWorker *scriptWorker, ScriptExecutionControl &scriptControl)
	: QObject(scriptWorker)
	, mResetStarted(false)
	, mScriptWorker(scriptWorker)
	, mScriptControl(scriptControl)
{
}

Threading::~Threading()
{
	reset();
}

void Threading::startMainThread(const QString &script)
{
	mScript = script;
	mErrorMessage.clear();

	QRegExp const mainRegexp("(.*var main\\s*=\\s*\\w*\\s*function\\(.*\\).*)|(.*function\\s+%1\\s*\\(.*\\).*)");
	bool needCallMain = mainRegexp.exactMatch(script) && !script.trimmed().endsWith("main();");

	startThread("main", mScriptWorker->createScriptEngine(), needCallMain ? script + "\nmain();" : script);
}

void Threading::startThread(QScriptValue const &threadId, QScriptValue const &function)
{
	startThread(threadId.toString(), cloneEngine(function.engine()), mScript + "\n" + function.toString() + "();");
}

void Threading::startThread(QString const &threadId, QScriptEngine *engine, QString const &script)
{
	mResetMutex.lock();
	if (mResetStarted) {
		QLOG_INFO() << "Threading: can't start new thread" << threadId << "with engine" << engine << "due to reset";
		delete engine;
		mResetMutex.unlock();
		return;
	}

	mThreadsMutex.lock();
	if (mThreads.contains(threadId)) {
		QLOG_ERROR() << "Threading: attempt to create a thread with an already occupied id" << threadId;
		mErrorMessage = tr("Attempt to create a thread with an already occupied id %1").arg(threadId);
		mThreads[threadId]->abort();
		mThreadsMutex.unlock();
		mResetMutex.unlock();
		return;
	}

	QLOG_INFO() << "Starting new thread" << threadId << "with engine" << engine;
	ScriptThread *thread = new ScriptThread(*this, threadId, engine, script);
	mThreads[threadId] = thread;
	mThreadsMutex.unlock();

	engine->moveToThread(thread);
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	thread->start();

	while (!thread->isEvaluating()) {
		QThread::yieldCurrentThread();
	}

	// wait until script actually start to avoid problems with multiple starts and resets
	// TODO: efficient AND safe solution
	for (int i = 0; i < 500; ++i) {
		QThread::yieldCurrentThread();
	}

	QLOG_INFO() << "Threading: started thread" << threadId << "with engine" << engine << ", thread object" << thread;
	mResetMutex.unlock();
}

void Threading::waitForAll()
{
	while (!mThreads.isEmpty()) {
		QThread::yieldCurrentThread();
	}
}

void Threading::joinThread(const QString &threadId)
{
	mThreadsMutex.lock();

	while ((!mThreads.contains(threadId) || !mThreads[threadId]->isRunning())
			&& !mFinishedThreads.contains(threadId))
	{
		mThreadsMutex.unlock();
		if (mResetStarted) {
			return;
		}

		QThread::yieldCurrentThread();
		mThreadsMutex.lock();
	}

	if (mFinishedThreads.contains(threadId)) {
		mThreadsMutex.unlock();
		return;
	}

	ScriptThread *thread = mThreads[threadId];
	mThreadsMutex.unlock();
	thread->wait();
}

QScriptEngine * Threading::cloneEngine(QScriptEngine *engine)
{
	QScriptEngine *result = mScriptWorker->copyScriptEngine(engine);
	result->evaluate(mScript);
	return result;
}

void Threading::reset()
{
	if (!tryLockReset()) {
		return;
	}

	mResetStarted = true;
	mResetMutex.unlock();
	QLOG_INFO() << "Threading: reset started";

	mMessageMutex.lock();
	for (QWaitCondition * const condition : mMessageQueueConditions.values()) {
		condition->wakeAll();
	}

	mMessageMutex.unlock();

	mThreadsMutex.lock();
	for (ScriptThread *thread : mThreads.values()) {
		mScriptControl.reset();  // TODO: find more sophisticated solution to prevent waiting after abortion
		thread->abort();
	}

	mFinishedThreads.clear();
	mThreadsMutex.unlock();

	mScriptControl.reset();
	waitForAll();

	qDeleteAll(mMessageQueueMutexes);
	qDeleteAll(mMessageQueueConditions);
	mMessageQueueMutexes.clear();
	mMessageQueueConditions.clear();
	mMessageQueues.clear();

	QLOG_INFO() << "Threading: reset ended";
	mResetStarted = false;
}

void Threading::threadFinished(const QString &id)
{
	QLOG_INFO() << "Threading: finishing thread" <<	id;

	mThreadsMutex.lock();
	if (!mThreads[id]->error().isEmpty() && mErrorMessage.isEmpty()) {
		mErrorMessage = mThreads[id]->error();
	}

	QLOG_INFO() << "Threading: thread" << id << "has finished, thread object" << mThreads[id];
	mThreads.remove(id);
	mFinishedThreads.insert(id);
	mThreadsMutex.unlock();

	if (!mErrorMessage.isEmpty()) {
		reset();
	}
}

void Threading::sendMessage(const QString &threadId, const QScriptValue &message)
{
	if (!tryLockReset()) {
		return;
	}

	mMessageMutex.lock();
	if (!mMessageQueueConditions.contains(threadId)) {
		mMessageQueueMutexes[threadId] = new QMutex();
		mMessageQueueConditions[threadId] = new QWaitCondition();
	}

	mMessageQueues[threadId].enqueue(message);
	mMessageQueueConditions[threadId]->wakeOne();
	mMessageMutex.unlock();

	mResetMutex.unlock();
}

QScriptValue Threading::receiveMessage()
{
	if (!tryLockReset()) {
		return QScriptValue();
	}

	QString threadId = static_cast<ScriptThread *>(QThread::currentThread())->id();
	mMessageMutex.lock();
	if (!mMessageQueueConditions.contains(threadId)) {
		mMessageQueueMutexes[threadId] = new QMutex();
		mMessageQueueConditions[threadId] = new QWaitCondition();
	}

	QMutex *mutex = mMessageQueueMutexes[threadId];
	QWaitCondition *condition = mMessageQueueConditions[threadId];
	QQueue<QScriptValue> &queue = mMessageQueues[threadId];
	mMessageMutex.unlock();

	mutex->lock();
	if (queue.isEmpty()) {
		mResetMutex.unlock();
		condition->wait(mutex);
		if (!tryLockReset()) {
			return QScriptValue();
		}
	}

	mutex->unlock();
	QScriptValue result = queue.dequeue();
	mResetMutex.unlock();
	return result;
}

QString Threading::errorMessage() const
{
	return mErrorMessage;
}

bool Threading::tryLockReset()
{
	mResetMutex.lock();
	if (mResetStarted) {
		mResetMutex.unlock();
	}

	return !mResetStarted;
}
