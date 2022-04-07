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

#ifndef DOWNLOADPROGRESSPAGE_H
#define DOWNLOADPROGRESSPAGE_H

#include "translatablewidgetinterface.h"
#include <mainobject.h>
namespace Ui {
class DownloadProgressPage;
}
class QStringListModel;
class DownloadProgressPage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DownloadProgressPage)
public:
    explicit DownloadProgressPage(QWidget *parent = nullptr);
    ~DownloadProgressPage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);
    void reset();

protected:
    void retranslateUi() override;
signals:
    void goBack();
    void goNext();
private slots:
    void onStartProgress(MainObject::Operations op, const QString &description, int max, int min);
    void onUpdateProgress(MainObject::Operations op, int val);
    void onIncreaseProgress(MainObject::Operations op, int val);
    void onEndProgress(MainObject::Operations op);
    void onSLDownloadFinished();
    void onSLDownloadFailed();
    void onNo17LRating(const QStringList &sets);

private:
    QStringListModel *m_noRatingsModel;
    Ui::DownloadProgressPage *ui;
    MainObject *m_object;
    QVector<QMetaObject::Connection> m_objectConnections;
};

#endif // DOWNLOADPROGRESSPAGE_H
