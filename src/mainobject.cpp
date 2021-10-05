/****************************************************************************\
   Copyright 2021 Luca Beldi
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
\****************************************************************************/
#include "mainobject.h"
#include "worker.h"
#include <QThread>

MainObject::MainObject(QObject *parent)
    :QObject(parent)
{
    m_workerThread = new QThread(this);
    m_worker = new Worker;
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread,&QThread::finished,m_worker,&QObject::deleteLater);
    connect(m_workerThread,&QThread::started,m_worker,&Worker::init);
    m_workerThread->start();
}

MainObject::~MainObject()
{
    m_workerThread->quit();
    m_workerThread->wait();
}
