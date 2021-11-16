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
}

void Worker::checkStopTimer()
{
    if (/*m_MTGAHrequestQueue.isEmpty() &&*/ m_SLrequestQueue.isEmpty())
        m_requestTimer->stop();
}

QSqlDatabase Worker::openWorkerDb()
{
    return openDb(m_workerDbName);
}

void Worker::init(){
    QMetaObject::invokeMethod(this,&Worker::actualInit,Qt::QueuedConnection);
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
    create17RatingsQuery.prepare(QStringLiteral("CREATE TABLE IF NOT EXISTS [SLRatings] ([name] TEXT PRIMARY KEY "
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
                                                ")"));
    if (!create17RatingsQuery.exec()) {
        emit initialisationFailed();
        return;
    }

    emit initialised();
}
void Worker::tryLogin(const QString &userName, const QString &password){
    QMetaObject::invokeMethod(this,std::bind(&Worker::actualTryLogin,this,userName,password),Qt::QueuedConnection);
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
    connect(reply, &QNetworkReply::errorOccurred, this, [reply, this]() { emit loginFalied(tr("Error: %1").arg(reply->errorString())); });
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, [reply, this]() -> void {
        if (reply->error() != QNetworkReply::NoError)
            return;
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
    });
}
void Worker::logOut(){
    QMetaObject::invokeMethod(this,&Worker::actualLogOut,Qt::QueuedConnection);
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
void Worker::downloadSetsMTGAH(){
    QMetaObject::invokeMethod(this,&Worker::actualDownloadSetsMTGAH,Qt::QueuedConnection);
}
void Worker::actualDownloadSetsMTGAH()
{
    const QUrl setsUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/Misc/Sets"));
    QNetworkReply *reply = m_nam->get(QNetworkRequest(setsUrl));
    connect(reply, &QNetworkReply::errorOccurred, this, &Worker::downloadSetsMTGAHFailed);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, [reply, this]() -> void {
        if (reply->error() != QNetworkReply::NoError)
            return;
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
    });
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
    for (auto i = sets.cbegin(), iEnd = sets.cend(); i != iEnd; ++i) {
        QSqlQuery insertSetsQuery(workerdb);
        insertSetsQuery.prepare(QStringLiteral("INSERT INTO [Sets] ([id]) VALUES (?)"));
        insertSetsQuery.addBindValue(*i);
        if (!insertSetsQuery.exec()) {
            emit downloadSetsMTGAHFailed();
            return;
        }
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
    if (reply->error() != QNetworkReply::NoError)
        return;
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
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
        const QJsonValue ratingValue = ratingObject[QLatin1String("rating")];
        const int ratingNum = ratingValue.isNull() ? -1 : ratingValue.toInt();
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
        updateRatingQuery.bindValue(QStringLiteral(":note"), noteStr);
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
void Worker::getCustomRatingTemplate(){
    QMetaObject::invokeMethod(this,&Worker::getCustomRatingTemplate,Qt::QueuedConnection);
}
void Worker::actualGetCustomRatingTemplate()
{
    const QUrl setsUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/User/customDraftRatingsForDisplay"));
    QNetworkReply *reply = m_nam->get(QNetworkRequest(setsUrl));
    connect(reply, &QNetworkReply::errorOccurred, this, &Worker::customRatingTemplateFailed);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, std::bind(&Worker::onCustomRatingTemplateFinished, this, reply));
}
void Worker::get17LRatings(const QStringList &sets, const QString &format){
    QMetaObject::invokeMethod(this,std::bind(&Worker::actualGet17LRatings,this,sets,format),Qt::QueuedConnection);
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
        m_SLrequestQueue.append(std::make_pair(set, QNetworkRequest(ratingsUrl)));
    }
    if (!m_requestTimer->isActive())
        m_requestTimer->start();
}

