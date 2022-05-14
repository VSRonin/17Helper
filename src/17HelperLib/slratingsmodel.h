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
#include "globals.h"
class SHLIB_EXPORT SLRatingsModel : public OfflineSqlTable
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(SLRatingsModel)
public:
    enum SLRatingsModelColumns { slmName = 0, slmSet, slmLastUpdate = GEnums::SLCount + 2 };
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    explicit SLRatingsModel(QObject *parent = nullptr);
    void setTable(const QString &databaseName, const QString &tableName = QString()) override;
    void setSLcodes(const QStringList &newSLcodes);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QStringList SLcodes;
};

#endif
