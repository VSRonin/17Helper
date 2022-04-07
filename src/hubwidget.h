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

#ifndef HUBWIDGET_H
#define HUBWIDGET_H

#include "translatablewidgetinterface.h"
#include <mainobject.h>
class QStackedWidget;
class LogInPage;
class InitialisationPage;
class DownloadMTGATemplatePage;
class DownloadOptionsPage;
class QPushButton;
class LogOutPage;
class DownloadProgressPage;
class UploadOptionsPage;
class CustomRatingsPage;
class HubWidget : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(HubWidget);

public:
    explicit HubWidget(QWidget *parent = nullptr);
    ~HubWidget();

protected:
    void retranslateUi() override;
private slots:
    void onSetsMTGAHDownloaded();
    void onLoggedIn();
    void onCustomRatingsTemplateDown();
    void onLoggedOut();
    void onAttemptLogOut();
    void backToDownloadOptions();
    void onStartProgress(MainObject::Operations op);
    void nextToUploadOptions();
    void onCustomiseRatings();
    void onApplyCustomRatings();

private:
    enum StackPages {
        spInitPage,
        spLogInPage,
        spRtgTemplatePage,
        spDownloadOptPage,
        spLogOutPage,
        spDownloadProgressPage,
        spUploadOptPage,
        spCustomRatingsPage
    };
    MainObject *m_object;
    QStackedWidget *m_stack;
    LogInPage *m_logInPage;
    InitialisationPage *m_initPage;
    DownloadMTGATemplatePage *m_downloadTemplatePage;
    DownloadOptionsPage *m_downloadOptionsPage;
    LogOutPage *m_logOutPage;
    DownloadProgressPage *m_downloadProgressPage;
    UploadOptionsPage *m_uploadOptionsPage;
    CustomRatingsPage *m_customRatingsPage;
};

#endif // HUBWIDGET_H
