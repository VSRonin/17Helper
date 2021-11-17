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
//#define DEBUG_SINGLE_THREAD
MainObject::MainObject(QObject *parent)
    : QObject(parent)
    , m_workerThread(nullptr)
    , m_objectDbName(QStringLiteral("ObjectDb"))
    , ratingsToUpload(0)
{
    m_SLMetricsModel = new QStandardItemModel(Worker::SLCount, 1, this);
    fillMetrics();
    m_formatsModel = new QStandardItemModel(dfCount, 1, this);
    fillFormats();
    QSqlDatabase objectDb = openDb(m_objectDbName);
    m_SLratingsModel = new QSqlTableModel(this, objectDb);
    m_setsModel = new QSqlQueryModel(this);
    m_setsProxy = new CheckableProxy(this);
    m_setsProxy->setSourceModel(m_setsModel);
    m_ratingTemplateModel = new RatingsModel(this, objectDb);
    QMetaObject::invokeMethod(this, std::bind(&MainObject::startProgress, this, opInitWorker, tr("Initialising"), 0, 0), Qt::QueuedConnection);
    m_worker = new Worker;
#ifndef DEBUG_SINGLE_THREAD
    m_workerThread = new QThread(this);
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::started, m_worker, &Worker::init);
#endif
    connect(m_worker, &Worker::initialised, this, &MainObject::onWorkerInit);
    connect(m_worker, &Worker::initialisationFailed, this, &MainObject::initialisationFailed);
    connect(m_worker, &Worker::loggedIn, this, &MainObject::onLoggedIn);
    connect(m_worker, &Worker::loginFalied, this, &MainObject::onLoginFalied);
    connect(m_worker, &Worker::loggedOut, this, &MainObject::onLoggedOut);
    connect(m_worker, &Worker::logoutFailed, this, &MainObject::onLoginFalied);
    connect(m_worker, &Worker::setsScryfall, this, &MainObject::onSetsScryfall);
    connect(m_worker, &Worker::setsMTGAH, this, &MainObject::onSetsMTGAH);
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
    return m_ratingTemplateModel;
}

QAbstractItemModel *MainObject::SLRatingsModel() const
{
    return m_SLratingsModel;
}

void MainObject::filterRatings(QString name, QStringList sets)
{
    if (sets.isEmpty() && name.isEmpty())
        return m_ratingTemplateModel->setFilter(QString());
    QString filterString;
    QSqlDriver *driver = openDb(m_objectDbName).driver();
    if (!sets.isEmpty()) {
        for (int i = 0; i < sets.size(); ++i)
            sets[i] = driver->escapeIdentifier(sets.at(i), QSqlDriver::FieldName);
        filterString = QLatin1String("[set] in (") + sets.join(QLatin1Char(',')) + QLatin1Char(')');
    }
    if (!name.isEmpty()) {
        if (!filterString.isEmpty())
            filterString += QLatin1String(" AND ");
        filterString += QLatin1String("[name] like ") + driver->escapeIdentifier(QLatin1Char('%') + name + QLatin1Char('%'), QSqlDriver::FieldName);
    }
    m_ratingTemplateModel->setFilter(filterString);
    m_SLratingsModel->setFilter(filterString);
}

