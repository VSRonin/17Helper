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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QList>
#include <QWidget>
#include <QDateTime>
#include "mainobject.h"
namespace Ui {
class MainWindow;
}
class QStandardItemModel;
class RatingsModel;
class SeventeenCard;

struct ProgressElement
{
    MainObject::Operations m_operation;
    QString m_description;
    int m_min;
    int m_max;
    int m_val;
    ProgressElement();
    ProgressElement(MainObject::Operations operation, const QString &description, int max, int min, int val);
    ProgressElement(const ProgressElement &other) = default;
    ProgressElement &operator=(const ProgressElement &other) = default;
};

class MainWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MainWindow)
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    enum CurrentError {
        NoError = 0x0,
        LoginError = 0x1,
        LogoutError = 0x2,
        MTGAHSetsError = 0x4,
        RatingTemplateFailed = 0x8,
        InitialisationError = 0x10
    };
    Q_DECLARE_FLAGS(CurrentErrors, CurrentError)
    CurrentErrors errors() const;

protected:
    void changeEvent(QEvent *event) override;
    void retranslateUi();

private:
    CurrentErrors m_error;
    MainObject *m_object;
    Ui::MainWindow *ui;
    QList<ProgressElement> progressQueue;
    void setSetsSectionEnabled(bool enabled);
    void setAllSetsSelection(Qt::CheckState check);
private slots:
    void onStartProgress(MainObject::Operations op, const QString &description, int max, int min);
    void onUpdateProgress(MainObject::Operations op, int val);
    void onIncreaseProgress(MainObject::Operations op, int val);
    void onEndProgress(MainObject::Operations op);
    void toggleLoginLogoutButtons();
    void doLogin();
    void doLogout();
    void do17Ldownload();
    void doMtgahUpload();
    void fillSets(const QStringList &sets);
    void fillSetNames(const QHash<QString, QString> &setNames);
    void onDownloaded17LRatings(const QString &set, const QSet<SeventeenCard> &ratings);
    void onDownloadedAll17LRatings();
    void onDownload17LRatingsProgress(int progress);
    void enableSetsSection() { setSetsSectionEnabled(true); }
    void disableSetsSection() { setSetsSectionEnabled(false); }
    void onLogin();
    void onLoginError(const QString &error);
    void onLogout();
    void onLogoutError(const QString &error);
    void onMTGAHSetsError();
    void onScryfallSetsError();
    void onTemplateDownloadFailed();
    void selectAllSets() { setAllSetsSelection(Qt::Checked); }
    void selectNoSets() { setAllSetsSelection(Qt::Unchecked); }
    void retrySetsDownload();
    void retryTemplateDownload();
    void onCustomRatingsTemplateDownloaded();
    void updateRatingsFiler();
    void onAllRatingsUploaded();
};
Q_DECLARE_OPERATORS_FOR_FLAGS(MainWindow::CurrentErrors);
#endif
