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
#include "configmanager.h"
#include "globals.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_configFile(new QFile(configFilePath(), this))
{ }

QString ConfigManager::configFilePath()
{
    return appSettingsPath() + QLatin1String("/17helperconfig.json");
}

bool ConfigManager::writeUserPass(const QString &userName, const QString &password)
{
    QJsonObject configObject = getConfigObject();
    if (userName.isEmpty())
        configObject.remove(QLatin1String("username"));
    else
        configObject[QLatin1String("username")] = userName;
    if (password.isEmpty())
        configObject.remove(QLatin1String("password"));
    else
        configObject[QLatin1String("password")] = password;
    return writeConfigObject(configObject);
}

bool ConfigManager::writeDataToDownload(const QString &format, const QStringList &sets)
{
    QJsonObject configObject = getConfigObject();
    if (format.isEmpty())
        configObject.remove(QLatin1String("SLformat"));
    else
        configObject[QLatin1String("SLformat")] = format;
    if (format.isEmpty())
        configObject.remove(QLatin1String("SLsets"));
    else {
        QJsonArray setsArray;
        for (const QString &set : sets)
            setsArray.append(set);
        configObject[QLatin1String("SLsets")] = setsArray;
    }
    return writeConfigObject(configObject);
}

bool ConfigManager::writeDataToUpload(GEnums::SLMetrics ratingBase, const QVector<GEnums::SLMetrics> &commentMetrics)
{
    QJsonObject configObject = getConfigObject();
    if (ratingBase == GEnums::SLCount)
        configObject.remove(QLatin1String("RatingBase"));
    else
        configObject[QLatin1String("RatingBase")] = int(ratingBase);
    if (commentMetrics.isEmpty())
        configObject.remove(QLatin1String("CommentMetrics"));
    else {
        QJsonArray commentsArray;
        for (GEnums::SLMetrics comment : commentMetrics)
            commentsArray.append(int(comment));
        configObject[QLatin1String("CommentMetrics")] = commentsArray;
    }
    return writeConfigObject(configObject);
}

std::pair<QString, QString> ConfigManager::readUserPass()
{
    QJsonObject configObject = getConfigObject();
    return std::make_pair(configObject.value(QLatin1String("username")).toString(), configObject.value(QLatin1String("password")).toString());
}

std::pair<QString, QStringList> ConfigManager::readDataToDownload()
{
    QJsonObject configObject = getConfigObject();
    const QJsonArray setsArray = configObject.value(QLatin1String("SLsets")).toArray();
    QStringList sets;
    for (const QJsonValue &i : setsArray)
        sets.append(i.toString());
    return std::make_pair(configObject.value(QLatin1String("SLformat")).toString(), sets);
}

std::pair<GEnums::SLMetrics, QVector<GEnums::SLMetrics>> ConfigManager::readDataToUpload()
{
    QJsonObject configObject = getConfigObject();
    const auto ratingBaseValue = configObject.constFind(QLatin1String("RatingBase"));
    if (ratingBaseValue == configObject.constEnd())
        return std::make_pair(GEnums::SLCount, QVector<GEnums::SLMetrics>());
    const QJsonArray setsArray = configObject.value(QLatin1String("CommentMetrics")).toArray();
    QVector<GEnums::SLMetrics> comments;
    comments.reserve(setsArray.size());
    for (const QJsonValue &i : setsArray)
        comments.append(static_cast<GEnums::SLMetrics>(i.toInt()));
    return std::make_pair(static_cast<GEnums::SLMetrics>(ratingBaseValue->toInt()), comments);
}

QJsonObject ConfigManager::getConfigObject() const
{
    if (m_configFile->exists()) {
        Q_ASSUME(m_configFile->open(QIODevice::ReadOnly));
        QJsonObject configObject = QJsonDocument::fromJson(m_configFile->readAll()).object();
        m_configFile->close();
        return configObject;
    }
    return QJsonObject();
}

bool ConfigManager::writeConfigObject(const QJsonObject &configObject) const
{
    if (!m_configFile->open(QIODevice::WriteOnly))
        return false;
    const QByteArray configArray = QJsonDocument(configObject).toJson(QJsonDocument::Indented);
    const bool result = m_configFile->write(configArray) == configArray.size();
    m_configFile->close();
    return result;
}
