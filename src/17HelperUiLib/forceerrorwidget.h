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

#ifndef FORCEERRORWIDGET_H
#define FORCEERRORWIDGET_H
#include <QWidget>
#ifdef QT_DEBUG
#include "17helperuilib_global.h"
class SHUILIB_EXPORT ForceErrorWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ForceErrorWidget)
public:
    explicit ForceErrorWidget(QWidget *parent = nullptr);
signals:
    void start();
};

#endif
#endif
