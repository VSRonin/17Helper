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

#ifndef CHECKABLEPROXY_H
#define CHECKABLEPROXY_H
#include <RoleMaskProxyModel>
#include "17helperlib_global.h"
class SHLIB_EXPORT CheckableProxy : public RoleMaskProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(CheckableProxy)
public:
    explicit CheckableProxy(QObject *parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void multiData(const QModelIndex &index, QModelRoleDataSpan roleDataSpan) const override;
};

#endif
