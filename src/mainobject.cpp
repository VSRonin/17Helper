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
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlDriver>
//#define DEBUG_SINGLE_THREAD
MainObject::MainObject(QObject *parent)
    : QObject(parent)
    , m_workerThread(nullptr)
    , m_objectDbName(QStringLiteral("ObjectDb"))
{
    m_SLMetricsModel = new QStandardItemModel(SLCount, 1, this);
    fillMetrics();
    m_formatsModel = new QStandardItemModel(dfCount, 1, this);
    fillFormats();
    m_setsModel = new QSqlQueryModel(this);
    m_setsProxy = new CheckableProxy(this);
    m_setsProxy->setSourceModel(m_setsModel);
    m_ratingTemplateModel = new RatingsModel(this, openDb(m_objectDbName));
    QTimer::singleShot(0, this, std::bind(&MainObject::startProgress, this, opInitWorker, tr("Initialising"), 0, 0));
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
#ifdef QT_DEBUG
    qDebug() << filterString;
#endif
    m_ratingTemplateModel->setFilter(filterString);
}

void MainObject::tryLogin(const QString &userName, const QString &password, bool rememberMe)
{
    emit startProgress(opLogIn, tr("Logging in"), 1, 0);
    QTimer::singleShot(0, m_worker, std::bind(&Worker::tryLogin, m_worker, userName, password));
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
    QTimer::singleShot(0, m_worker, &Worker::logOut);
}

