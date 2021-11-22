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

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QJsonObject>
class QFile;
class ConfigManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ConfigManager)
public:
    explicit ConfigManager(QObject *parent = nullptr);
    static QString configFilePath();
    void writeUserPass(const QString &userName, const QString &password);

private:
    QJsonObject getConfigObject() const;
    bool writeConfigObject(const QJsonObject &configObject) const;
    QFile *m_configFile;
};
#endif
