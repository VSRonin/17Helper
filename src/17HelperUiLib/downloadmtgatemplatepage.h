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

#ifndef DOWNLOADMTGATEMPLATEPAGE_H
#define DOWNLOADMTGATEMPLATEPAGE_H

#include "translatablewidgetinterface.h"

namespace Ui {
class DownloadMTGATemplatePage;
}
class MainObject;
class DownloadMTGATemplatePage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DownloadMTGATemplatePage)
public:
    explicit DownloadMTGATemplatePage(QWidget *parent = nullptr);
    ~DownloadMTGATemplatePage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);
    void reset();

protected:
    void retranslateUi() override;
private slots:
    void onCustomRatingTemplateFailed();
    void retryTemplateDownload();

private:
    Ui::DownloadMTGATemplatePage *ui;
    MainObject *m_object = nullptr;
    QVector<QMetaObject::Connection> m_objectConnections;
};

#endif // DOWNLOADMTGATEMPLATEPAGE_H
