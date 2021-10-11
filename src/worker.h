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
#include "mtgahcard.h"
#include "seventeencard.h"
#include <QMultiHash>
#include <QNetworkRequest>
#include <QObject>
#include <QSet>
#include <QSqlDatabase>
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;
class Worker : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Worker)
    enum { RequestTimerTimeout = 200 };

public:
    enum SetType {
        stcore = 0x1,
        stexpansion = 0x2,
        stmasters = 0x4,
        stmasterpiece = 0x8,
        stfrom_the_vault = 0x10,
        stspellbook = 0x20,
        stpremium_deck = 0x40,
        stduel_deck = 0x80,
        stdraft_innovation = 0x100,
        sttreasure_chest = 0x200,
        stcommander = 0x400,
        stplanechase = 0x800,
        starchenemy = 0x1000,
        stvanguard = 0x2000,
        stfunny = 0x4000,
        ststarter = 0x8000,
        stbox = 0x10000,
        stpromo = 0x20000,
        sttoken = 0x40000,
        stmemorabilia = 0x80000,

        stEnd = 0x100000
    };
    explicit Worker(QObject *parent = nullptr);
public slots:
    void init();
    void tryLogin(const QString &userName, const QString &password);
    void logOut();
    void downloadSetsMTGAH();

    void getCustomRatingTemplate();
    void get17LRatings(const QStringList &sets, const QString &format);
    void uploadRatings(const QStringList &sets);
private slots:
    void processSLrequestQueue();
    void processMTGAHrequestQueue();
    void checkStopTimer();
    void parseSetsScryfall(QNetworkReply *reply, const QStringList &sets);

signals:
    void initialised();
    void initialisationFailed();
    void loggedIn();
    void loginFalied(const QString &error);
    void loggedOut();
    void logoutFailed(const QString &error);
    void downloadSetsMTGAHFailed();
    void setsMTGAH(bool needsUpdate);
    void downloadSetsScryfallFailed();
    void customRatingTemplateFailed();
    void setsScryfall(bool needsUpdate);
    void failed17LRatings();
    void downloadedAll17LRatings();
    void download17LRatingsProgress(int progress);
    void allRatingsUploaded();
    void ratingUploaded(const QString &card);
    void ratingsUploadMaxProgress(int progress);
    void ratingsUploadProgress(int progress);
    void failedUploadRating(const MtgahCard &card);
    void downloaded17LRatings(const QString &set, const QSet<SeventeenCard> &ratings);

private:
    void saveMTGAHSets(QStringList sets);
    void downloadSetsScryfall();
    int setTypeCode(const QString &setType) const;
    QList<std::pair<QString, QNetworkRequest>> m_SLrequestQueue;
    QList<std::pair<MtgahCard, QNetworkRequest>> m_MTGAHrequestQueue;
    QNetworkAccessManager *m_nam;
    int m_SLrequestOutstanding;
    int m_MTGAHrequestOutstanding;
    const QString m_workerDbName;
    QSqlDatabase openWorkerDb();
    QTimer *m_requestTimer;
};

#endif
