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
#include "globals.h"
#include "worker.h"
#include "configmanager.h"
#include "checkableproxy.h"
#include "ratingsmodel.h"
#include <QThread>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlError>
#include <QSortFilterProxyModel>
#include "slratingsmodel.h"
#include "setsmodel.h"
#include "setsfiltermodel.h"
#include "customratingmodel.h"
//#define DEBUG_SINGLE_THREAD
MainObject::MainObject(QObject *parent)
    : QObject(parent)
    , m_workerThread(nullptr)
    , m_objectDbName(QStringLiteral("ObjectDb"))
    , ratingsToUpload(0)
{
    m_SLMetricsModel = new QStandardItemModel(GEnums::SLCount, 1, this);
    fillMetrics();
    m_formatsModel = new QStandardItemModel(dfCount, 1, this);
    fillFormats();
    QSqlDatabase objectDb = openDb(m_objectDbName);
    m_SLratingsModel = new SLRatingsModel(this);
    m_SLratingsProxy = new QSortFilterProxyModel(this);
    m_SLratingsProxy->setSourceModel(m_SLratingsModel);
    m_setsModel = new SetsModel(this);
    m_setsFilter = new SetsFilterModel(this);
    m_setsFilter->setSourceModel(m_setsModel);
    m_setsFilter->setFilterEnabled(true);
    m_setsProxy = new CheckableProxy(this);
    m_setsProxy->setSourceModel(m_setsFilter);
    m_ratingTemplateModel = new RatingsModel(this);
    m_ratingTemplateProxy = new QSortFilterProxyModel(this);
    m_ratingTemplateProxy->setSourceModel(m_ratingTemplateModel);
    m_customRatingsModel = new CustomRatingModel(this);
    m_configManager = new ConfigManager(this);
    QMetaObject::invokeMethod(this, std::bind(&MainObject::startProgress, this, opInitWorker, tr("Initialising"), 0, 0), Qt::QueuedConnection);

    m_worker = new Worker;
#ifndef DEBUG_SINGLE_THREAD
    m_workerThread = new QThread(this);
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::started, m_worker, &Worker::init);
#endif
    connect(m_worker, &Worker::initialised, this, &MainObject::onWorkerInit);
    connect(m_worker, &Worker::initialisationFailed, this, &MainObject::onInitialisationFailed);
    connect(m_worker, &Worker::loggedIn, this, &MainObject::onLoggedIn);
    connect(m_worker, &Worker::loginFalied, this, &MainObject::onLoginFalied);
    connect(m_worker, &Worker::loggedOut, this, &MainObject::onLoggedOut);
    connect(m_worker, &Worker::logoutFailed, this, &MainObject::onLoginFalied);
    connect(m_worker, &Worker::setsScryfall, this, &MainObject::onSetsScryfall);
    connect(m_worker, &Worker::downloadSetsScryfallFailed, this, &MainObject::onDownloadSetsScryfallFailed);
    connect(m_worker, &Worker::setsMTGAH, this, &MainObject::onSetsMTGAH);
    connect(m_worker, &Worker::downloadSetsMTGAHFailed, this, &MainObject::onDownloadSetsMTGAHFailed);
    connect(m_worker, &Worker::customRatingTemplate, this, &MainObject::onRatingsTemplate);
    connect(m_worker, &Worker::customRatingTemplateFailed, this, &MainObject::onCustomRatingTemplateFailed);
    connect(m_worker, &Worker::downloadedAll17LRatings, this, &MainObject::on17LandsDownloadFinished);
    connect(m_worker, &Worker::downloaded17LRatings, this, &MainObject::on17LandsSetDownload);
    connect(m_worker, &Worker::failed17LRatings, this, &MainObject::on17LandsDownloadError);
    connect(m_worker, &Worker::ratingsCalculated, this, &MainObject::onRatingsCalculated);
    connect(m_worker, &Worker::ratingCalculated, this, &MainObject::onRatingCalculated);
    connect(m_worker, &Worker::failedRatingCalculation, this, &MainObject::onRatingsCalculationFailed);
    connect(m_worker, &Worker::allRatingsUploaded, this, &MainObject::onAllRatingsUploaded);
    connect(m_worker, &Worker::ratingUploaded, this, &MainObject::onRatingUploaded);
    connect(m_worker, &Worker::failedUploadRating, this, &MainObject::onFailedUploadRating);
