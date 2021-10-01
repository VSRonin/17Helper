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

#ifndef RATINGSMODEL_H
#define RATINGSMODEL_H
#include <QAbstractTableModel>
#include <QMultiHash>
class MtgahCard;
class RatingsModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(RatingsModel)
public:
    enum RatingsModelColumns {
        rmcSet,
        rmcName,
        rmcArenaId,
        rmcRating,
        rmcNote

        ,
        rmcCount
    };
    explicit RatingsModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void setRatingsTemplate(QMultiHash<QString, MtgahCard> *tmplt);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    QMultiHash<QString, MtgahCard> *m_ratingsTemplate;
};

#endif
