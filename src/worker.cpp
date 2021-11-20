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
#include "worker.h"
#include "globals.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include <QDir>
#include <QStandardPaths>
#ifdef QT_DEBUG
#    include <QDebug>
#endif
Worker::Worker(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_SLrequestOutstanding(0)
    , m_MTGAHrequestOutstanding(0)
    , m_workerDbName(QStringLiteral("WorkerDb"))
{
    m_requestTimer = new QTimer(this);
    m_requestTimer->setInterval(RequestTimerTimeout);
    connect(m_requestTimer, &QTimer::timeout, this, &Worker::processMTGAHrequestQueue);
    connect(m_requestTimer, &QTimer::timeout, this, &Worker::processSLrequestQueue);
    connect(m_requestTimer, &QTimer::timeout, this, &Worker::checkStopTimer);
}

void Worker::checkStopTimer()
{
    if (m_MTGAHrequestQueue.isEmpty() && m_SLrequestQueue.isEmpty())
        m_requestTimer->stop();
}

QSqlDatabase Worker::openWorkerDb()
{
    return openDb(m_workerDbName);
}

void Worker::init()
{
    QMetaObject::invokeMethod(this, &Worker::actualInit, Qt::QueuedConnection);
}

void Worker::actualInit()
{
    QSqlDatabase workerdb = openWorkerDb();
    Q_ASSERT(workerdb.driver()->hasFeature(QSqlDriver::PreparedQueries));
    Q_ASSERT(workerdb.driver()->hasFeature(QSqlDriver::PositionalPlaceholders));
    Q_ASSERT(workerdb.driver()->hasFeature(QSqlDriver::NamedPlaceholders));
    Q_ASSERT(workerdb.driver()->hasFeature(QSqlDriver::Transactions));
    QSqlQuery createSetsQuery(workerdb);
    createSetsQuery.prepare(
            QStringLiteral("CREATE TABLE IF NOT EXISTS [Sets] ([id] TEXT PRIMARY KEY, [name] TEXT, [type] INTEGER, [release_date] TEXT)"));
    if (!createSetsQuery.exec()) {
        emit initialisationFailed();
        return;
    }
    QSqlQuery createRatingsQuery(workerdb);
    createRatingsQuery.prepare(QStringLiteral("CREATE TABLE IF NOT EXISTS [Ratings] ([set] TEXT NOT NULL, [name] TEXT NOT NULL, [id_arena] INTEGER "
                                              "PRIMARY KEY, [lastUpdate] TEXT, [rating] INTEGER, [note] TEXT)"));
    if (!createRatingsQuery.exec()) {
        emit initialisationFailed();
        return;
    }
    QSqlQuery create17RatingsQuery(workerdb);
    create17RatingsQuery.prepare(QStringLiteral("CREATE TABLE IF NOT EXISTS [SLRatings] ([name] TEXT NOT NULL "
                                                ", [set] TEXT NOT NULL "
                                                ", [seen_count] INTEGER NOT NULL "
                                                ", [avg_seen] REAL NOT NULL "
                                                ", [pick_count] INTEGER NOT NULL "
                                                ", [avg_pick] REAL NOT NULL "
                                                ", [game_count] INTEGER NOT NULL "
                                                ", [win_rate] REAL NOT NULL "
                                                ", [opening_hand_game_count] INTEGER NOT NULL "
                                                ", [opening_hand_win_rate] REAL NOT NULL "
                                                ", [drawn_game_count] INTEGER NOT NULL "
                                                ", [drawn_win_rate] REAL NOT NULL "
                                                ", [ever_drawn_game_count] INTEGER NOT NULL "
                                                ", [ever_drawn_win_rate] REAL NOT NULL "
                                                ", [never_drawn_game_count] INTEGER NOT NULL "
                                                ", [never_drawn_win_rate] REAL NOT NULL "
                                                ", [drawn_improvement_win_rate] REAL NOT NULL "
                                                ", [lastUpdate] TEXT NOT NULL "
                                                ", PRIMARY KEY ([set], [name]) "
                                                ")"));
    if (!create17RatingsQuery.exec()) {
        emit initialisationFailed();
        return;
    }

    emit initialised();
}
void Worker::tryLogin(const QString &userName, const QString &password)
{
    QMetaObject::invokeMethod(this, std::bind(&Worker::actualTryLogin, this, userName, password), Qt::QueuedConnection);
}
void Worker::actualTryLogin(const QString &userName, const QString &password)
{
    if (userName.isEmpty() || password.isEmpty()) {
        emit loginFalied(tr("Please enter username and Password"));
        return;
    }
    const QUrl loginUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/Account/Signin?email=") + userName
                                              + QStringLiteral("&password=") + password);
    QNetworkReply *reply = m_nam->get(QNetworkRequest(loginUrl));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, std::bind(&Worker::onLogIn, this, reply));
}
void Worker::logOut()
{
    QMetaObject::invokeMethod(this, &Worker::actualLogOut, Qt::QueuedConnection);
}
void Worker::actualLogOut()
{
    const QUrl setsUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/Account/Signout"));
    QNetworkReply *reply = m_nam->post(QNetworkRequest(setsUrl), QByteArray());
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::errorOccurred, this, [reply, this]() { emit logoutFailed(tr("Error: %1").arg(reply->errorString())); });
    connect(reply, &QNetworkReply::finished, this, [reply, this]() -> void {
        if (reply->error() != QNetworkReply::NoError)
            return;
        emit loggedOut();
    });
}
void Worker::downloadSetsMTGAH()
{
    QMetaObject::invokeMethod(this, &Worker::actualDownloadSetsMTGAH, Qt::QueuedConnection);
}
void Worker::actualDownloadSetsMTGAH()
{
    const QUrl setsUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/Misc/Sets"));
    QNetworkReply *reply = m_nam->get(QNetworkRequest(setsUrl));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, std::bind(&Worker::onSetsMTGAHDownloaded, this, reply));
}

