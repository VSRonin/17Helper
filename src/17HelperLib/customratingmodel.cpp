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
#include "customratingmodel.h"
#include "globals.h"
/****************************************************************************\
   Copyright 2022 Luca Beldi
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

#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QSqlDatabase>
CustomRatingModel::CustomRatingModel(QObject *parent)
    : OfflineSqlQueryModel(parent)
{ }

void CustomRatingModel::setQuery(const QString &databaseName)
{
    m_databaseName = databaseName;
    OfflineSqlQueryModel::setQuery(prepareQuery());
}

bool CustomRatingModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return false;
    if (index.column() != crmComment && index.column() != crmRating)
        return false;
    const QVariant commentVal = index.column() == crmComment ? value : index.sibling(index.row(), crmComment).data();
    const QVariant ratingVal = index.column() == crmRating ? value : index.sibling(index.row(), crmRating).data();
#ifdef QT_DEBUG
    qDebug() << commentVal;
    qDebug() << ratingVal;
#endif
    const int idArena = index.sibling(index.row(), crmIdArena).data().toInt();
    QSqlDatabase db = openDb(m_databaseName);
    QSqlQuery updateQuery(db);
    if ((ratingVal.isNull() || ratingVal.toInt() == -1) && commentVal.toString().isEmpty()) {
        Q_ASSUME(updateQuery.prepare(QStringLiteral("DELETE FROM [CustomRatings] WHERE [id_arena]=:id_arena")));
    } else {
        Q_ASSUME(updateQuery.prepare(
                QStringLiteral("INSERT OR REPLACE INTO [CustomRatings] ([id_arena], [rating], [note]) VALUES (:id_arena, :rating, :note)")));
        if (ratingVal.isNull() || ratingVal.toInt() == -1)
            updateQuery.bindValue(QStringLiteral(":rating"), QVariant(QMetaType(QMetaType::Int)));
        else
            updateQuery.bindValue(QStringLiteral(":rating"), ratingVal);
        if (commentVal.toString().isEmpty())
            updateQuery.bindValue(QStringLiteral(":note"), QVariant(QMetaType(QMetaType::QString)));
        else
            updateQuery.bindValue(QStringLiteral(":note"), commentVal);
    }
    updateQuery.bindValue(QStringLiteral(":id_arena"), idArena);
    if (updateQuery.exec()) {
        setInternalData(index, value);
        return true;
    }
    return false;
}

QVariant CustomRatingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return OfflineSqlQueryModel::headerData(section, orientation, role);
    switch (section) {
    case crmSet:
        return tr("Set");
    case crmName:
        return tr("Name");
    case crmComment:
        return tr("Notes");
    case crmRating:
        return tr("Rating");
    case crmIdArena:
        return tr("Id Arena");
    default:
        return QVariant();
    }
}

Qt::ItemFlags CustomRatingModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return OfflineSqlQueryModel::flags(index);
    if (index.column() != crmComment && index.column() != crmRating)
        return OfflineSqlQueryModel::flags(index);
    return OfflineSqlQueryModel::flags(index) | Qt::ItemIsEditable;
}

void CustomRatingModel::setFilter(const QString &filter)
{
    m_filter = filter;
    if (m_databaseName.isEmpty())
        return;
    OfflineSqlQueryModel::setQuery(prepareQuery());
}

int CustomRatingModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return OfflineSqlQueryModel::columnCount(parent);
    return qMax(int(crmCount), OfflineSqlQueryModel::columnCount(parent));
}

QSqlQuery CustomRatingModel::prepareQuery()
{
    QString queryString =
            QStringLiteral("SELECT [Ratings].[set], [Ratings].[name], [Ratings].[id_arena], [CustomRatings].[rating], [CustomRatings].[note] FROM "
                           "[Ratings] LEFT JOIN [CustomRatings] ON [Ratings].[id_arena]=[CustomRatings].[id_arena]");
    if (!m_filter.isEmpty())
        queryString += QLatin1String(" WHERE ") + m_filter;
    QSqlDatabase db = openDb(m_databaseName);
    QSqlQuery selectQuery(db);
    Q_ASSUME(selectQuery.prepare(queryString));
    return selectQuery;
}
