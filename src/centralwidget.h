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

#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H
#include <QList>
#include <QWidget>
#include <QDateTime>
#include "mainobject.h"
namespace Ui {
class CentralWidget;
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

class CentralWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(CentralWidget)
public:
    explicit CentralWidget(QWidget *parent = nullptr);
    ~CentralWidget();
    enum CurrentError {
        NoError = 0x0,
        LoginError = 0x1,
        LogoutError = 0x2,
        MTGAHSetsError = 0x4,
        RatingTemplateFailed = 0x8,
        SLDownloadError = 0x10,
        RatingCalculationError = 0x20,
        UploadError = 0x40
    };
    Q_DECLARE_FLAGS(CurrentErrors, CurrentError)
    CurrentErrors errors() const;

protected:
    void changeEvent(QEvent *event) override;
    void retranslateUi();

private:
    CurrentErrors m_error;
    MainObject *m_object;
    Ui::CentralWidget *ui;
    QList<ProgressElement> progressQueue;
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
    void on17LandsDownloadFinished();
    void on17Lerror();
    void doMtgahUpload(bool clear);
    void onRememberPass(int state);
    void onLogin();
    void onLoginError(const QString &error);
    void onLogout();
    void onLogoutError(const QString &error);
    void selectAllSets() { setAllSetsSelection(Qt::Checked); }
    void selectNoSets() { setAllSetsSelection(Qt::Unchecked); }
    void retrySetsDownload();
    void retryTemplateDownload();
    void onCustomRatingsTemplateDownloaded();
    void updateRatingsFiler();
    void checkUploadButtonEnabled();
    void enableAll(bool enable);
    void onUploadedRatings();
    void onFailedUploadRating();
    void onLoadUserPass(const QString &userName, const QString &password);
    void onLoadDownloadFormat(const QString &format);
    void onLoadUploadRating(GEnums::SLMetrics ratingBase);
    void onInitialisationFailed();
    void onCustomRatingTemplateFailed();
    void onRatingsCalculationFailed();
    void onRatingsCalculated();
    void onDownloadSetsMTGAHFailed();
    void onSetsMTGAHDownloaded();
signals:
    void updatedUploadedStatus(const QString &card);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(CentralWidget::CurrentErrors);
#endif