void MainObject::tryLogin(const QString &userName, const QString &password, bool rememberMe)
{
    emit startProgress(opLogIn, tr("Logging in"), 1, 0);
    m_worker->tryLogin(userName, password);
    if (rememberMe) {
        const QString configPath = appSettingsPath();
        QJsonObject configObject;
        QFile configFile(configPath + QDir::separator() + QLatin1String("17helperconfig.json"));
        if (configFile.exists()) {
            Q_ASSUME(configFile.open(QIODevice::ReadOnly));
            configObject = QJsonDocument::fromJson(configFile.readAll()).object();
            configFile.close();
        }
        configObject[QLatin1String("username")] = userName;
        configObject[QLatin1String("password")] = password;
        Q_ASSUME(configFile.open(QIODevice::WriteOnly));
        configFile.write(QJsonDocument(configObject).toJson(QJsonDocument::Indented));
    }
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
        SLcodes.reserve(Worker::SLCount);
        for (int i = 0; i < Worker::SLCount; ++i)
            SLcodes.append(QString());
    }
    Q_ASSERT(SLcodes.size() == Worker::SLCount);
    SLcodes[Worker::SLseen_count] = tr("#S");
    SLcodes[Worker::SLavg_seen] = tr("ALSA");
    SLcodes[Worker::SLpick_count] = tr("#P");
    SLcodes[Worker::SLavg_pick] = tr("ATA");
    SLcodes[Worker::SLgame_count] = tr("#GP");
    SLcodes[Worker::SLwin_rate] = tr("GPWR");
    SLcodes[Worker::SLopening_hand_game_count] = tr("#OH");
    SLcodes[Worker::SLopening_hand_win_rate] = tr("OHWR");
    SLcodes[Worker::SLdrawn_game_count] = tr("#GD");
    SLcodes[Worker::SLdrawn_win_rate] = tr("GDWR");
    SLcodes[Worker::SLever_drawn_game_count] = tr("#GIH");
    SLcodes[Worker::SLever_drawn_win_rate] = tr("GIHWR");
    SLcodes[Worker::SLnever_drawn_game_count] = tr("#GND");
    SLcodes[Worker::SLnever_drawn_win_rate] = tr("GNDWR");
    SLcodes[Worker::SLdrawn_improvement_win_rate] = tr("IWD");

    m_SLMetricsModel->item(Worker::SLseen_count)->setData(tr("Number Seen (%1)").arg(SLcodes.at(Worker::SLseen_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLavg_seen)->setData(tr("Average Last Seen At (%1)").arg(SLcodes.at(Worker::SLavg_seen)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLpick_count)->setData(tr("Number Picked (%1)").arg(SLcodes.at(Worker::SLpick_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLavg_pick)->setData(tr("Average Taken At (%1)").arg(SLcodes.at(Worker::SLavg_pick)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLgame_count)->setData(tr("Number of Games Played (%1)").arg(SLcodes.at(Worker::SLgame_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLwin_rate)->setData(tr("Games Played Win Rate (%1)").arg(SLcodes.at(Worker::SLwin_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLopening_hand_game_count)
            ->setData(tr("Number of Games in Opening Hand (%1)").arg(SLcodes.at(Worker::SLopening_hand_game_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLopening_hand_win_rate)
            ->setData(tr("Opening Hand Win Rate (%1)").arg(SLcodes.at(Worker::SLopening_hand_win_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLdrawn_game_count)
            ->setData(tr("Number of Games Drawn (%1)").arg(SLcodes.at(Worker::SLdrawn_game_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLdrawn_win_rate)
            ->setData(tr("Games Drawn Win Rate (%1)").arg(SLcodes.at(Worker::SLdrawn_win_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLever_drawn_game_count)
            ->setData(tr("Number of Games In Hand (%1)").arg(SLcodes.at(Worker::SLever_drawn_game_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLever_drawn_win_rate)
            ->setData(tr("Games in Hand Win Rate (%1)").arg(SLcodes.at(Worker::SLever_drawn_win_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLnever_drawn_game_count)
            ->setData(tr("Number of Games Not Drawn (%1)").arg(SLcodes.at(Worker::SLnever_drawn_game_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLnever_drawn_win_rate)
            ->setData(tr("Games Not Drawn Win Rate (%1)").arg(SLcodes.at(Worker::SLnever_drawn_win_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(Worker::SLdrawn_improvement_win_rate)
            ->setData(tr("Improvement When Drawn (%1)").arg(SLcodes.at(Worker::SLdrawn_improvement_win_rate)), Qt::DisplayRole);
}

void MainObject::download17Lands(const QString &format)
{
    if (format.isEmpty())
        return;
    QStringList sets;
    for (int i = 0, iEnd = m_setsProxy->rowCount(); i < iEnd; ++i) {
        if (m_setsProxy->index(i, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
            sets.append(m_setsProxy->index(i, 1).data().toString());
    }
    if (sets.isEmpty())
        return;
    emit startProgress(opDownload17Ratings, tr("Downloading 17Lands Data"), sets.size(), 0);
    m_worker->get17LRatings(sets, format);
}

void MainObject::uploadMTGAH(Worker::SLMetrics ratingMethod, const QLocale &locale, bool clear)
{
    QStringList sets;
    for (int i = 0, iEnd = m_setsProxy->rowCount(); i < iEnd; ++i) {
        if (m_setsProxy->index(i, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
            sets.append(m_setsProxy->index(i, 1).data().toString());
    }
    if (sets.isEmpty())
        return;
    QVector<Worker::SLMetrics> commentMetrics;
    for (int i = 0, iEnd = m_SLMetricsModel->rowCount(); i < iEnd; ++i) {
        const QModelIndex currIdx = m_SLMetricsModel->index(i, 0);
        if (currIdx.data(Qt::CheckStateRole).toInt() == Qt::Checked)
            commentMetrics.append(currIdx.data(Qt::UserRole).value<Worker::SLMetrics>());
    }
    QSqlDatabase objectDb = openDb(m_objectDbName);
    QStringList setsEscaped = sets;
    for (auto i = setsEscaped.begin(), iEnd = setsEscaped.end(); i != iEnd; ++i)
        *i = objectDb.driver()->escapeIdentifier(*i, QSqlDriver::FieldName);
    QSqlQuery setsQuery(objectDb);
    setsQuery.prepare(QLatin1String("SELECT COUNT([id_arena]) FROM [Ratings] LEFT JOIN [SLRatings] on [Ratings].[name]=[SLRatings].[name] and "
                                    "[Ratings].[set]=[SLRatings].[set] WHERE "
                                    "[seen_count] NOT NULL AND [Ratings].[set] in (")
                      + setsEscaped.join(QLatin1Char(',')) + QLatin1Char(')'));
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

void MainObject::onWorkerInit()
{
    emit endProgress(opInitWorker);
    m_SLratingsModel->setTable(QStringLiteral("SLRatings"));
    m_SLratingsModel->select();
    Q_ASSERT(m_SLratingsModel->lastError().type() == QSqlError::NoError);
    m_ratingTemplateModel->setTable();
    selectSetsModel();
    emit startProgress(opDownloadSets, tr("Loading Sets"), 0, 0);
    emit startProgress(opDownloadSetsData, tr("Downloading Set Details"), 0, 0);
    m_worker->downloadSetsMTGAH();
}

void MainObject::fillMetrics()
{
    for (int i = 0; i < Worker::SLCount; ++i) {
        QStandardItem *item = new QStandardItem;
        item->setData(i, Qt::UserRole);
        if (i == Worker::SLdrawn_win_rate || i == Worker::SLavg_pick)
            item->setData(Qt::Checked, Qt::CheckStateRole);
        else
            item->setData(Qt::Unchecked, Qt::CheckStateRole);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
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
    QSqlQuery setsQuery(objectDb);
    setsQuery.prepare(QStringLiteral(
            "select [name], [id] from (SELECT [id], CASE WHEN [name] is NULL then [id] ELSE [name] END as [name], CASE WHEN [release_date] is  NULL "
            "then DATE() ELSE [release_date] END as [release_date] FROM [Sets] where [type] is null or [type] & ?) order by [release_date] desc"));
    setsQuery.addBindValue(DraftableSet);
    Q_ASSUME(setsQuery.exec());
    m_setsModel->setQuery(std::move(setsQuery));
    m_setsProxy->setData(m_setsProxy->index(0, 0), Qt::Checked, Qt::CheckStateRole);
}

void MainObject::onLoggedIn()
{
    emit endProgress(opLogIn);
    emit loggedIn();
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

void MainObject::onSetsMTGAH(bool needsUpdate)
{
    emit endProgress(opDownloadSets);
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
    emit endProgress(opUploadMTGAH);
    emit startProgress(opDownloadRatingTemplate, tr("Refreshing Ratings"), 0, 0);
}

void MainObject::onRatingUploaded(const QString &card)
{
    Q_UNUSED(card)
    emit increaseProgress(opUploadMTGAH, 1);
}

void MainObject::onFailedUploadRating()
{
    emit endProgress(opUploadMTGAH);
    emit failedUploadRating();
}