void Worker::saveMTGAHSets(QStringList sets)
{
    QSqlDatabase workerdb = openWorkerDb();
    QSqlQuery getSetsQuery(workerdb);
    getSetsQuery.prepare(QStringLiteral("SELECT [id] FROM [Sets]"));
    if (!getSetsQuery.exec()) {
        emit downloadSetsMTGAHFailed();
        return;
    }
    while (getSetsQuery.next())
        sets.removeAll(getSetsQuery.value(0).toString().trimmed());
    getSetsQuery.clear();
    if (sets.isEmpty()) {
        emit setsMTGAH(false);
        return;
    }
    std::sort(sets.begin(), sets.end());
    sets.erase(std::unique(sets.begin(), sets.end()), sets.end());
    Q_ASSUME(workerdb.transaction());
    for (auto i = sets.cbegin(), iEnd = sets.cend(); i != iEnd; ++i) {
        QSqlQuery insertSetsQuery(workerdb);
        insertSetsQuery.prepare(QStringLiteral("INSERT INTO [Sets] ([id]) VALUES (?)"));
        insertSetsQuery.addBindValue(*i);
        if (!insertSetsQuery.exec()) {
            Q_ASSUME(workerdb.rollback());
            emit downloadSetsMTGAHFailed();
            return;
        }
    }
    if (!workerdb.commit()) {
        Q_ASSUME(workerdb.rollback());
        emit downloadSetsMTGAHFailed();
        return;
    }
    emit setsMTGAH(true);
}

void Worker::downloadSetsScryfall()
{
    QStringList sets;
    QSqlDatabase workerdb = openWorkerDb();
    QSqlQuery scryfallSets(workerdb);
    scryfallSets.prepare(QStringLiteral("SELECT [id] FROM [Sets] WHERE [name] IS NULL OR [type] IS NULL OR [release_date] IS NULL"));
    if (scryfallSets.exec()) {
        while (scryfallSets.next())
            sets.append(scryfallSets.value(0).toString());
    }
    if (sets.isEmpty()) {
        emit setsScryfall(false);
        return;
    }
    const QUrl setsUrl = QUrl::fromUserInput(QStringLiteral("https://api.scryfall.com/sets"));
    QNetworkReply *reply = m_nam->get(QNetworkRequest(setsUrl));
    connect(reply, &QNetworkReply::errorOccurred, this, &Worker::downloadSetsScryfallFailed);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, std::bind(&Worker::parseSetsScryfall, this, reply, sets));
}

void Worker::parseSetsScryfall(QNetworkReply *reply, const QStringList &sets)
{
    if (reply->error() != QNetworkReply::NoError)
        return;
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        emit downloadSetsScryfallFailed();
        return;
    }
    QJsonParseError parseErr;
    const QJsonDocument setsDocument = QJsonDocument::fromJson(reply->readAll(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !setsDocument.isObject()) {
        emit downloadSetsScryfallFailed();
        return;
    }
    const QJsonObject setsObject = setsDocument.object();
    const QJsonValue setsArrVal = setsObject[QLatin1String("data")];
    if (!setsArrVal.isArray()) {
        emit downloadSetsScryfallFailed();
        return;
    }
    QSqlDatabase workerdb = openWorkerDb();
    const QJsonArray setsArray = setsArrVal.toArray();
    for (auto i = setsArray.cbegin(), iEnd = setsArray.cend(); i != iEnd; ++i) {
        const QJsonObject setObj = i->toObject();
        const QString setStr = setObj[QLatin1String("code")].toString().trimmed().toUpper();
        if (setStr.isEmpty())
            continue;
        if (sets.contains(setStr)) {
            QSqlQuery updateSetQuery(workerdb);
            updateSetQuery.prepare(
                    QStringLiteral("UPDATE [Sets] SET [name] = :name , [type] = :type , [release_date] = :releaseDt WHERE [id] = :id"));
            updateSetQuery.bindValue(QStringLiteral(":name"), setObj[QLatin1String("name")].toString().trimmed());
            updateSetQuery.bindValue(QStringLiteral(":type"), setTypeCode(setObj[QLatin1String("set_type")].toString().trimmed()));
            updateSetQuery.bindValue(QStringLiteral(":releaseDt"), setObj[QLatin1String("released_at")].toString().trimmed());
            updateSetQuery.bindValue(QStringLiteral(":id"), setStr);
            Q_ASSUME(updateSetQuery.exec());
        }
    }
    emit setsScryfall(true);
}