#ifndef DEBUG_SINGLE_THREAD
    m_workerThread->start();
#else
    m_worker->init();
#endif
}

MainObject::~MainObject()
{
#ifndef DEBUG_SINGLE_THREAD
    m_workerThread->quit();
    m_workerThread->wait();
#endif
}

QAbstractItemModel *MainObject::SLMetricsModel() const
{
    return m_SLMetricsModel;
}

QAbstractItemModel *MainObject::setsModel() const
{
    return m_setsProxy;
}

QAbstractItemModel *MainObject::formatsModel() const
{
    return m_formatsModel;
}

QAbstractItemModel *MainObject::ratingsModel() const
{
    return m_ratingTemplateProxy;
}

QAbstractItemModel *MainObject::seventeenLandsRatingsModel() const
{
    return m_SLratingsProxy;
}

QAbstractItemModel *MainObject::customRatingsModel() const
{
    return m_customRatingsModel;
}

void MainObject::filterRatings(QString name)
{
    QStringList setsMTGAH;
    QStringList setsSL;
    for (int i = 0, iEnd = m_setsProxy->rowCount(); i != iEnd; ++i) {
        if (m_setsProxy->index(i, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked) {
            setsMTGAH.append(m_setsProxy->index(i, SetsModel::smcSetID).data().toString());
            setsSL.append(m_setsProxy->index(i, SetsModel::smcParentSet).data().toString());
        }
    }
    Q_ASSERT(setsMTGAH.size() == setsSL.size());
    if (setsMTGAH.isEmpty() && name.isEmpty())
        return m_ratingTemplateModel->setFilter(QString());
    QString filterStringMTGAH;
    QString filterStringSL;
    QSqlDriver *driver = openDb(m_objectDbName).driver();
    if (!setsMTGAH.isEmpty()) {
        for (int i = 0; i < setsMTGAH.size(); ++i) {
            setsMTGAH[i] = driver->escapeIdentifier(setsMTGAH.at(i), QSqlDriver::FieldName);
            setsSL[i] = driver->escapeIdentifier(setsSL.at(i), QSqlDriver::FieldName);
        }
        filterStringMTGAH = QLatin1String("[set] in (") + setsMTGAH.join(QLatin1Char(',')) + QLatin1Char(')');
        filterStringSL = QLatin1String("[set] in (") + setsSL.join(QLatin1Char(',')) + QLatin1Char(')');
    }
    if (!name.isEmpty()) {
        for (QString *filterString : {&filterStringMTGAH, &filterStringSL}) {
            if (!filterString->isEmpty())
                *filterString += QLatin1String(" AND ");
            *filterString +=
                    QLatin1String("[name] like ") + driver->escapeIdentifier(QLatin1Char('%') + name + QLatin1Char('%'), QSqlDriver::FieldName);
        }
    }
    m_customRatingsModel->setFilter(filterStringMTGAH);
    m_ratingTemplateModel->setFilter(filterStringMTGAH);
    m_SLratingsModel->setFilter(filterStringSL);
}

void MainObject::showOnlyDraftableSets(bool showOnly)
{
    m_setsFilter->setFilterEnabled(showOnly);
}

bool MainObject::oneSetSelected() const
{
    for (int i = 0, iEnd = m_setsProxy->rowCount(); i < iEnd; ++i) {
        if (m_setsProxy->index(i, 0).data(Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked)
            return true;
    }
    return false;
}

QString MainObject::configPath() const
{
    return m_configManager->configFilePath();
}

void MainObject::tryLogin(const QString &userName, const QString &password, bool rememberMe)
{
    emit startProgress(opLogIn, tr("Logging in"), 1, 0);
    m_worker->tryLogin(userName, password);
    Q_ASSUME(m_configManager->writeUserPass(rememberMe ? userName : QString(), rememberMe ? password : QString()));
}

void MainObject::logOut()
{
    emit startProgress(opLogOut, tr("Logging out"), 1, 0);
    m_worker->logOut();
}

void MainObject::retranslateModels()
{
    m_formatsModel->setData(m_formatsModel->index(dfPremierDraft, 0), tr("Premier Draft"));
    m_formatsModel->setData(m_formatsModel->index(dfQuickDraft, 0), tr("Quick Draft"));
    m_formatsModel->setData(m_formatsModel->index(dfTradDraft, 0), tr("Traditional Draft"));
    m_formatsModel->setData(m_formatsModel->index(dfDraftChallenge, 0), tr("Draft Challenge"));
    m_formatsModel->setData(m_formatsModel->index(dfSealed, 0), tr("Sealed"));
    m_formatsModel->setData(m_formatsModel->index(dfTradSealed, 0), tr("Traditional Sealed"));

    if (SLcodes.isEmpty()) {
        SLcodes.reserve(GEnums::SLCount);
        for (int i = 0; i < GEnums::SLCount; ++i)
            SLcodes.append(QString());
    }
    Q_ASSERT(SLcodes.size() == GEnums::SLCount);
    SLcodes[GEnums::SLseen_count] = tr("#S");
    SLcodes[GEnums::SLavg_seen] = tr("ALSA");
    SLcodes[GEnums::SLpick_count] = tr("#P");
    SLcodes[GEnums::SLavg_pick] = tr("ATA");
    SLcodes[GEnums::SLgame_count] = tr("#GP");
    SLcodes[GEnums::SLwin_rate] = tr("GPWR");
    SLcodes[GEnums::SLopening_hand_game_count] = tr("#OH");
    SLcodes[GEnums::SLopening_hand_win_rate] = tr("OHWR");
    SLcodes[GEnums::SLdrawn_game_count] = tr("#GD");
    SLcodes[GEnums::SLdrawn_win_rate] = tr("GDWR");
    SLcodes[GEnums::SLever_drawn_game_count] = tr("#GIH");
    SLcodes[GEnums::SLever_drawn_win_rate] = tr("GIHWR");
    SLcodes[GEnums::SLnever_drawn_game_count] = tr("#GND");
    SLcodes[GEnums::SLnever_drawn_win_rate] = tr("GNDWR");
    SLcodes[GEnums::SLdrawn_improvement_win_rate] = tr("IWD");

    QStringList translatedSLCodes;
    translatedSLCodes.reserve(GEnums::SLCount);
    for (int i = 0; i < GEnums::SLCount; ++i)
        translatedSLCodes.append(QString());
    translatedSLCodes[GEnums::SLseen_count] = tr("Number Seen (%1)");
    translatedSLCodes[GEnums::SLavg_seen] = tr("Average Last Seen At (%1)");
    translatedSLCodes[GEnums::SLpick_count] = tr("Number Picked (%1)");
    translatedSLCodes[GEnums::SLavg_pick] = tr("Average Taken At (%1)");
    translatedSLCodes[GEnums::SLgame_count] = tr("Number of Games Played (%1)");
    translatedSLCodes[GEnums::SLwin_rate] = tr("Games Played Win Rate (%1)");
    translatedSLCodes[GEnums::SLopening_hand_game_count] = tr("Number of Games in Opening Hand (%1)");
    translatedSLCodes[GEnums::SLopening_hand_win_rate] = tr("Opening Hand Win Rate (%1)");
    translatedSLCodes[GEnums::SLdrawn_game_count] = tr("Number of Games Drawn (%1)");
    translatedSLCodes[GEnums::SLdrawn_win_rate] = tr("Games Drawn Win Rate (%1)");
    translatedSLCodes[GEnums::SLever_drawn_game_count] = tr("Number of Games In Hand (%1)");
    translatedSLCodes[GEnums::SLever_drawn_win_rate] = tr("Games in Hand Win Rate (%1)");
    translatedSLCodes[GEnums::SLnever_drawn_game_count] = tr("Number of Games Not Drawn (%1)");
    translatedSLCodes[GEnums::SLnever_drawn_win_rate] = tr("Games Not Drawn Win Rate (%1)");
    translatedSLCodes[GEnums::SLdrawn_improvement_win_rate] = tr("Improvement When Drawn (%1)");
    for (int i = 0; i < GEnums::SLCount; ++i)
        translatedSLCodes[i] = translatedSLCodes.at(i).arg(SLcodes.at(i));
    m_SLratingsModel->setSLcodes(translatedSLCodes);
    for (int i = 0, iEnd = m_SLMetricsModel->rowCount(); i < iEnd; ++i)
        m_SLMetricsModel->item(i)->setData(translatedSLCodes.at(i), Qt::DisplayRole);
}

void MainObject::download17Lands(const QString &format)
{
    if (format.isEmpty())
        return;
    QStringList sets;
    QStringList setsToSave;
    for (int i = 0, iEnd = m_setsProxy->rowCount(); i < iEnd; ++i) {
        if (m_setsProxy->index(i, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked) {
            sets.append(m_setsProxy->index(i, SetsModel::smcParentSet).data().toString());
            setsToSave.append(m_setsProxy->index(i, SetsModel::smcSetID).data().toString());
        }
    }
    if (sets.isEmpty())
        return;
    std::sort(sets.begin(), sets.end());
    sets.erase(std::unique(sets.begin(), sets.end()), sets.end());
    m_configManager->writeDataToDownload(format, setsToSave);
    emit startProgress(opDownload17Ratings, tr("Downloading 17Lands Data"), sets.size(), 0);
    m_worker->get17LRatings(sets, format);
}

void MainObject::uploadMTGAH(GEnums::SLMetrics ratingMethod, const QLocale &locale, bool clear)
{
    QStringList sets;
    for (int i = 0, iEnd = m_setsProxy->rowCount(); i < iEnd; ++i) {
        if (m_setsProxy->index(i, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
            sets.append(m_setsProxy->index(i, 1).data().toString());
    }
    if (sets.isEmpty())
        return;
    QVector<GEnums::SLMetrics> commentMetrics;
    for (int i = 0, iEnd = m_SLMetricsModel->rowCount(); i < iEnd; ++i) {
        const QModelIndex currIdx = m_SLMetricsModel->index(i, 0);
        if (currIdx.data(Qt::CheckStateRole).toInt() == Qt::Checked)
            commentMetrics.append(currIdx.data(Qt::UserRole).value<GEnums::SLMetrics>());
    }
    QSqlDatabase objectDb = openDb(m_objectDbName);
    QStringList setsEscaped = sets;
    for (auto i = setsEscaped.begin(), iEnd = setsEscaped.end(); i != iEnd; ++i)
        *i = objectDb.driver()->escapeIdentifier(*i, QSqlDriver::FieldName);
    QString setsQueryString;
    if (clear) {
        setsQueryString = QLatin1String("SELECT COUNT([id_arena]) FROM [Ratings] WHERE [set] in (");
    } else {
        setsQueryString = QLatin1String("SELECT COUNT([id_arena]) FROM [Ratings] left join [Sets] on [Ratings].[set]=[Sets].[id] "
                                        "left join [SLRatings] on [SLRatings].[name]=[Ratings].[name] and [SLRatings].[set]=CASE WHEN "
                                        "[Sets].[parent_set] IS NULL THEN [Sets].[id] ELSE [Sets].[parent_set] end "
                                        "WHERE  [seen_count] NOT NULL AND [Ratings].[set] in (");
    }
    setsQueryString += setsEscaped.join(QLatin1Char(',')) + QLatin1Char(')');
    QSqlQuery setsQuery(objectDb);
    setsQuery.prepare(setsQueryString);
    Q_ASSUME(setsQuery.exec());
    int resultSize = 0;
    if (setsQuery.next())
        resultSize = setsQuery.value(0).toInt();
    if (resultSize <= 0)
        return;
    ratingsToUpload = resultSize;
    emit startProgress(opCalculateRatings, tr("Formatting Ratings"), ratingsToUpload, 0);
    if (clear)
        m_worker->clearRatings(sets, ratingMethod, commentMetrics, SLcodes, locale);
    else
        m_worker->uploadRatings(sets, ratingMethod, commentMetrics, SLcodes, locale);
}

void MainObject::cancelUpload()
{
    m_worker->cancelUpload();
}

void MainObject::onWorkerInit()
{
    m_SLratingsModel->setTable(m_objectDbName);
    m_ratingTemplateModel->setTable(m_objectDbName);
    m_customRatingsModel->setQuery(m_objectDbName);
    selectSetsModel();
    QMetaObject::invokeMethod(this, &MainObject::init, Qt::QueuedConnection);
    emit endProgress(opInitWorker);
    downloadSetsMTGAH();
}
void MainObject::downloadSetsMTGAH()
{
    emit startProgress(opDownloadSets, tr("Loading Sets"), 0, 0);
    emit startProgress(opDownloadSetsData, tr("Downloading Set Details"), 0, 0);
    m_worker->downloadSetsMTGAH();
}

void MainObject::onInitialisationFailed()
{
    emit endProgress(opInitWorker);
    emit initialisationFailed();
}

void MainObject::init()
{
    std::pair<QString, QString> userPass = m_configManager->readUserPass();
    if (!userPass.first.isEmpty() && !userPass.second.isEmpty())
        emit loadUserPass(userPass.first, userPass.second);
    std::pair<QString, QStringList> downloadData = m_configManager->readDataToDownload();
    if (!downloadData.first.isEmpty())
        emit loadDownloadFormat(downloadData.first);
    if (!downloadData.second.isEmpty()) {
        for (int i = 0, iEnd = m_setsProxy->rowCount(); i < iEnd; ++i) {
            if (downloadData.second.contains(m_setsProxy->index(i, SetsModel::smcParentSet).data().toString()))
                m_setsProxy->setData(m_setsProxy->index(i, 0), Qt::Checked, Qt::CheckStateRole);
            else
                m_setsProxy->setData(m_setsProxy->index(i, 0), Qt::Unchecked, Qt::CheckStateRole);
        }
    }
    std::pair<GEnums::SLMetrics, QList<GEnums::SLMetrics>> uploadData = m_configManager->readDataToUpload();
    if (uploadData.first != GEnums::SLCount)
        emit loadUploadRating(uploadData.first);
    if (!uploadData.second.isEmpty()) {
        for (int i = 0, iEnd = m_SLMetricsModel->rowCount(); i < iEnd; ++i) {
            const QModelIndex currIdx = m_SLMetricsModel->index(i, 0);
            if (uploadData.second.contains(currIdx.data(Qt::UserRole).value<GEnums::SLMetrics>()))
                m_SLMetricsModel->setData(currIdx, Qt::Checked, Qt::CheckStateRole);
            else
                m_SLMetricsModel->setData(currIdx, Qt::Unchecked, Qt::CheckStateRole);
        }
    }
}

void MainObject::fillMetrics()
{
    for (int i = 0; i < GEnums::SLCount; ++i) {
        QStandardItem *item = new QStandardItem;
        item->setData(i, Qt::UserRole);
        if (i == GEnums::SLdrawn_win_rate || i == GEnums::SLavg_pick)
            item->setData(Qt::Checked, Qt::CheckStateRole);
        else
            item->setData(Qt::Unchecked, Qt::CheckStateRole);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        m_SLMetricsModel->setItem(i, 0, item);
    }
}

void MainObject::fillFormats()
{
    m_formatsModel->setData(m_formatsModel->index(dfPremierDraft, 0), QStringLiteral("PremierDraft"), Qt::UserRole);
    m_formatsModel->setData(m_formatsModel->index(dfQuickDraft, 0), QStringLiteral("QuickDraft"), Qt::UserRole);
    m_formatsModel->setData(m_formatsModel->index(dfTradDraft, 0), QStringLiteral("TradDraft"), Qt::UserRole);
    m_formatsModel->setData(m_formatsModel->index(dfDraftChallenge, 0), QStringLiteral("DraftChallenge"), Qt::UserRole);
    m_formatsModel->setData(m_formatsModel->index(dfSealed, 0), QStringLiteral("Sealed"), Qt::UserRole);
    m_formatsModel->setData(m_formatsModel->index(dfTradSealed, 0), QStringLiteral("TradSealed"), Qt::UserRole);
}

void MainObject::selectSetsModel()
{
    QSqlDatabase objectDb = openDb(m_objectDbName);
    m_setsModel->setQuery(objectDb);
    m_setsProxy->setData(m_setsProxy->index(0, 0), Qt::Checked, Qt::CheckStateRole);
}

void MainObject::onLoggedIn()
{
    emit endProgress(opLogIn);
    emit loggedIn();
    getCustomRatingTemplate();
}

void MainObject::getCustomRatingTemplate()
{
    emit startProgress(opDownloadRatingTemplate, tr("Downloading custom ratings from MTGA Helper"), 0, 0);
    m_worker->getCustomRatingTemplate();
}

void MainObject::onLoggedOut()
{
    emit endProgress(opLogOut);
    emit loggedOut();
}

void MainObject::onLogoutFalied(const QString &error)
{
    emit endProgress(opLogOut);
    emit logoutFailed(error);
}

void MainObject::onLoginFalied(const QString &error)
{
    emit endProgress(opLogIn);
    emit loginFalied(error);
}

void MainObject::onSetsScryfall(bool needsUpdate)
{
    emit endProgress(opDownloadSetsData);
    if (needsUpdate)
        selectSetsModel();
}

void MainObject::onDownloadSetsScryfallFailed()
{
    emit endProgress(opDownloadSetsData);
    emit downloadSetsMTGAHFailed();
}

void MainObject::onDownloadSetsMTGAHFailed()
{
    emit endProgress(opDownloadSets);
    emit downloadSetsScryfallFailed();
}

void MainObject::onSetsMTGAH(bool needsUpdate)
{
    emit endProgress(opDownloadSets);
    emit setsMTGAHDownloaded();
    if (needsUpdate)
        selectSetsModel();
}

void MainObject::onRatingsTemplate(bool needsUpdate)
{
    emit endProgress(opDownloadRatingTemplate);
    if (needsUpdate)
        m_ratingTemplateModel->select();
}
void MainObject::onCustomRatingTemplateFailed()
{
    emit endProgress(opDownloadRatingTemplate);
    emit customRatingTemplateFailed();
}

void MainObject::on17LandsSetDownload(const QString &set)
{
    Q_UNUSED(set)
    emit increaseProgress(opDownload17Ratings, 1);
}

void MainObject::on17LandsDownloadFinished()
{
    m_SLratingsModel->select();
    emit endProgress(opDownload17Ratings);
    emit SLDownloadFinished();
}

void MainObject::on17LandsDownloadError()
{
    emit endProgress(opDownload17Ratings);
    emit SLDownloadFailed();
}

void MainObject::onRatingsCalculated()
{
    emit endProgress(opCalculateRatings);
    emit ratingsCalculated();
    emit startProgress(opUploadMTGAH, tr("Uploading Data to MTGA Helper"), ratingsToUpload, 0);
}

void MainObject::onRatingCalculated(const QString &card)
{
    Q_UNUSED(card)
    emit increaseProgress(opCalculateRatings, 1);
}

void MainObject::onRatingsCalculationFailed()
{
    emit endProgress(opCalculateRatings);
    emit ratingsCalculationFailed();
}

void MainObject::onAllRatingsUploaded()
{
    emit ratingsUploaded();
    emit endProgress(opUploadMTGAH);
    emit startProgress(opDownloadRatingTemplate, tr("Refreshing Ratings"), 0, 0);
}

void MainObject::onRatingUploaded(const QString &card)
{
    Q_UNUSED(card)
    emit increaseProgress(opUploadMTGAH, 1);
    emit ratingUploaded(card);
}

void MainObject::onFailedUploadRating()
{
    emit endProgress(opUploadMTGAH);
    emit failedUploadRating();
}
