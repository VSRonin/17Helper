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

#ifndef SETSFILTERMODEL_H
#define SETSFILTERMODEL_H
#include <QSortFilterProxyModel>
#include "globals.h"
class SetsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(SetsFilterModel)
    enum { DraftableSet = GEnums::stcore | GEnums::stexpansion | GEnums::stmasters | GEnums::stdraft_innovation };

public:
    explicit SetsFilterModel(QObject *parent = nullptr);
    bool filterEnabled() const;
    void setFilterEnabled(bool newFilterEnabled);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    bool m_filterEnabled;
};

#endif
