#include "worker.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#ifdef QT_DEBUG
#    include <QDebug>
#endif
Worker::Worker(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_SLrequestOutstanding(0)
    , m_MTGAHrequestOutstanding(0)
{
    QTimer *requestTimer = new QTimer(this);
    requestTimer->setInterval(100);
    connect(requestTimer, &QTimer::timeout, this, &Worker::processSLrequestQueue);
    connect(requestTimer, &QTimer::timeout, this, &Worker::processMTGAHrequestQueue);
    requestTimer->start();
}

QMultiHash<QString, MtgahCard> *Worker::ratingsTemplate()
{
    return &m_ratingsTemplate;
}

void Worker::tryLogin(const QString &userName, const QString &password)
{
    if (userName.isEmpty() || password.isEmpty()) {
        emit loginFalied();
        return;
    }
    const QUrl loginUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/Account/Signin?email=") + userName
                                              + QStringLiteral("&password=") + password);
    QNetworkReply *reply = m_nam->get(QNetworkRequest(loginUrl));
    connect(reply, &QNetworkReply::errorOccurred, this, &Worker::loginFalied);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, [reply, this]() -> void {
        if (reply->error() != QNetworkReply::NoError)
            return;
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
            emit loginFalied();
            return;
        }
        QJsonParseError parseErr;
        QJsonDocument loginDocument = QJsonDocument::fromJson(reply->readAll(), &parseErr);
        if (parseErr.error != QJsonParseError::NoError || !loginDocument.isObject()) {
            emit loginFalied();
            return;
        }
        QJsonObject loginObject = loginDocument.object();
        if (!loginObject[QLatin1String("isAuthenticated")].toBool(false)) {
            emit loginFalied();
            return;
        }
        emit loggedIn();
    });
}

void Worker::logOut()
{
    const QUrl setsUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/Account/Signout"));
    QNetworkReply *reply = m_nam->post(QNetworkRequest(setsUrl), QByteArray());
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::errorOccurred, this, &Worker::logoutFailed);
    connect(reply, &QNetworkReply::finished, this, [reply, this]() -> void {
        if (reply->error() != QNetworkReply::NoError)
            return;
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
            emit logoutFailed();
            return;
        }
        emit loggedOut();
    });
}

void Worker::downloadSetsMTGAH()
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
            const QString setStr = i->toObject()[QLatin1String("name")].toString().toUpper();
            if (!setStr.isEmpty())
                setList.append(setStr.toUpper());
        }
        if (setList.isEmpty()) {
            emit downloadSetsMTGAHFailed();
            return;
        }
        for (auto i = setList.begin(); i != setList.end(); ++i) {
            for (auto j = i + 1; j != setList.end();) {
                if (*i == *j)
                    j = setList.erase(j);
                else
                    ++j;
            }
        }
        emit setsMTGAH(setList);
    });
}

void Worker::downloadSetsScryfall()
{
    const QUrl setsUrl = QUrl::fromUserInput(QStringLiteral("https://api.scryfall.com/sets"));
    QNetworkReply *reply = m_nam->get(QNetworkRequest(setsUrl));
    connect(reply, &QNetworkReply::errorOccurred, this, &Worker::downloadSetsScryfallFailed);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, [reply, this]() -> void {
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
        const QJsonArray setsArray = setsArrVal.toArray();
        QHash<QString, QString> setNames;
        for (auto i = setsArray.cbegin(), iEnd = setsArray.cend(); i != iEnd; ++i) {
            const QJsonObject setObj = i->toObject();
            const QString setStr = setObj[QLatin1String("code")].toString().trimmed().toUpper();
            if (setStr.isEmpty())
                continue;
            if (!setNames.contains(setStr))
                setNames.insert(setStr, setObj[QLatin1String("name")].toString().trimmed());
        }
        if (setNames.isEmpty()) {
            emit downloadSetsScryfallFailed();
            return;
        }
        emit setsScryfall(setNames);
    });
}