void Worker::onCustomRatingTemplateFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        qDebug() << "CustomRatingTemplate Failed: " << reply->readAll();
        emit customRatingTemplateFailed();
        return;
    }
    QJsonParseError parseErr;
    const QJsonDocument ratingsDocument = QJsonDocument::fromJson(reply->readAll(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !ratingsDocument.isArray()) {
        emit customRatingTemplateFailed();
        return;
    }
    bool oneFound = false;
    bool needUpdate = false;
    QSqlDatabase workerdb = openWorkerDb();
    Q_ASSUME(workerdb.transaction());
    const QJsonArray ratingsArray = ratingsDocument.array();
    for (auto i = ratingsArray.cbegin(), iEnd = ratingsArray.cend(); i != iEnd; ++i) {
        if (!i->isObject())
            continue;
        const QJsonObject ratingObject = i->toObject();
        const QJsonObject cardObject = ratingObject[QLatin1String("card")].toObject();
        if (cardObject.isEmpty())
            continue;
        const int idArenaVal = cardObject[QLatin1String("idArena")].toInt();
        const QString setStr = cardObject[QLatin1String("set")].toString().trimmed().toUpper();
        if (setStr.isEmpty())
            continue;
        const QString nameStr = cardObject[QLatin1String("name")].toString().trimmed();
        if (nameStr.isEmpty())
            continue;
        oneFound = true;
        const QJsonValue noteValue = ratingObject[QLatin1String("note")];
        const QString noteStr = noteValue.toString();
        const QJsonValue ratngValue = ratingObject[QLatin1String("rating")];
        const int ratingNum = ratngValue.isNull() ? -1 : ratngValue.toInt();
        if (!needUpdate) {
            QSqlQuery checkRatingQuery(workerdb);
            checkRatingQuery.prepare(
                    QStringLiteral("SELECT [id_arena], [name], [set], [rating], [note] FROM [Ratings] WHERE [id_arena] = :id_arena"));
            checkRatingQuery.bindValue(QStringLiteral(":id_arena"), idArenaVal);
            if (checkRatingQuery.exec() && checkRatingQuery.next() && checkRatingQuery.value(1).toString() == nameStr
                && checkRatingQuery.value(2).toString() == setStr && checkRatingQuery.value(3).toInt() == ratingNum
                && checkRatingQuery.value(4).toString() == noteStr)
                continue;
            needUpdate = true;
        }
        QSqlQuery updateRatingQuery(workerdb);
        updateRatingQuery.prepare(
                QStringLiteral("INSERT OR REPLACE INTO [Ratings] ([id_arena], [name], [set], [rating], [note], [lastUpdate]) VALUES (:id_arena, "
                               ":name, :set, :rating, :note, (select [lastUpdate] from [Ratings] where [id_arena]=:id_arena_search))"));
        updateRatingQuery.bindValue(QStringLiteral(":id_arena"), idArenaVal);
        updateRatingQuery.bindValue(QStringLiteral("id_arena_search"), idArenaVal);
        updateRatingQuery.bindValue(QStringLiteral(":name"), nameStr);
        updateRatingQuery.bindValue(QStringLiteral(":set"), setStr);
        updateRatingQuery.bindValue(QStringLiteral(":rating"), ratingNum);
        if (noteStr.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            updateRatingQuery.bindValue(QStringLiteral(":note"), QVariant(QMetaType(QMetaType::QString)));
#else
            updateRatingQuery.bindValue(QStringLiteral(":note"), QVariant(QVariant::String));
#endif
        } else {
            updateRatingQuery.bindValue(QStringLiteral(":note"), noteStr);
        }
        if (!updateRatingQuery.exec()) {
            Q_ASSUME(workerdb.rollback());
            emit customRatingTemplateFailed();
            return;
        }
    }
    if (!oneFound) {
        Q_ASSUME(workerdb.rollback());
        emit customRatingTemplateFailed();
        return;
    }
    if (!workerdb.commit()) {
        Q_ASSUME(workerdb.rollback());
        emit customRatingTemplateFailed();
        return;
    }
    emit customRatingTemplate(needUpdate);
}

