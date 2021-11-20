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
#include "setsmodel.h"

SetsModel::SetsModel(QObject *parent)
    : OfflineSqlQueryModel(parent)
{ }

int SetsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return OfflineSqlQueryModel::columnCount(parent);
    return qMax(int(smcCount), OfflineSqlQueryModel::columnCount(parent));
}

QVariant SetsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return OfflineSqlQueryModel::headerData(section, orientation, role);
    switch (section) {
    case smcSetID:
        return tr("Set");
    case smcSetName:
        return tr("Name");
    case smcType:
        return tr("Type");
    default:
        return QVariant();
    }
}
