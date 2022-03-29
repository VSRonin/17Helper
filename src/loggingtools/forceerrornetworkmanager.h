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

#ifndef FORCEERRORNETWORKMANAGER_H
#define FORCEERRORNETWORKMANAGER_H
#include <QNetworkAccessManager>
#ifdef QT_DEBUG
class ForceErrorNetworkManager : public QNetworkAccessManager
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ForceErrorNetworkManager)
public:
    explicit ForceErrorNetworkManager(QObject *parent = nullptr);

protected:
    QNetworkReply *createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &originalReq, QIODevice *outgoingData = nullptr) override;
};

#endif
#endif