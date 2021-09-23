#include "worker.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#ifdef QT_DEBUG
#include <QDebug>
#endif
Worker::Worker(QObject *parent)
    : m_nam(new QNetworkAccessManager(this))
{

}

void Worker::tryLogin(const QString &userName, const QString &password)
{
    if(userName.isEmpty() || password.isEmpty()){
        emit loginFalied();
        return;
    }
    const QUrl loginUrl = QUrl::fromUserInput(
        QStringLiteral("https://mtgahelper.com/api/Account/Signin?email=") + userName + QStringLiteral("&password=") + password
    );
    QNetworkReply* reply = m_nam->get(QNetworkRequest(loginUrl));
    connect(reply,&QNetworkReply::errorOccurred,this,&Worker::loginFalied);
    connect(reply,&QNetworkReply::finished,reply,&QNetworkReply::deleteLater);
    connect(reply,&QNetworkReply::finished,this,[reply,this]()->void{
        QJsonParseError parseErr;
        QJsonDocument loginDocument = QJsonDocument::fromJson(reply->readAll(),&parseErr);
        if(parseErr.error != QJsonParseError::NoError || !loginDocument.isObject()){
            emit loginFalied();
            return;
        }
        QJsonObject loginObject = loginDocument.object();
        if(!loginObject[QLatin1String("isAuthenticated")].toBool(false)){
            emit loginFalied();
            return;
        }
        emit loggedIn();
    });

}

void Worker::logOut()
{
    const QUrl setsUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/Account/Signout"));
    QNetworkReply* reply = m_nam->post(QNetworkRequest(setsUrl),QByteArray());
    connect(reply,&QNetworkReply::finished,reply,&QNetworkReply::deleteLater);
    connect(reply,&QNetworkReply::errorOccurred,this,&Worker::logoutFailed);
    connect(reply,&QNetworkReply::finished,this,[reply,this]()->void{
        if(reply->error()==QNetworkReply::NoError)
            emit loggedOut();
    });
}

void Worker::downloadSetsMTGAH()
{
    const QUrl setsUrl = QUrl::fromUserInput(QStringLiteral("https://mtgahelper.com/api/Misc/Sets"));
    QNetworkReply* reply = m_nam->get(QNetworkRequest(setsUrl));
    connect(reply,&QNetworkReply::errorOccurred,this,&Worker::downloadSetsMTGAHFailed);
    connect(reply,&QNetworkReply::finished,reply,&QNetworkReply::deleteLater);
    connect(reply,&QNetworkReply::finished,this,[reply,this]()->void{
        if(reply->error()!=QNetworkReply::NoError)
            return;
        QJsonParseError parseErr;
        QJsonDocument setsDocument = QJsonDocument::fromJson(reply->readAll(),&parseErr);
        if(parseErr.error != QJsonParseError::NoError || !setsDocument.isObject()){
            emit downloadSetsMTGAHFailed();
            return;
        }
        QJsonObject setsObject = setsDocument.object();
        QJsonValue setsArrVal = setsObject[QLatin1String("sets")];
        if(!setsArrVal.isArray()){
            emit downloadSetsMTGAHFailed();
            return;
        }
        QJsonArray setsArray = setsArrVal.toArray();
        QStringList setList;
        for(auto i = setsArray.cbegin(), iEnd=setsArray.cend();i!=iEnd;++i){
            const QString setStr = i->toObject()[QLatin1String("name")].toString().toUpper();
            if(!setStr.isEmpty())
                setList.append(setStr.toUpper());
        }
        if(setList.isEmpty()){
            emit downloadSetsMTGAHFailed();
            return;
        }
        for(auto i = setList.begin();i!=setList.end();++i){
            for(auto j = i+1;j!=setList.end();){
                if(*i == *j)
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
    QNetworkReply* reply = m_nam->get(QNetworkRequest(setsUrl));
    connect(reply,&QNetworkReply::errorOccurred,this,&Worker::downloadSetsMTGAHFailed);
    connect(reply,&QNetworkReply::finished,reply,&QNetworkReply::deleteLater);
    connect(reply,&QNetworkReply::finished,this,[reply,this]()->void{
        if(reply->error()!=QNetworkReply::NoError)
            return;
        QJsonParseError parseErr;
        const QJsonDocument setsDocument = QJsonDocument::fromJson(reply->readAll(),&parseErr);
        if(parseErr.error != QJsonParseError::NoError || !setsDocument.isObject()){
            emit downloadSetsScryfallFailed();
            return;
        }
        const QJsonObject setsObject = setsDocument.object();
        const QJsonValue setsArrVal = setsObject[QLatin1String("data")];
        if(!setsArrVal.isArray()){
            emit downloadSetsScryfallFailed();
            return;
        }
        const QJsonArray setsArray = setsArrVal.toArray();
        QHash<QString,QString> setNames;
        for(auto i = setsArray.cbegin(), iEnd=setsArray.cend();i!=iEnd;++i){
            const QJsonObject setObj = i->toObject();
            const QString setStr = setObj[QLatin1String("arena_code")].toString().trimmed().toUpper();
            if(setStr.isEmpty())
                continue;
            if(!setNames.contains(setStr))
                setNames.insert(setStr, setObj[QLatin1String("name")].toString().trimmed());
        }
        if(setNames.isEmpty()){
            emit downloadSetsScryfallFailed();
            return;
        }
        emit setsScryfall(setNames);
    });
}

