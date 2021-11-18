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
#include "offlinesqltable.h"
#include "globals.h"
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlRecord>
OfflineSqlTable::OfflineSqlTable(QObject *parent)
    : QAbstractTableModel(parent)
    , m_colCount(0)
    , m_rowCount(0)
{ }

int OfflineSqlTable::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_colCount;
}

int OfflineSqlTable::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_rowCount;
}
bool OfflineSqlTable::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return false;
    QSqlDatabase db = openDb(m_databaseName);
    QString selectQueryString = QLatin1String("UPDATE ") + db.driver()->escapeIdentifier(m_tableName, QSqlDriver::TableName) + QLatin1String(" SET ")
            + db.driver()->escapeIdentifier(m_headers.at(index.column()), QSqlDriver::FieldName) + QLatin1String("=? WHERE ");
    for (int i = 0; i < m_colCount; ++i) {
        if (i > 0)
            selectQueryString += QLatin1String("AND ");
        selectQueryString += db.driver()->escapeIdentifier(m_headers.at(i), QSqlDriver::FieldName) + QLatin1String("=? ");
    }
    QSqlQuery selectQuery(db);
    selectQuery.prepare(selectQueryString);
    selectQuery.addBindValue(value);
    for (int i = 0; i < m_colCount; ++i)
        selectQuery.addBindValue(data(index.sibling(index.row(), i)));
    if (selectQuery.exec()) {
        const int dataIndex = (index.row() * m_colCount) + index.column();
        m_data[dataIndex] = value;
        return true;
    }
    return false;
}

QVariant OfflineSqlTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical)
        return section + 1;
    if (section < 0 || section >= m_headers.size())
        return QVariant();
    return m_headers.at(section);
}
QVariant OfflineSqlTable::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const int dataIndex = (index.row() * m_colCount) + index.column();
        return m_data.at(dataIndex);
    }
    return QVariant();
}

void OfflineSqlTable::setTable(const QString &databaseName, const QString &tableName)
{
    m_databaseName = databaseName;
    m_tableName = tableName;
    select();
}

bool OfflineSqlTable::select()
{
    QSqlDatabase db = openDb(m_databaseName);
    QSqlQuery selectQuery(db);
    selectQuery.prepare(QLatin1String("SELECT * FROM ") + db.driver()->escapeIdentifier(m_tableName, QSqlDriver::TableName) + QLatin1String(" WHERE ")
                        + m_filter);
    if (!selectQuery.exec())
        return false;
    beginResetModel();
    m_colCount = m_rowCount = 0;
    m_data.clear();
    m_headers.clear();
    int newRowCount = 0;
    while (selectQuery.next()) {
        if (m_colCount == 0) {
            const QSqlRecord selectRecord = selectQuery.record();
            m_colCount = selectRecord.count();
            m_rowCount = qMax(0, selectQuery.size());
            m_headers.reserve(m_colCount);
            for (int i = 0; i < m_colCount; ++i)
                m_headers.append(selectRecord.fieldName(i));
            m_data.reserve(m_colCount * m_rowCount);
        }
        for (int i = 0; i < m_colCount; ++i)
            m_data.append(selectQuery.value(i));
        ++newRowCount;
    }
    if (m_rowCount == 0)
        m_rowCount = newRowCount;
    Q_ASSERT(m_rowCount == newRowCount);
    Q_ASSERT(m_rowCount * m_colCount == m_data.size());
    endResetModel();
    return true;
}

void OfflineSqlTable::setFilter(const QString &filter)
{
    m_filter = filter;
    select();
}
