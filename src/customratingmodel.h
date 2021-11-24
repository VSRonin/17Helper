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

#ifndef CUSTOMRATINGMODEL_H
#define CUSTOMRATINGMODEL_H
#include "offlinesqlquerymodel.h"
#include <QVariant>
class CustomRatingModel : public OfflineSqlQueryModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(CustomRatingModel)
public:
    enum CustomRatingModelColumns {
        crmSet,
        crmName,
        crmIdArena,
        crmRating,
        crmComment,

        crmCount
    };
    explicit CustomRatingModel(QObject *parent = nullptr);
    void setQuery(const QString &databaseName);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    int columnCount(const QModelIndex &parent) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual void setFilter(const QString &filter);

private:
    QSqlQuery prepareQuery();
    QString m_filter;
    QString m_databaseName;
};

#endif
