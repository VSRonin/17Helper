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
#include "forceerrornetworkmanager.h"
#ifdef QT_DEBUG
#    include "globals.h"
ForceErrorNetworkManager::ForceErrorNetworkManager(QObject *parent)
    : QNetworkAccessManager(parent)
{ }

QNetworkReply *ForceErrorNetworkManager::createRequest(Operation op, const QNetworkRequest &originalReq, QIODevice *outgoingData)
{
    QNetworkRequest adjReq = originalReq;
    if ((dtFailCustomRatingTemplate && originalReq.url().toString().endsWith(QStringLiteral("customDraftRatingsForDisplay")))
        || (dtFail17LRatings && originalReq.url().toString().contains(QStringLiteral("17lands.com")))
        || (dtFailLogin && originalReq.url().toString().contains(QStringLiteral("Signin")))
        || (dtFailLogout && originalReq.url().toString().contains(QStringLiteral("Signout")))
        || (dtFailUploadRating && originalReq.url().toString().endsWith(QStringLiteral("CustomDraftRating")))
        || (dtFailSetsMTGAH && originalReq.url().toString().endsWith(QStringLiteral("api/Misc/Sets")))
        || (dtFailSetsScryfall && originalReq.url().toString().contains(QStringLiteral("scryfall.com"))))
        adjReq.setUrl(QUrl());
    return QNetworkAccessManager::createRequest(op, adjReq, outgoingData);
}
#endif