void Worker::on17LDownloadFinished(QNetworkReply *reply, const QString &currSet)
{
    --m_SLrequestOutstanding;
    if (reply->error() != QNetworkReply::NoError)
        return;
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        emit failed17LRatings();
        return;
    }
    QJsonParseError parseErr;
    const QJsonDocument ratingsDocument = QJsonDocument::fromJson(reply->readAll(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !ratingsDocument.isArray()) {
        emit failed17LRatings();
        return;
    }
    const QString currDateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    QSqlDatabase workerdb = openWorkerDb();
    Q_ASSUME(workerdb.transaction());
    bool oneFound = false;
    const QJsonArray ratingsArray = ratingsDocument.array();
    for (auto i = ratingsArray.cbegin(), iEnd = ratingsArray.cend(); i != iEnd; ++i) {
        if (!i->isObject())
            continue;
        const QJsonObject ratingObject = i->toObject();
        const QString nameStr = ratingObject[QLatin1String("name")].toString();
        if (nameStr.isEmpty())
            continue;
        QSqlQuery updateRatingQuery(workerdb);
        updateRatingQuery.prepare(
                QStringLiteral("INSERT OR REPLACE INTO [SLRatings] ([set], [name], [seen_count], [avg_seen], [pick_count], [avg_pick], [game_count], "
                               "[win_rate], "
                               "[opening_hand_game_count], [opening_hand_win_rate], [drawn_game_count], [drawn_win_rate], [ever_drawn_game_count], "
                               "[ever_drawn_win_rate], [never_drawn_game_count], [never_drawn_win_rate], [drawn_improvement_win_rate], [lastUpdate]) "
                               "VALUES"
                               "(:set, :name, :seen_count, :avg_seen, :pick_count, :avg_pick, :game_count, :win_rate, :opening_hand_game_count, "
                               ":opening_hand_win_rate, :drawn_game_count, :drawn_win_rate, :ever_drawn_game_count, :ever_drawn_win_rate, "
                               ":never_drawn_game_count, :never_drawn_win_rate, :drawn_improvement_win_rate, :lastUpdate)"));
        updateRatingQuery.bindValue(QStringLiteral(":set"), currSet);
        updateRatingQuery.bindValue(QStringLiteral(":lastUpdate"), currDateTime);
        updateRatingQuery.bindValue(QStringLiteral(":name"), nameStr);
        updateRatingQuery.bindValue(QStringLiteral(":seen_count"), ratingObject[QLatin1String("seen_count")].toInt());
        updateRatingQuery.bindValue(QStringLiteral(":avg_seen"), ratingObject[QLatin1String("avg_seen")].toDouble());
        updateRatingQuery.bindValue(QStringLiteral(":pick_count"), ratingObject[QLatin1String("pick_count")].toInt());
        updateRatingQuery.bindValue(QStringLiteral(":avg_pick"), ratingObject[QLatin1String("avg_pick")].toDouble());
        updateRatingQuery.bindValue(QStringLiteral(":game_count"), ratingObject[QLatin1String("game_count")].toInt());
        updateRatingQuery.bindValue(QStringLiteral(":win_rate"), ratingObject[QLatin1String("win_rate")].toDouble());
        updateRatingQuery.bindValue(QStringLiteral(":opening_hand_game_count"), ratingObject[QLatin1String("opening_hand_game_count")].toInt());
        updateRatingQuery.bindValue(QStringLiteral(":opening_hand_win_rate"), ratingObject[QLatin1String("opening_hand_win_rate")].toDouble());
        updateRatingQuery.bindValue(QStringLiteral(":drawn_game_count"), ratingObject[QLatin1String("drawn_game_count")].toInt());
        updateRatingQuery.bindValue(QStringLiteral(":drawn_win_rate"), ratingObject[QLatin1String("drawn_win_rate")].toDouble());
        updateRatingQuery.bindValue(QStringLiteral(":ever_drawn_game_count"), ratingObject[QLatin1String("ever_drawn_game_count")].toInt());
        updateRatingQuery.bindValue(QStringLiteral(":ever_drawn_win_rate"), ratingObject[QLatin1String("ever_drawn_win_rate")].toDouble());
        updateRatingQuery.bindValue(QStringLiteral(":never_drawn_game_count"), ratingObject[QLatin1String("never_drawn_game_count")].toInt());
        updateRatingQuery.bindValue(QStringLiteral(":never_drawn_win_rate"), ratingObject[QLatin1String("never_drawn_win_rate")].toDouble());
        updateRatingQuery.bindValue(QStringLiteral(":drawn_improvement_win_rate"),
                                    ratingObject[QLatin1String("drawn_improvement_win_rate")].toDouble());
        if (!updateRatingQuery.exec()) {
            emit failed17LRatings();
            Q_ASSUME(workerdb.rollback());
            return;
        }
        oneFound = true;
    }
    if (!oneFound) {
        emit failed17LRatings();
        Q_ASSUME(workerdb.rollback());
        return;
    }
    if (!workerdb.commit()) {
        emit failed17LRatings();
        Q_ASSUME(workerdb.rollback());
        return;
    }
    emit downloaded17LRatings(currSet);
    if (m_SLrequestQueue.size() + m_SLrequestOutstanding == 0)
        emit downloadedAll17LRatings();
}

