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

#ifndef WORKER_H
#define WORKER_H
#include <QObject>
class QNetworkAccessManager;
class Worker : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Worker)
public:
    explicit Worker(QObject* parent = nullptr);
public slots:
    void tryLogin(const QString& userName, const QString& password);
    void downloadSetsMTGAH();
    void downloadSetsScryfall(const QStringList& sets);
signals:
    void loggedIn();
    void loginFalied();
    void downloadSetsMTGAHFailed();
    void setsMTGAH(const QStringList& sets);
    void downloadSetsScryfallFailed();
    void setsScryfall(const QHash<QString,QString>& sets);
private:
    QNetworkAccessManager *m_nam;
};

#endif
