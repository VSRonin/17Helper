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

#ifndef DOWNLOADOPTIONSPAGE_H
#define DOWNLOADOPTIONSPAGE_H

#include "translatablewidgetinterface.h"
#include "globals.h"
namespace Ui {
class DownloadOptionsPage;
}
class MainObject;
class DownloadOptionsPage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DownloadOptionsPage)
public:
    explicit DownloadOptionsPage(QWidget *parent = nullptr);
    ~DownloadOptionsPage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);
signals:
    void logOut();

protected:
    void retranslateUi() override;
private slots:
    void onLoadDownloadFormat(const QString &format, GEnums::RatingTimeMethod timeMethod, const QDate &from, const QDate &to,
                              GEnums::RatingTimeScale timeScale, int timeSpan);
    void onShowOnlyDraftableSetsChanged(bool showOnly);
    void onFromEditChanged(const QDate fromDate);
    void onTimeRadioChanged();
    void onTodayCheckChanged();
    void selectAllSets();
    void selectNoSets();
    void checkDownloadButtonEnabled();
    void onDownloadClicked();

private:
    Ui::DownloadOptionsPage *ui;
    MainObject *m_object;
    QVector<QMetaObject::Connection> m_objectConnections;
};

#endif // DOWNLOADOPTIONSPAGE_H
