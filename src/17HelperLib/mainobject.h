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
#include <QSet>
#include "17helperlib_global.h"
#include "globals.h"
class Worker;
class QThread;
class QStandardItemModel;
class QAbstractItemModel;
class SetsModel;
class SLRatingsModel;
class CheckableProxy;
class RatingsModel;
class QSortFilterProxyModel;
class SetsFilterModel;
class CustomRatingModel;
class ConfigManager;
class SLMetricsFilterModel;
class SHLIB_EXPORT MainObject : public QObject
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
    QAbstractItemModel *customRatingsModel() const;
    void filterRatings(QString name);
    void showOnlyDraftableSets(bool showOnly);
    void showOnlySLRatios(bool showOnly);
    bool oneSetSelected() const;
    bool oneNonDraftableSetSelected() const;
    QString configPath() const;
    QStringList failedUploadCards() const;
    QString setFullName(const QString &setCode) const;
public slots:
    void tryLogin(const QString &userName, const QString &password, bool rememberMe = false);
    void logOut();
    void retranslateModels();
    void download17Lands(const QString &format);
    void download17Lands(const QString &format, GEnums::RatingTimeScale scale, int period);
    void download17Lands(const QString &format, const QDate &fromDate, const QDate &toDate);
    void uploadMTGAH(GEnums::SLMetrics ratingMethod, const QLocale &locale, bool clear);
    void cancelUpload();
    void downloadSetsMTGAH();
    void getCustomRatingTemplate();
    void setAllSetsSelection(Qt::CheckState check);
    void setAllSLMetricsSelection(Qt::CheckState check);
    void fetchLoginInfos();
    void fetchDownloadData();
    void fetchUploadData();
private slots:
    void onWorkerInit();
    void onInitialisationFailed();
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
    void onRatingUploadFailed(const QString &card);
    void onCustomRatingTemplateFailed();
    void onDownloadSetsScryfallFailed();
    void onDownloadSetsMTGAHFailed();
signals:
    void loggedIn();
    void loginFalied(const QString &error);
    void loggedOut();
    void logoutFailed(const QString &error);
    void initialised();
    void initialisationFailed();
    void customRatingTemplate();
    void customRatingTemplateFailed();
    void setsReady();
    void startProgress(MainObject::Operations op, const QString &description, int max, int min);
    void updateProgress(MainObject::Operations op, int val);
    void increaseProgress(MainObject::Operations op, int val);
    void endProgress(MainObject::Operations op);
    void SLDownloadFinished();
    void SLDownloadFailed();
    void no17LRating(const QStringList &sets);
    void ratingsCalculated();
    void ratingsCalculationFailed();
    void ratingUploaded(const QString &card);
    void ratingUploadFailed(const QString &card);
    void ratingsUploaded();
    void loadUserPass(const QString &userName, const QString &password);
    void loadDownloadFormat(const QString &format, GEnums::RatingTimeMethod timeMethod, const QDate &from, const QDate &to,
                            GEnums::RatingTimeScale timeScale, int timeSpan);
    void loadUploadRating(GEnums::SLMetrics ratingBase);
    void setsMTGAHDownloaded();
    void downloadSetsMTGAHFailed();
    void downloadSetsScryfallFailed();
    void showOnlyDraftableSetsChanged(bool showOnlyDraftable);
    void showOnlySLRatiosChanged(bool showOnlyRatios);

private:
    void actualDownload17Lands(const QString &format, const QStringList &sets, const QDate &fromDate, const QDate &toDate);
    void init();
    void fillMetrics();
    void fillFormats();
    void selectSetsModel();
    Worker *m_worker;
    ConfigManager *m_configManager;
    QThread *m_workerThread;
    const QString m_objectDbName;
    QStringList SLcodes;
    QStandardItemModel *m_SLMetricsModel;
    SLMetricsFilterModel *m_SLMetricsProxy;
    QStandardItemModel *m_formatsModel;
    SetsModel *m_setsModel;
    SetsFilterModel *m_setsFilter;
    CheckableProxy *m_setsProxy;
    RatingsModel *m_ratingTemplateModel;
    SLRatingsModel *m_SLratingsModel;
    QSortFilterProxyModel *m_ratingTemplateProxy;
    QSortFilterProxyModel *m_SLratingsProxy;
    CustomRatingModel *m_customRatingsModel;
    int ratingsToUpload;
    QSet<QString> m_failedUploadCards;
    std::pair<QStringList, QStringList> getSetsForDownload() const;
};

#endif