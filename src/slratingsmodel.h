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

#ifndef SLRATINGSMODEL_H
#define SLRATINGSMODEL_H
#include "offlinesqltable.h"
class SLRatingsModel : public OfflineSqlTable
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(SLRatingsModel)
public:
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    explicit SLRatingsModel(QObject *parent = nullptr);
    void setTable(const QString &databaseName, const QString &tableName = QString()) override;
    void setSLcodes(const QStringList &newSLcodes);

private:
    QStringList SLcodes;
};

#endif
