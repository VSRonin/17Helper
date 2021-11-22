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
ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_configFile(new QFile(configFilePath(), this))
{ }

QString ConfigManager::configFilePath()
{
    return appSettingsPath() + QLatin1String("/17helperconfig.json");
}

void ConfigManager::writeUserPass(const QString &userName, const QString &password)
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
    writeConfigObject(configObject);
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
    const bool result = m_configFile->write(configArray) != configArray.size();
    m_configFile->close();
    return result;
}
