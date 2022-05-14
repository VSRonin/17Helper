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
#include "setsfiltermodel.h"
#include "setsmodel.h"
SetsFilterModel::SetsFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{ }

bool SetsFilterModel::filterEnabled() const
{
    return m_filterEnabled;
}

void SetsFilterModel::setFilterEnabled(bool newFilterEnabled)
{
    if (m_filterEnabled == newFilterEnabled)
        return;
    m_filterEnabled = newFilterEnabled;
    invalidateFilter();
}

bool SetsFilterModel::isDaraftable(int source_row) const
{
    return sourceModel()->index(source_row, SetsModel::smcType).data().toInt() & DraftableSet;
}

bool SetsFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (source_parent.isValid())
        return false;
    if (!m_filterEnabled)
        return true;
    return isDaraftable(source_row);
}