void Worker::onSetsMTGAHDownloaded(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit downloadSetsMTGAHFailed();
        return;
    }
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        emit downloadSetsMTGAHFailed();
        return;
    }
    QJsonParseError parseErr;
    QJsonDocument setsDocument = QJsonDocument::fromJson(reply->readAll(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !setsDocument.isObject()) {
        emit downloadSetsMTGAHFailed();
        return;
    }
    QJsonObject setsObject = setsDocument.object();
    QJsonValue setsArrVal = setsObject[QLatin1String("sets")];
    if (!setsArrVal.isArray()) {
        emit downloadSetsMTGAHFailed();
        return;
    }
    QJsonArray setsArray = setsArrVal.toArray();
    QStringList setList;
    for (auto i = setsArray.cbegin(), iEnd = setsArray.cend(); i != iEnd; ++i) {
        const QString setStr = i->toObject()[QLatin1String("name")].toString().trimmed().toUpper();
        if (!setStr.isEmpty())
            setList.append(setStr.toUpper());
    }
    if (setList.isEmpty()) {
        emit downloadSetsMTGAHFailed();
        return;
    }
    saveMTGAHSets(setList);
    downloadSetsScryfall();
}

void Worker::onLogIn(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit loginFalied(tr("Error: %1").arg(reply->errorString()));
        return;
    }
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        emit loginFalied(tr("Error: %1").arg(reply->errorString()));
        return;
    }
    QJsonParseError parseErr;
    QJsonDocument loginDocument = QJsonDocument::fromJson(reply->readAll(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !loginDocument.isObject()) {
        emit loginFalied(tr("Invalid response from mtgahelper.com"));
        return;
    }
    QJsonObject loginObject = loginDocument.object();
    if (!loginObject[QLatin1String("isAuthenticated")].toBool(false)) {
        emit loginFalied(tr("Invalid response from mtgahelper.com"));
        return;
    }
    emit loggedIn();
}

int Worker::setTypeCode(const QString &setType) const
{
    if (setType == QStringLiteral("core"))
        return stcore;
    if (setType == QStringLiteral("expansion"))
        return stexpansion;
    if (setType == QStringLiteral("masters"))
        return stmasters;
    if (setType == QStringLiteral("masterpiece"))
        return stmasterpiece;
    if (setType == QStringLiteral("from_the_vault"))
        return stfrom_the_vault;
    if (setType == QStringLiteral("spellbook"))
        return stspellbook;
    if (setType == QStringLiteral("premium_deck"))
        return stpremium_deck;
    if (setType == QStringLiteral("duel_deck"))
        return stduel_deck;
    if (setType == QStringLiteral("draft_innovation"))
        return stdraft_innovation;
    if (setType == QStringLiteral("treasure_chest"))
        return sttreasure_chest;
    if (setType == QStringLiteral("commander"))
        return stcommander;
    if (setType == QStringLiteral("planechase"))
        return stplanechase;
    if (setType == QStringLiteral("archenemy"))
        return starchenemy;
    if (setType == QStringLiteral("vanguard"))
        return stvanguard;
    if (setType == QStringLiteral("funny"))
        return stfunny;
    if (setType == QStringLiteral("starter"))
        return ststarter;
    if (setType == QStringLiteral("box"))
        return stbox;
    if (setType == QStringLiteral("promo"))
        return stpromo;
    if (setType == QStringLiteral("token"))
        return sttoken;
    if (setType == QStringLiteral("memorabilia"))
        return stmemorabilia;
    return 0;
}
void Worker::getCustomRatingTemplate()
{
    QMetaObject::invokeMethod(this, &Worker::actualGetCustomRatingTemplate, Qt::QueuedConnection);
}
void Worker::actualGetCustomRatingTemplate()
{
    const QUrl setsUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/User/customDraftRatingsForDisplay"));
    QNetworkReply *reply = m_nam->get(QNetworkRequest(setsUrl));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, std::bind(&Worker::onCustomRatingTemplateFinished, this, reply));
}
void Worker::get17LRatings(const QStringList &sets, const QString &format)
{
    QMetaObject::invokeMethod(this, std::bind(&Worker::actualGet17LRatings, this, sets, format), Qt::QueuedConnection);
}
void Worker::actualGet17LRatings(const QStringList &sets, const QString &format)
{
    if (sets.isEmpty() || format.isEmpty()) {
        emit failed17LRatings();
        return;
    }
    m_SLrequestQueue.clear();
    for (const QString &set : sets) {
        const QUrl ratingsUrl = QUrl::fromUserInput(QStringLiteral("https://www.17lands.com/card_ratings/data?expansion=") + set
                                                    + QLatin1String("&format=") + format);
        m_SLrequestQueue.enqueue(std::make_pair(set, QNetworkRequest(ratingsUrl)));
    }
    if (!m_requestTimer->isActive())
        m_requestTimer->start();
}

