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
#include <QMultiHash>
#include <QNetworkRequest>
#include <QObject>
#include <QSet>
#include <QSqlDatabase>
#include <QQueue>
#include <QJsonObject>
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;
class Worker : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Worker)
    enum { RequestTimerTimeout = 100 };

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
    enum SLMetrics {
        SLseen_count,
        SLavg_seen,
        SLpick_count,
        SLavg_pick,
        SLgame_count,
        SLwin_rate,
        SLopening_hand_game_count,
        SLopening_hand_win_rate,
        SLdrawn_game_count,
        SLdrawn_win_rate,
        SLever_drawn_game_count,
        SLever_drawn_win_rate,
        SLnever_drawn_game_count,
        SLnever_drawn_win_rate,
        SLdrawn_improvement_win_rate,

        SLCount
    };
    explicit Worker(QObject *parent = nullptr);

public slots:
    void init();
    void tryLogin(const QString &userName, const QString &password);
    void logOut();
    void downloadSetsMTGAH();
    void getCustomRatingTemplate();
    void get17LRatings(const QStringList &sets, const QString &format);
    void uploadRatings(const QStringList &sets, SLMetrics ratingMethod, const QVector<SLMetrics> &commentStats, const QStringList &SLcodes,
                       const QLocale &locale);
    void clearRatings(const QStringList &sets, SLMetrics ratingMethod, const QVector<SLMetrics> &commentStats, const QStringList &SLcodes,
                      const QLocale &locale);
private slots:
    void processSLrequestQueue();
    void processMTGAHrequestQueue();
    void checkStopTimer();
    void parseSetsScryfall(QNetworkReply *reply, const QStringList &sets);
    void onCustomRatingTemplateFinished(QNetworkReply *reply);
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
    void downloaded17LRatings(const QString &set);
    void allRatingsUploaded();
    void ratingUploaded(const QString &card);
    void failedUploadRating();
    void customRatingTemplate(bool needsUpdate);
    void ratingsCalculated();
    void ratingCalculated(const QString &card);
    void failedRatingCalculation();

private:
    void actualInit();
    void actualTryLogin(const QString &userName, const QString &password);
    void actualLogOut();
    void actualDownloadSetsMTGAH();
    void actualGetCustomRatingTemplate();
    void actualGet17LRatings(const QStringList &sets, const QString &format);
    void actualUploadRatings(QStringList sets, SLMetrics ratingMethod, QVector<SLMetrics> commentStats, const QStringList &SLcodes,
                             const QLocale &locale, bool clear);
    static int ratingValue(SLMetrics metric, const double &val, const QVector<double> &deciles);
    static QMap<QString, QVector<double>> reduceDeciles(const QMap<QString, QVector<double>> &data);
    QString commentvalue(SLMetrics metric, const QVariant &value, const QLocale &locale) const;
    QString commentString(const QSqlQuery &query, const QVector<SLMetrics> &commentStats, const QStringList &SLcodes, const QLocale &locale) const;
    QString fieldName(SLMetrics metric) const;
    void saveMTGAHSets(QStringList sets);
    void downloadSetsScryfall();
    int setTypeCode(const QString &setType) const;
    QQueue<std::pair<QString, QNetworkRequest>> m_SLrequestQueue;
    QQueue<QJsonObject> m_MTGAHrequestQueue;
    QNetworkAccessManager *m_nam;
    int m_SLrequestOutstanding;
    int m_MTGAHrequestOutstanding;
    const QString m_workerDbName;
    QSqlDatabase openWorkerDb();
    QTimer *m_requestTimer;
};
Q_DECLARE_METATYPE(Worker::SLMetrics)
#endif
