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
#include "globals.h"
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
    enum { MaximumUploadAttemps = 3 };

public:
    explicit Worker(QObject *parent = nullptr);

public slots:
    void init();
    void tryLogin(const QString &userName, const QString &password);
    void logOut();
    void downloadSetsMTGAH();
    void getCustomRatingTemplate();
    void get17LRatings(const QStringList &sets, const QString &format);
    void uploadRatings(const QStringList &sets, GEnums::SLMetrics ratingMethod, const QVector<GEnums::SLMetrics> &commentStats,
                       const QStringList &SLcodes, const QLocale &locale);
    void clearRatings(const QStringList &sets, GEnums::SLMetrics ratingMethod, const QVector<GEnums::SLMetrics> &commentStats,
                      const QStringList &SLcodes, const QLocale &locale);
    void cancelUpload();
private slots:
    void processSLrequestQueue();
    void processMTGAHrequestQueue();
    void checkStopTimer();
    void parseSetsScryfall(QNetworkReply *reply, const QStringList &sets);
    void onCustomRatingTemplateFinished(QNetworkReply *reply);
    void on17LDownloadFinished(QNetworkReply *reply, const QString &currSet);
    void onSetsMTGAHDownloaded(QNetworkReply *reply);
    void onLogIn(QNetworkReply *reply);
    void onLogOut(QNetworkReply *reply);
    void onRatingUploaded(QNetworkReply *reply, const QJsonObject &reqData);
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
    void actualUploadRatings(QStringList sets, GEnums::SLMetrics ratingMethod, QVector<GEnums::SLMetrics> commentStats, const QStringList &SLcodes,
                             const QLocale &locale, bool clear);
    static int ratingValue(GEnums::SLMetrics metric, const double &val, const QVector<double> &deciles);
    static QMap<QString, QVector<double>> reduceDeciles(const QMap<QString, QVector<double>> &data);
    QString commentvalue(GEnums::SLMetrics metric, const QVariant &value, const QLocale &locale) const;
    QString commentString(const QSqlQuery &query, const QVector<GEnums::SLMetrics> &commentStats, const QStringList &SLcodes,
                          const QLocale &locale) const;
    QString fieldName(GEnums::SLMetrics metric) const;
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
    std::atomic_bool m_cancelUpload;
};

#endif