void Worker::processSLrequestQueue()
{
    if (m_SLrequestQueue.isEmpty())
        return;
    const std::pair<QString, QNetworkRequest> currReq = m_SLrequestQueue.dequeue();
    const QString currSet = currReq.first;
    ++m_SLrequestOutstanding;
    QNetworkReply *reply = m_nam->get(currReq.second);
    connect(reply, &QNetworkReply::errorOccurred, this, &Worker::failed17LRatings);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, std::bind(&Worker::on17LDownloadFinished, this, reply, currSet));
}
void Worker::clearRatings(const QStringList &sets, SLMetrics ratingMethod, const QVector<SLMetrics> &commentStats, const QStringList &SLcodes,
                          const QLocale &locale)
{
    QMetaObject::invokeMethod(this, std::bind(&Worker::actualUploadRatings, this, sets, ratingMethod, commentStats, SLcodes, locale, true),
                              Qt::QueuedConnection);
}
void Worker::uploadRatings(const QStringList &sets, SLMetrics ratingMethod, const QVector<SLMetrics> &commentStats, const QStringList &SLcodes,
                           const QLocale &locale)
{
    QMetaObject::invokeMethod(this, std::bind(&Worker::actualUploadRatings, this, sets, ratingMethod, commentStats, SLcodes, locale, false),
                              Qt::QueuedConnection);
}
void Worker::actualUploadRatings(QStringList sets, SLMetrics ratingMethod, QVector<SLMetrics> commentStats, const QStringList &SLcodes,
                                 const QLocale &locale, bool clear)
{
    if (sets.isEmpty() || SLcodes.isEmpty()) {
        emit failedRatingCalculation();
        return;
    }
    std::sort(commentStats.begin(), commentStats.end());
    commentStats.erase(std::unique(commentStats.begin(), commentStats.end()), commentStats.end());
    QSqlDatabase workerdb = openWorkerDb();
    const QString ratingMethodField = fieldName(ratingMethod);
    if (ratingMethodField.isEmpty()) {
        emit failedRatingCalculation();
        return;
    }
    QStringList commentFields;
    commentFields.reserve(commentStats.size());
    for (SLMetrics i : qAsConst(commentStats))
        commentFields.append(fieldName(i));
    for (auto i = sets.begin(), iEnd = sets.end(); i != iEnd; ++i)
        *i = workerdb.driver()->escapeIdentifier(*i, QSqlDriver::FieldName);
    QSqlQuery percentilesRatingQuery(workerdb);
    percentilesRatingQuery.prepare(QLatin1String("SELECT [Ratings].[set], [") + ratingMethodField
                                   + QLatin1String("] FROM [Ratings] LEFT JOIN [SLRatings] on [Ratings].[name]=[SLRatings].[name] and "
                                                   "[Ratings].[set]=[SLRatings].[set] WHERE  [seen_count] NOT NULL AND [Ratings].[set] in (")
                                   + sets.join(QLatin1Char(',')) + QLatin1String(") order by [") + ratingMethodField + QLatin1String("] asc"));
    Q_ASSUME(percentilesRatingQuery.exec());
    QMap<QString, QVector<double>> percentiles;
    while (percentilesRatingQuery.next()) {
        percentiles[percentilesRatingQuery.value(0).toString()].append(percentilesRatingQuery.value(1).toDouble());
    }
    if (percentiles.isEmpty()
        || std::any_of(percentiles.cbegin(), percentiles.cend(), [](const QVector<double> &vec) -> bool { return vec.isEmpty(); })) {
        emit failedRatingCalculation();
        return;
    }
    percentiles = reduceDeciles(percentiles);
    percentilesRatingQuery.clear();
    QString ratingsToUploadQueryString =
            QLatin1String("SELECT [id_arena], [Ratings].[set], [Ratings].[name], [") + ratingMethodField + QLatin1Char(']');
    if (!commentFields.isEmpty()) {
        ratingsToUploadQueryString += QLatin1String(", [") + commentFields.join(QLatin1String("], [")) + QLatin1Char(']');
    }
    ratingsToUploadQueryString += QLatin1String(+" FROM [Ratings] LEFT JOIN [SLRatings] on [Ratings].[name]=[SLRatings].[name] and "
                                                 "[Ratings].[set]=[SLRatings].[set] WHERE [seen_count] "
                                                 "NOT NULL AND [Ratings].[set] in (")
            + sets.join(QLatin1Char(',')) + QLatin1Char(')');
    QSqlQuery ratingsToUploadQuery(workerdb);
    ratingsToUploadQuery.prepare(ratingsToUploadQueryString);
    Q_ASSUME(ratingsToUploadQuery.exec());
    m_MTGAHrequestQueue.clear();
    while (ratingsToUploadQuery.next()) {
        QJsonObject cardData;
        cardData[QLatin1String("idArena")] = ratingsToUploadQuery.value(0).toInt();
        const int cardRating =
                ratingValue(ratingMethod, ratingsToUploadQuery.value(3).toDouble(), percentiles.value(ratingsToUploadQuery.value(1).toString()));
        if (cardRating < 0 || clear)
            cardData[QLatin1String("rating")] = QJsonValue();
        else
            cardData[QLatin1String("rating")] = cardRating;
        const QString commentStr = commentString(ratingsToUploadQuery, commentStats, SLcodes, locale);
        if (commentStr.isEmpty() || clear)
            cardData[QLatin1String("note")] = QJsonValue();
        else
            cardData[QLatin1String("note")] = commentStr;
        cardData[QLatin1String("attempt")] = 0;
        const QString cardName = ratingsToUploadQuery.value(2).toString();
        ;
        cardData[QLatin1String("name")] = cardName;
        m_MTGAHrequestQueue.enqueue(cardData);
        emit ratingCalculated(cardName);
    }
    if (m_MTGAHrequestQueue.isEmpty()) {
        emit failedRatingCalculation();
        return;
    }
    emit ratingsCalculated();
    QObject *contextObject = new QObject(this);
    connect(this, &Worker::failedUploadRating, contextObject, &QObject::deleteLater);
    connect(this, &Worker::allRatingsUploaded, contextObject, &QObject::deleteLater);
    connect(this, &Worker::allRatingsUploaded, contextObject, [sets, this]() {
        QSqlDatabase workerdb = openWorkerDb();
        Q_ASSUME(workerdb.transaction());
        QSqlQuery ratingsUpdateDateQuery(workerdb);
        const QString ratingsUpdateDateQueryString = QLatin1String("UPDATE [Ratings] SET [lastUpdate]='")
                + QDateTime::currentDateTime().toString(Qt::ISODate) + QLatin1String("' WHERE [set] in (") + sets.join(QLatin1Char(','))
                + QLatin1Char(')');
        ratingsUpdateDateQuery.prepare(ratingsUpdateDateQueryString);
        Q_ASSUME(ratingsUpdateDateQuery.exec());
        if (!workerdb.commit())
            Q_ASSUME(workerdb.rollback());
    });
    if (!m_requestTimer->isActive())
        m_requestTimer->start();
}

