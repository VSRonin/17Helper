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
#include <QNetworkRequest>
#include <QSet>
#include <QMultiHash>
#include "mtgahcard.h"
#include "seventeencard.h"
class QNetworkAccessManager;
class Worker : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Worker)
public:
    explicit Worker(QObject* parent = nullptr);
    QMultiHash<QString,MtgahCard> *ratingsTemplate();
public slots:
    void tryLogin(const QString& userName, const QString& password);
    void logOut();
    void downloadSetsMTGAH();
    void downloadSetsScryfall();
    void getCustomRatingTemplate();
    void get17LRatings(const QStringList& sets, const QString& format);
private slots:
    void processSLrequestQueue();
signals:
    void loggedIn();
    void loginFalied();
    void loggedOut();
    void logoutFailed();
    void downloadSetsMTGAHFailed();
    void setsMTGAH(const QStringList& sets);
    void downloadSetsScryfallFailed();
    void customRatingTemplateFailed();
    void customRatingTemplate(const QMultiHash<QString,MtgahCard>& sets);
    void setsScryfall(const QHash<QString,QString>& sets);
    void failed17LRatings();
    void downloaded17LRatings(const QString& set, const QSet<SeventeenCard>& ratings);
    void downloadedAll17LRatings();

private:
    QList<std::pair<QString,QNetworkRequest> > m_SLrequestQueue;
    QMultiHash<QString,MtgahCard> m_ratingsTemplate;
    QNetworkAccessManager *m_nam;
    int m_SLrequestOutstanding;
};

#endif