void Worker::getCustomRatingTemplate()
{
    const QUrl setsUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/User/customDraftRatingsForDisplay"));
    QNetworkReply *reply = m_nam->get(QNetworkRequest(setsUrl));
    connect(reply, &QNetworkReply::errorOccurred, this, &Worker::customRatingTemplateFailed);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    connect(reply, &QNetworkReply::finished, this, [reply, this]() -> void {
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
        decltype(m_ratingsTemplate) rtgsTemplate;
        const QJsonArray ratingsArray = ratingsDocument.array();
        for (auto i = ratingsArray.cbegin(), iEnd = ratingsArray.cend(); i != iEnd; ++i) {
            QCoreApplication::processEvents();
            if (!i->isObject())
                continue;
            const QJsonObject ratingObject = i->toObject();
            const QJsonObject cardObject = ratingObject[QLatin1String("card")].toObject();
            if (cardObject.isEmpty())
                continue;
            const int idArenaVal = cardObject[QLatin1String("idArena")].toInt();
            if (std::find_if(rtgsTemplate.cbegin(), rtgsTemplate.cend(),
                             [idArenaVal](const MtgahCard &card) -> bool { return idArenaVal == card.id_arena; })
                != rtgsTemplate.cend())
                continue;
            const QString setStr = cardObject[QLatin1String("set")].toString().trimmed().toUpper();
            if (setStr.isEmpty())
                continue;
            const QString nameStr = cardObject[QLatin1String("name")].toString();
            if (nameStr.isEmpty())
                continue;
            MtgahCard card;
            card.name = nameStr;
            card.id_arena = idArenaVal;
            card.set = setStr;
            const QJsonValue noteValue = ratingObject[QLatin1String("note")];
            if (!noteValue.isNull())
                card.note = noteValue.toString();
            const QJsonValue ratingValue = ratingObject[QLatin1String("rating")];
            if (!ratingValue.isNull())
                card.rating = ratingValue.toInt();
            rtgsTemplate.insert(setStr, card);
        }
        if (rtgsTemplate.isEmpty()) {
            emit customRatingTemplateFailed();
            return;
        }
        m_ratingsTemplate = rtgsTemplate;
        emit customRatingTemplate(m_ratingsTemplate);
    });
}

void Worker::get17LRatings(const QStringList &sets, const QString &format)
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
        QSet<SeventeenCard> rtgsList;
        const QJsonArray ratingsArray = ratingsDocument.array();
        for (auto i = ratingsArray.cbegin(), iEnd = ratingsArray.cend(); i != iEnd; ++i) {
            QCoreApplication::processEvents();
            if (!i->isObject())
                continue;
            const QJsonObject ratingObject = i->toObject();
            const QString nameStr = ratingObject[QLatin1String("name")].toString();
            if (nameStr.isEmpty())
                continue;
            SeventeenCard card;
            card.name = nameStr;
            card.seen_count = ratingObject[QLatin1String("seen_count")].toInt();
            card.avg_seen = ratingObject[QLatin1String("avg_seen")].toDouble();
            card.pick_count = ratingObject[QLatin1String("pick_count")].toInt();
            card.avg_pick = ratingObject[QLatin1String("avg_pick")].toDouble();
            card.game_count = ratingObject[QLatin1String("game_count")].toInt();
            card.win_rate = ratingObject[QLatin1String("win_rate")].toDouble();
            card.opening_hand_game_count = ratingObject[QLatin1String("opening_hand_game_count")].toInt();
            card.opening_hand_win_rate = ratingObject[QLatin1String("opening_hand_win_rate")].toDouble();
            card.drawn_game_count = ratingObject[QLatin1String("drawn_game_count")].toInt();
            card.drawn_win_rate = ratingObject[QLatin1String("drawn_win_rate")].toDouble();
            card.ever_drawn_game_count = ratingObject[QLatin1String("ever_drawn_game_count")].toInt();
            card.ever_drawn_win_rate = ratingObject[QLatin1String("ever_drawn_win_rate")].toDouble();
            card.never_drawn_game_count = ratingObject[QLatin1String("never_drawn_game_count")].toInt();
            card.never_drawn_win_rate = ratingObject[QLatin1String("never_drawn_win_rate")].toDouble();
            card.drawn_improvement_win_rate = ratingObject[QLatin1String("drawn_improvement_win_rate")].toDouble();
            rtgsList.insert(card);
        }
        if (rtgsList.isEmpty()) {
            emit failed17LRatings();
            return;
        }
        emit downloaded17LRatings(currSet, rtgsList);
        if (m_SLrequestQueue.size() + m_SLrequestOutstanding == 0)
            emit downloadedAll17LRatings();
        emit download17LRatingsProgress(m_SLrequestQueue.size() + m_SLrequestOutstanding);
    });
}

void Worker::uploadRatings(const QStringList &sets)
{
    for (const QString &set : sets) {
        auto cardsRange = qAsConst(m_ratingsTemplate).equal_range(set);
        if (cardsRange.first == m_ratingsTemplate.cend())
            continue;
        for (auto i = cardsRange.first; i != cardsRange.second; ++i) {
            const QUrl ratingUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/User/CustomDraftRating"));
            QNetworkRequest ratingReq(ratingUrl);
            ratingReq.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
            m_MTGAHrequestQueue.append(std::make_pair(*i, ratingReq));
        }
    }
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
        if (reply->error() != QNetworkReply::NoError){
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