void Worker::processMTGAHrequestQueue()
{
    if (m_MTGAHrequestQueue.isEmpty())
        return;
    if (m_MTGAHrequestOutstanding > 0)
        return;
    QJsonObject reqData = m_MTGAHrequestQueue.takeFirst();
    if (reqData.value(QLatin1String("attempt")).toInt() >= 3) {
        m_MTGAHrequestQueue.clear();
        emit failedUploadRating();
        return;
    }
    const QUrl ratingUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/User/CustomDraftRating"));
    QNetworkRequest ratingReq(ratingUrl);
    ratingReq.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    ++m_MTGAHrequestOutstanding;
    QNetworkReply *reply = m_nam->put(ratingReq, QJsonDocument(reqData).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, [reply, this, reqData]() -> void {
        --m_MTGAHrequestOutstanding;
        if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
            qDebug().noquote() << QStringLiteral("Failed: ") << reqData.value(QLatin1String("idArena")).toInt()
                               << reqData.value(QLatin1String("name")).toString() << QStringLiteral(" Body: ") << reply->readAll();
            reqData[QLatin1String("attempt")] = reqData.value(QLatin1String("attempt")).toInt() + 1;
            m_MTGAHrequestQueue.enqueue(reqData);
            if (!m_requestTimer->isActive())
                m_requestTimer->start();
            return;
        }
        qDebug().noquote() << QStringLiteral("Done: ") << reqData.value(QLatin1String("idArena")).toInt()
                           << reqData.value(QLatin1String("name")).toString();
        emit ratingUploaded(reqData.value(QLatin1String("name")).toString());
        if (m_MTGAHrequestQueue.size() + m_MTGAHrequestOutstanding == 0) {
            emit allRatingsUploaded();
            getCustomRatingTemplate();
        }
    });
}

