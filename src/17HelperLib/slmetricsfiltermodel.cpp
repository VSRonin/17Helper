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
#include "slmetricsfiltermodel.h"
#include "globals.h"
SLMetricsFilterModel::SLMetricsFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{ }

bool SLMetricsFilterModel::filterEnabled() const
{
    return m_filterEnabled;
}

void SLMetricsFilterModel::setFilterEnabled(bool newFilterEnabled)
{
    if (m_filterEnabled == newFilterEnabled)
        return;
    m_filterEnabled = newFilterEnabled;
    invalidateFilter();
}

bool SLMetricsFilterModel::isSLMetricRatio(int source_row) const
{
    switch (sourceModel()->index(source_row, 0).data(Qt::UserRole).toInt()) {
    case GEnums::SLavg_seen:
    case GEnums::SLavg_pick:
    case GEnums::SLwin_rate:
    case GEnums::SLopening_hand_win_rate:
    case GEnums::SLdrawn_win_rate:
    case GEnums::SLever_drawn_win_rate:
    case GEnums::SLnever_drawn_win_rate:
    case GEnums::SLdrawn_improvement_win_rate:
        return true;
    default:
        return false;
    }
}

bool SLMetricsFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (source_parent.isValid())
        return false;
    if (!m_filterEnabled)
        return true;
    return isSLMetricRatio(source_row);
}
