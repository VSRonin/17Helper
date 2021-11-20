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
#include "ratingsmodel.h"

RatingsModel::RatingsModel(QObject *parent)
    : OfflineSqlTable(parent)
{ }

void RatingsModel::setTable(const QString &databaseName, const QString &)
{
    OfflineSqlTable::setTable(databaseName, QStringLiteral("Ratings"));
    select();
}

int RatingsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return OfflineSqlTable::columnCount(parent);
    return qMax(int(rmcCount), OfflineSqlTable::columnCount(parent));
}

QVariant RatingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical)
        return section + 1;
    switch (section) {
    case rmcSet:
        return tr("Set");
    case rmcName:
        return tr("Name");
    case rmcArenaId:
        return tr("Id Arena");
    case rmcRating:
        return tr("Rating");
    case rmcNote:
        return tr("Notes");
    default:
        return QVariant();
    }
}

Qt::ItemFlags RatingsModel::flags(const QModelIndex &index) const
{
    return OfflineSqlTable::flags(index) & ~Qt::ItemIsEditable;
}
