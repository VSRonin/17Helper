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

void SetsModel::setQuery(const QSqlDatabase &db)
{
    QSqlQuery setsQuery(db);
    setsQuery.prepare(QStringLiteral(
            "select [name], [id], [type], [parent_set] from ( SELECT [id], CASE WHEN [name] is NULL then [id] ELSE [name] END as [name], CASE WHEN "
            "[release_date] is  NULL then DATE() ELSE [release_date] END as [release_date], CASE WHEN [type] is  NULL then 1 ELSE [type] END as "
            "[type], CASE WHEN [parent_set] IS NULL THEN [id] ELSE [parent_set] END as [parent_set] FROM [Sets]) order by [release_date] desc"));
    OfflineSqlQueryModel::setQuery(std::move(setsQuery));
}

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
    case smcParentSet:
        return tr("Parent Set");
    default:
        return QVariant();
    }
}
