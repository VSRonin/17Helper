/****************************************************************************\
   Copyright 2022 Luca Beldi
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

#ifndef UPLOADOPTIONSPAGE_H
#define UPLOADOPTIONSPAGE_H
#include <globals.h>
#include "translatablewidgetinterface.h"

namespace Ui {
class UploadOptionsPage;
}
class MainObject;
class NoCheckProxy;
class UploadOptionsPage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(UploadOptionsPage)
public:
    explicit UploadOptionsPage(QWidget *parent = nullptr);
    ~UploadOptionsPage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);
signals:
    void customiseRatings();

protected:
    void retranslateUi() override;
private slots:
    void onShowOnlyRatiosChanged(bool showOnly);
    void selectAllMetrics();
    void selectNoMetrics();
    void onLoadUploadRating(GEnums::SLMetrics ratingBase);
    void doMtgahUpload(bool clear);

private:
    Ui::UploadOptionsPage *ui;
    MainObject *m_object;
    QVector<QMetaObject::Connection> m_objectConnections;
    NoCheckProxy *m_SLMetricsProxy;
};

#endif // UPLOADOPTIONSPAGE_H