QString Worker::fieldName(SLMetrics metric) const
{
    switch (metric) {
    case SLseen_count:
        return QStringLiteral("seen_count");
    case SLavg_seen:
        return QStringLiteral("avg_seen");
    case SLpick_count:
        return QStringLiteral("pick_count");
    case SLavg_pick:
        return QStringLiteral("avg_pick");
    case SLgame_count:
        return QStringLiteral("game_count");
    case SLwin_rate:
        return QStringLiteral("win_rate");
    case SLopening_hand_game_count:
        return QStringLiteral("opening_hand_game_count");
    case SLopening_hand_win_rate:
        return QStringLiteral("opening_hand_win_rate");
    case SLdrawn_game_count:
        return QStringLiteral("drawn_game_count");
    case SLdrawn_win_rate:
        return QStringLiteral("drawn_win_rate");
    case SLever_drawn_game_count:
        return QStringLiteral("ever_drawn_game_count");
    case SLever_drawn_win_rate:
        return QStringLiteral("ever_drawn_win_rate");
    case SLnever_drawn_game_count:
        return QStringLiteral("never_drawn_game_count");
    case SLnever_drawn_win_rate:
        return QStringLiteral("never_drawn_win_rate");
    case SLdrawn_improvement_win_rate:
        return QStringLiteral("drawn_improvement_win_rate");
    default:
        return QString();
    }
}
QString Worker::commentvalue(SLMetrics metric, const QVariant &value, const QLocale &locale) const
{
    switch (metric) {
    case SLseen_count:
    case SLgame_count:
    case SLpick_count:
    case SLopening_hand_game_count:
    case SLdrawn_game_count:
    case SLever_drawn_game_count:
    case SLnever_drawn_game_count:
        return locale.toString(value.toInt());
    case SLavg_pick:
    case SLavg_seen:
        return locale.toString(value.toDouble(), 'f', 2);
    case SLwin_rate:
    case SLopening_hand_win_rate:
    case SLdrawn_win_rate:
    case SLever_drawn_win_rate:
    case SLnever_drawn_win_rate:
    case SLdrawn_improvement_win_rate:
        return locale.toString(value.toDouble() * 100.0, 'f', 2) + locale.percent();
    default:
        return QString();
    }
}
QString Worker::commentString(const QSqlQuery &query, const QVector<SLMetrics> &commentStats, const QStringList &SLcodes, const QLocale &locale) const
{
    QStringList result;
    result.reserve(commentStats.size());
    for (SLMetrics metric : commentStats)
        result.append(SLcodes.at(metric) + QLatin1Char(':') + commentvalue(metric, query.value(fieldName(metric)), locale));
    return result.join(QLatin1Char(' '));
}
int Worker::ratingValue(SLMetrics metric, const double &val, const QVector<double> &deciles)
{
    Q_ASSERT(!deciles.isEmpty());
    switch (metric) {
    case SLseen_count:
    case SLavg_seen:
    case SLgame_count:
    case SLwin_rate:
    case SLopening_hand_game_count:
    case SLopening_hand_win_rate:
    case SLdrawn_game_count:
    case SLdrawn_win_rate:
    case SLever_drawn_game_count:
    case SLever_drawn_win_rate:
    case SLnever_drawn_game_count:
    case SLnever_drawn_win_rate:
    case SLdrawn_improvement_win_rate:
    case SLpick_count:
        return std::distance(deciles.cbegin(), std::lower_bound(deciles.cbegin(), deciles.cend(), val)) + 1;
    case SLavg_pick:
        return std::distance(std::lower_bound(deciles.cbegin(), deciles.cend(), val), deciles.cend());
    default:
        return 0;
    }
}

QMap<QString, QVector<double>> Worker::reduceDeciles(const QMap<QString, QVector<double>> &data)
{
    QMap<QString, QVector<double>> deciles;
    for (auto j = data.cbegin(), jEnd = data.cend(); j != jEnd; ++j) {
        const int dataSize = j->size();
        if (dataSize <= 10) {
            deciles.insert(j.key(), j.value());
            continue;
        }
        const auto setIter = deciles.insert(j.key(), QVector<double>());
        setIter->reserve(10);
        for (int i = 1; i < 10; ++i)
            setIter->append(j->at((i * dataSize) / 10));
        setIter->append(j->last());
        Q_ASSERT(setIter->size() == 10);
    }
    return deciles;
}
