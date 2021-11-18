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

#ifndef MAINOBJECT_H
#define MAINOBJECT_H
#include <QObject>
#include "worker.h"
class QThread;
class QStandardItemModel;
class QAbstractItemModel;
class QSqlQueryModel;
class SLRatingsModel;
class CheckableProxy;
class RatingsModel;
class QSortFilterProxyModel;
class MainObject : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MainObject)
public:
    explicit MainObject(QObject *parent = nullptr);
    ~MainObject();

    enum DraftFormats {
        dfPremierDraft,
        dfQuickDraft,
        dfTradDraft,
        dfDraftChallenge,
        dfSealed,
        dfTradSealed,

        dfCount
    };
    enum { DraftableSet = Worker::stcore | Worker::stexpansion | Worker::stdraft_innovation };
    enum Operations {
        opNoOperation = 0,
        opInitWorker = 1,
        opDownloadSets,
        opDownloadSetsData,
        opLogIn,
        opLogOut,
        opDownloadRatingTemplate,
        opDownload17Ratings,
        opCalculateRatings,
        opUploadMTGAH
    };

    QAbstractItemModel *SLMetricsModel() const;
    QAbstractItemModel *setsModel() const;
    QAbstractItemModel *formatsModel() const;
    QAbstractItemModel *ratingsModel() const;
    QAbstractItemModel *seventeenLandsRatingsModel() const;
    void filterRatings(QString name, QStringList sets);
public slots:
    void tryLogin(const QString &userName, const QString &password, bool rememberMe = false);
    void logOut();
    void retranslateModels();
    void download17Lands(const QString &format);
    void uploadMTGAH(Worker::SLMetrics ratingMethod, const QLocale &locale, bool clear);
private slots:
    void onWorkerInit();
    void onLoggedIn();
    void onLoginFalied(const QString &error);
    void onLoggedOut();
    void onLogoutFalied(const QString &error);
    void onSetsScryfall(bool needsUpdate);
    void onSetsMTGAH(bool needsUpdate);
    void onRatingsTemplate(bool needsUpdate);
    void on17LandsSetDownload(const QString &set);
    void on17LandsDownloadFinished();
    void on17LandsDownloadError();
    void onRatingsCalculated();
    void onRatingCalculated(const QString &card);
    void onRatingsCalculationFailed();
    void onAllRatingsUploaded();
    void onRatingUploaded(const QString &card);
    void onFailedUploadRating();
    void onCustomRatingTemplateFailed();
signals:
    void loggedIn();
    void loginFalied(const QString &error);
    void loggedOut();
    void logoutFailed(const QString &error);
    void initialisationFailed();
    void customRatingTemplateFailed();
    void setsReady();
    void startProgress(Operations op, const QString &description, int max, int min);
    void updateProgress(Operations op, int val);
    void increaseProgress(Operations op, int val);
    void endProgress(Operations op);
    void SLDownloadFinished();
    void SLDownloadFailed();
    void ratingsCalculationFailed();
    void failedUploadRating();

private:
    // double ratingValue(const SeventeenCard &card, SLMetrics method) const;
    // QString commentString(const SeventeenCard &card, const QLocale &locale) const;
    void fillMetrics();
    void fillFormats();
    void selectSetsModel();
    Worker *m_worker;
    QThread *m_workerThread;
    const QString m_objectDbName;
    QStringList SLcodes;
    QStandardItemModel *m_SLMetricsModel;
    QStandardItemModel *m_formatsModel;
    QSqlQueryModel *m_setsModel;
    CheckableProxy *m_setsProxy;
    RatingsModel *m_ratingTemplateModel;
    SLRatingsModel *m_SLratingsModel;
    QSortFilterProxyModel *m_ratingTemplateProxy;
    QSortFilterProxyModel *m_SLratingsProxy;
    int ratingsToUpload;
};

#endif