void MainObject::retranslateModels()
{
    m_formatsModel->setData(m_formatsModel->index(dfPremierDraft, 0), tr("Premier Draft"));
    m_formatsModel->setData(m_formatsModel->index(dfQuickDraft, 0), tr("Quick Draft"));
    m_formatsModel->setData(m_formatsModel->index(dfTradDraft, 0), tr("Traditional Draft"));
    m_formatsModel->setData(m_formatsModel->index(dfSealed, 0), tr("Sealed"));
    m_formatsModel->setData(m_formatsModel->index(dfTradSealed, 0), tr("Traditional Sealed"));

    if (SLcodes.isEmpty()) {
        SLcodes.reserve(SLCount);
        for (int i = 0; i < SLCount; ++i)
            SLcodes.append(QString());
    }
    Q_ASSERT(SLcodes.size() == SLCount);
    SLcodes[SLseen_count] = tr("#S");
    SLcodes[SLavg_seen] = tr("ALSA");
    SLcodes[SLpick_count] = tr("#P");
    SLcodes[SLavg_pick] = tr("ATA");
    SLcodes[SLgame_count] = tr("#GP");
    SLcodes[SLwin_rate] = tr("GPWR");
    SLcodes[SLopening_hand_game_count] = tr("#OH");
    SLcodes[SLopening_hand_win_rate] = tr("OHWR");
    SLcodes[SLdrawn_game_count] = tr("#GD");
    SLcodes[SLdrawn_win_rate] = tr("GDWR");
    SLcodes[SLever_drawn_game_count] = tr("#GIH");
    SLcodes[SLever_drawn_win_rate] = tr("GIHWR");
    SLcodes[SLnever_drawn_game_count] = tr("#GND");
    SLcodes[SLnever_drawn_win_rate] = tr("GNDWR");
    SLcodes[SLdrawn_improvement_win_rate] = tr("IWD");

    m_SLMetricsModel->item(SLseen_count)->setData(tr("Number Seen (%1)").arg(SLcodes.at(SLseen_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLavg_seen)->setData(tr("Average Last Seen At (%1)").arg(SLcodes.at(SLavg_seen)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLpick_count)->setData(tr("Number Picked (%1)").arg(SLcodes.at(SLpick_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLavg_pick)->setData(tr("Average Taken At (%1)").arg(SLcodes.at(SLavg_pick)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLgame_count)->setData(tr("Number of Games Played (%1)").arg(SLcodes.at(SLgame_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLwin_rate)->setData(tr("Games Played Win Rate (%1)").arg(SLcodes.at(SLwin_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLopening_hand_game_count)
            ->setData(tr("Number of Games in Opening Hand (%1)").arg(SLcodes.at(SLopening_hand_game_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLopening_hand_win_rate)
            ->setData(tr("Opening Hand Win Rate (%1)").arg(SLcodes.at(SLopening_hand_win_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLdrawn_game_count)->setData(tr("Number of Games Drawn (%1)").arg(SLcodes.at(SLdrawn_game_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLdrawn_win_rate)->setData(tr("Games Drawn Win Rate (%1)").arg(SLcodes.at(SLdrawn_win_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLever_drawn_game_count)
            ->setData(tr("Number of Games In Hand (%1)").arg(SLcodes.at(SLever_drawn_game_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLever_drawn_win_rate)->setData(tr("Games in Hand Win Rate (%1)").arg(SLcodes.at(SLever_drawn_win_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLnever_drawn_game_count)
            ->setData(tr("Number of Games Not Drawn (%1)").arg(SLcodes.at(SLnever_drawn_game_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLnever_drawn_win_rate)
            ->setData(tr("Games Not Drawn Win Rate (%1)").arg(SLcodes.at(SLnever_drawn_win_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLdrawn_improvement_win_rate)
            ->setData(tr("Improvement When Drawn (%1)").arg(SLcodes.at(SLdrawn_improvement_win_rate)), Qt::DisplayRole);
}

void MainObject::download17Lands(const QStringList &sets)
{
    // #TODO
}

void MainObject::onWorkerInit()
{
    emit endProgress(opInitWorker);
    selectSetsModel();
    getLast17LDownloadDate();
    m_ratingTemplateModel->setTable();
    emit startProgress(opDownloadSets, tr("Loading Sets"), 0, 0);
    emit startProgress(opDownloadSetsData, tr("Downloading Set Details"), 0, 0);
    QTimer::singleShot(0, m_worker, &Worker::downloadSetsMTGAH);
}

void MainObject::fillMetrics()
{
    for (int i = 0; i < SLCount; ++i) {
        QStandardItem *item = new QStandardItem;
        item->setData(i, Qt::UserRole);
        if (i == SLdrawn_win_rate || i == SLavg_pick)
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
    m_formatsModel->setData(m_formatsModel->index(dfSealed, 0), QStringLiteral("Sealed"), Qt::UserRole);
    m_formatsModel->setData(m_formatsModel->index(dfTradSealed, 0), QStringLiteral("TradSealed"), Qt::UserRole);
}

void MainObject::selectSetsModel()
{
    QSqlDatabase objectDb = openDb(m_objectDbName);
    QSqlQuery setsQuery(objectDb);
    setsQuery.prepare(QStringLiteral(
            "select [name], [id] from (SELECT [id], CASE WHEN [name] is NULL then [id] ELSE [name] END as [name], CASE WHEN [release_date] is "
            "NULL then DATE() ELSE [release_date] END as [release_date] FROM [Sets] where [type] & ?) order by [release_date] desc"));
    setsQuery.addBindValue(DraftableSet);
    Q_ASSUME(setsQuery.exec());
    m_setsModel->setQuery(std::move(setsQuery));
}

void MainObject::onLoggedIn()
{
    emit endProgress(opLogIn);
    emit loggedIn();
    emit startProgress(opDownloadRatingTemplate, tr("Downloading custom ratings from MTGA Helper"), 0, 0);
    QTimer::singleShot(0, m_worker, &Worker::getCustomRatingTemplate);
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

void MainObject::onSetsMTGAH()
{
    emit endProgress(opDownloadSets);
    m_setsProxy->setData(m_setsProxy->index(0, 0), Qt::Checked, Qt::CheckStateRole);
}

void MainObject::onRatingsTemplate(bool needsUpdate)
{
    emit endProgress(opDownloadRatingTemplate);
    if (needsUpdate)
        m_ratingTemplateModel->select();
}

QString MainObject::commentString(const SeventeenCard &card, const QLocale &locale) const
{
    QStringList result;
    const QString percent = locale.percent();
    if (m_SLMetricsModel->index(SLseen_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLseen_count) + QLatin1Char(':') + locale.toString(card.seen_count));
    if (m_SLMetricsModel->index(SLavg_seen, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLavg_seen) + QLatin1Char(':') + locale.toString(card.avg_seen, 'f', 2));
    if (m_SLMetricsModel->index(SLpick_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLpick_count) + QLatin1Char(':') + locale.toString(card.pick_count));
    if (m_SLMetricsModel->index(SLavg_pick, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLavg_pick) + QLatin1Char(':') + locale.toString(card.avg_pick, 'f', 2));
    if (m_SLMetricsModel->index(SLgame_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLgame_count) + QLatin1Char(':') + locale.toString(card.game_count));
    if (m_SLMetricsModel->index(SLwin_rate, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLwin_rate) + QLatin1Char(':') + locale.toString(card.win_rate * 100.0, 'f', 2) + percent);
    if (m_SLMetricsModel->index(SLopening_hand_game_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLopening_hand_game_count) + QLatin1Char(':') + locale.toString(card.opening_hand_game_count));
    if (m_SLMetricsModel->index(SLopening_hand_win_rate, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLopening_hand_win_rate) + QLatin1Char(':') + locale.toString(card.opening_hand_win_rate * 100.0, 'f', 2) + percent);
    if (m_SLMetricsModel->index(SLdrawn_game_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLdrawn_game_count) + QLatin1Char(':') + locale.toString(card.drawn_game_count));
    if (m_SLMetricsModel->index(SLdrawn_win_rate, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLdrawn_win_rate) + QLatin1Char(':') + locale.toString(card.drawn_win_rate * 100.0, 'f', 2) + percent);
    if (m_SLMetricsModel->index(SLever_drawn_game_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLever_drawn_game_count) + QLatin1Char(':') + locale.toString(card.ever_drawn_game_count));
    if (m_SLMetricsModel->index(SLever_drawn_win_rate, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLever_drawn_win_rate) + QLatin1Char(':') + locale.toString(card.ever_drawn_win_rate * 100.0, 'f', 2) + percent);
    if (m_SLMetricsModel->index(SLnever_drawn_game_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLnever_drawn_game_count) + QLatin1Char(':') + locale.toString(card.never_drawn_game_count));
    if (m_SLMetricsModel->index(SLnever_drawn_win_rate, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLnever_drawn_win_rate) + QLatin1Char(':') + locale.toString(card.never_drawn_win_rate * 100.0, 'f', 2) + percent);
    if (m_SLMetricsModel->index(SLdrawn_improvement_win_rate, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLdrawn_improvement_win_rate) + QLatin1Char(':') + locale.toString(card.drawn_improvement_win_rate * 100.0, 'f', 2)
                      + percent);
    return result.join(QLatin1Char(' '));
}

double MainObject::ratingValue(const SeventeenCard &card, SLMetrics method) const
{
    switch (method) {
    case SLseen_count:
        return card.seen_count;
    case SLavg_seen:
        return card.avg_seen;
    case SLpick_count:
        return card.pick_count;
    case SLavg_pick:
        return card.avg_pick;
    case SLgame_count:
        return card.game_count;
    case SLwin_rate:
        return card.win_rate;
    case SLopening_hand_game_count:
        return card.opening_hand_game_count;
    case SLopening_hand_win_rate:
        return card.opening_hand_win_rate;
    case SLdrawn_game_count:
        return card.drawn_game_count;
    case SLdrawn_win_rate:
        return card.drawn_win_rate;
    case SLever_drawn_game_count:
        return card.ever_drawn_game_count;
    case SLever_drawn_win_rate:
        return card.ever_drawn_win_rate;
    case SLnever_drawn_game_count:
        return card.never_drawn_game_count;
    case SLnever_drawn_win_rate:
        return card.never_drawn_win_rate;
    case SLdrawn_improvement_win_rate:
        return card.drawn_improvement_win_rate;
    default:
        return 0;
    }
}
