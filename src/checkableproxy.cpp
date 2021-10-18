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
#include "checkableproxy.h"

CheckableProxy::CheckableProxy(QObject *parent)
    :RoleMaskProxyModel(parent)
{
    addMaskedRole(Qt::CheckStateRole);
}

Qt::ItemFlags CheckableProxy::flags(const QModelIndex &index) const
{
    return RoleMaskProxyModel::flags(index) | Qt::ItemIsUserCheckable;
}

QVariant CheckableProxy::data(const QModelIndex &index, int role) const
{
    const QVariant result = RoleMaskProxyModel::data(index,role);
    if(!result.isValid() && role==Qt::CheckStateRole)
        return Qt::Unchecked;
    return result;
}