void Worker::processSLrequestQueue()
{
    if (m_SLrequestQueue.isEmpty())
        return;
    const std::pair<QString, QNetworkRequest> currReq = m_SLrequestQueue.takeFirst();
    const QString currSet = currReq.first;
    ++m_SLrequestOutstanding;
    QNetworkReply *reply = m_nam->get(currReq.second);
    connect(reply, &QNetworkReply::errorOccurred, this, &Worker::failed17LRatings);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, [reply, this, currSet]() -> void {
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
        const QDateTime currDateTime = QDateTime::currentDateTime();
        QSqlDatabase workerdb = openWorkerDb();
        Q_ASSUME(workerdb.transaction());
        bool oneFound = false;
        const QJsonArray ratingsArray = ratingsDocument.array();
        for (auto i = ratingsArray.cbegin(), iEnd = ratingsArray.cend(); i != iEnd; ++i) {
            QCoreApplication::processEvents();
            if (!i->isObject())
                continue;
            const QJsonObject ratingObject = i->toObject();
            const QString nameStr = ratingObject[QLatin1String("name")].toString();
            if (nameStr.isEmpty())
                continue;
            QSqlQuery updateRatingQuery(workerdb);
            updateRatingQuery.prepare(
                    QStringLiteral("INSERT OR REPLACE INTO [SLRatings] ([name], [seen_count], [avg_seen], [pick_count], [avg_pick], [game_count], [win_rate], [opening_hand_game_count], [opening_hand_win_rate], [drawn_game_count], [drawn_win_rate], [ever_drawn_game_count], [ever_drawn_win_rate], [never_drawn_game_count], [never_drawn_win_rate], [drawn_improvement_win_rate], [lastUpdate]) "
                                   "VALUES"
                                   "(:name, :seen_count, :avg_seen, :pick_count, :avg_pick, :game_count, :win_rate, :opening_hand_game_count, :opening_hand_win_rate, :drawn_game_count, :drawn_win_rate, :ever_drawn_game_count, :ever_drawn_win_rate, :never_drawn_game_count, :never_drawn_win_rate, :drawn_improvement_win_rate, :lastUpdate)")
                        );
            updateRatingQuery.bindValue(QStringLiteral(":lastUpdate"),currDateTime.toString(Qt::ISODate));
            updateRatingQuery.bindValue(QStringLiteral(":name"),nameStr);
            updateRatingQuery.bindValue(QStringLiteral("seen_count"),ratingObject[QLatin1String("seen_count")].toInt());
            updateRatingQuery.bindValue(QStringLiteral("avg_seen"),ratingObject[QLatin1String("avg_seen")].toDouble());
            updateRatingQuery.bindValue(QStringLiteral("pick_count"),ratingObject[QLatin1String("pick_count")].toInt());
            updateRatingQuery.bindValue(QStringLiteral("avg_pick"),ratingObject[QLatin1String("avg_pick")].toDouble());
            updateRatingQuery.bindValue(QStringLiteral("game_count"),ratingObject[QLatin1String("game_count")].toInt());
            updateRatingQuery.bindValue(QStringLiteral("win_rate"),ratingObject[QLatin1String("win_rate")].toDouble());
            updateRatingQuery.bindValue(QStringLiteral("opening_hand_game_count"),ratingObject[QLatin1String("opening_hand_game_count")].toInt());
            updateRatingQuery.bindValue(QStringLiteral("opening_hand_win_rate"),ratingObject[QLatin1String("opening_hand_win_rate")].toDouble());
            updateRatingQuery.bindValue(QStringLiteral("drawn_game_count"),ratingObject[QLatin1String("drawn_game_count")].toInt());
            updateRatingQuery.bindValue(QStringLiteral("drawn_win_rate"),ratingObject[QLatin1String("drawn_win_rate")].toDouble());
            updateRatingQuery.bindValue(QStringLiteral("ever_drawn_game_count"),ratingObject[QLatin1String("ever_drawn_game_count")].toInt());
            updateRatingQuery.bindValue(QStringLiteral("ever_drawn_win_rate"),ratingObject[QLatin1String("ever_drawn_win_rate")].toDouble());
            updateRatingQuery.bindValue(QStringLiteral("never_drawn_game_count"),ratingObject[QLatin1String("never_drawn_game_count")].toInt());
            updateRatingQuery.bindValue(QStringLiteral("never_drawn_win_rate"),ratingObject[QLatin1String("never_drawn_win_rate")].toDouble());
            updateRatingQuery.bindValue(QStringLiteral("drawn_improvement_win_rate"),ratingObject[QLatin1String("drawn_improvement_win_rate")].toDouble());
            if(!updateRatingQuery.exec()){
                emit failed17LRatings();
                Q_ASSUME(workerdb.rollback());
                return;
            }
            oneFound=true;
        }
        if (!oneFound) {
            emit failed17LRatings();
            Q_ASSUME(workerdb.rollback());
            return;
        }
        if(!workerdb.commit()){
            emit failed17LRatings();
            Q_ASSUME(workerdb.rollback());
            return;
        }
        emit downloaded17LRatings(currSet);
        if (m_SLrequestQueue.size() + m_SLrequestOutstanding == 0)
            emit downloadedAll17LRatings();
        emit download17LRatingsProgress(m_SLrequestQueue.size() + m_SLrequestOutstanding);
    });
}

void Worker::uploadRatings(const QStringList &sets)
{
    for (const QString &set : sets) {
        /*auto cardsRange = qAsConst(m_ratingsTemplate).equal_range(set);
        if (cardsRange.first == m_ratingsTemplate.cend())
            continue;
        for (auto i = cardsRange.first; i != cardsRange.second; ++i) {
            const QUrl ratingUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/User/CustomDraftRating"));
            QNetworkRequest ratingReq(ratingUrl);
            ratingReq.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
            m_MTGAHrequestQueue.append(std::make_pair(*i, ratingReq));
        }*/
    }
    if (!m_requestTimer->isActive())
        m_requestTimer->start();
    emit ratingsUploadMaxProgress(m_MTGAHrequestQueue.size());
}

void Worker::processMTGAHrequestQueue()
{
    if (m_MTGAHrequestQueue.isEmpty())
        return;
    const std::pair<MtgahCard, QNetworkRequest> currReq = m_MTGAHrequestQueue.takeFirst();
    const MtgahCard currCard = currReq.first;
    QJsonObject cardData;
    cardData[QLatin1String("idArena")] = currCard.id_arena;
    if (currCard.note.isEmpty())
        cardData[QLatin1String("note")] = QJsonValue();
    else
        cardData[QLatin1String("note")] = currCard.note;
    if (currCard.rating < 0)
        cardData[QLatin1String("rating")] = QJsonValue();
    else
        cardData[QLatin1String("rating")] = currCard.rating;
    ++m_MTGAHrequestOutstanding;
    QNetworkReply *reply = m_nam->put(currReq.second, QJsonDocument(cardData).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::errorOccurred, this, std::bind(&Worker::failedUploadRating, this, currCard));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, [reply, this, currCard]() -> void {
        --m_MTGAHrequestOutstanding;
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << QStringLiteral("Failed: ") << currCard.name;
            return;
        }
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
            qDebug() << QStringLiteral("Failed: ") << currCard.name;
            emit failedUploadRating(currCard);
            return;
        }
        emit ratingUploaded(currCard.name);
        if (m_MTGAHrequestQueue.size() + m_MTGAHrequestOutstanding == 0)
            emit allRatingsUploaded();
        emit ratingsUploadProgress(m_MTGAHrequestQueue.size() + m_MTGAHrequestOutstanding);
    });
}
