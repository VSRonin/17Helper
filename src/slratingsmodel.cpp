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
#include "slratingsmodel.h"
#include "worker.h"
SLRatingsModel::SLRatingsModel(QObject *parent)
    : OfflineSqlTable(parent)
{ }

void SLRatingsModel::setTable(const QString &databaseName, const QString &)
{
    OfflineSqlTable::setTable(databaseName, QStringLiteral("SLRatings"));
    select();
}

void SLRatingsModel::setSLcodes(const QStringList &newSLcodes)
{
    SLcodes = newSLcodes;
    emit headerDataChanged(Qt::Horizontal, 2, Worker::SLCount + 2);
}

int SLRatingsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return OfflineSqlTable::columnCount(parent);
    return qMax(int(Worker::SLCount) + 3, OfflineSqlTable::columnCount(parent));
}

QVariant SLRatingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical)
        return section + 1;
    switch (section) {
    case 0:
        return tr("Name", "Card Name");
    case 1:
        return tr("Set");
    case Worker::SLCount + 2:
        return tr("Last Update Date");
    default:
        if (section < 0 || section >= SLcodes.size() + 2)
            return QVariant();
        return SLcodes.at(section - 2);
    }
}

Qt::ItemFlags SLRatingsModel::flags(const QModelIndex &index) const
{
    return OfflineSqlTable::flags(index) & ~Qt::ItemIsEditable;
}
