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

#include "hubwidget.h"
#include "loginpage.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCoreApplication>
#include <QMessageBox>
#include "initialisationpage.h"
#include "downloadmtgatemplatepage.h"
#include "downloadoptionspage.h"
#include "logoutpage.h"
#include "downloadprogresspage.h"
#include "uploadoptionspage.h"
#include "customratingspage.h"
#include <QPushButton>
HubWidget::HubWidget(QWidget *parent)
    : TranslatableWidgetInterface(parent)
    , m_object(new MainObject(this))
{
    m_stack = new QStackedWidget(this);
    m_initPage = new InitialisationPage(this);
    m_initPage->setMainObject(m_object);
    m_stack->addWidget(m_initPage);
    m_logInPage = new LogInPage(this);
    m_logInPage->setMainObject(m_object);
    m_stack->addWidget(m_logInPage);
    m_downloadTemplatePage = new DownloadMTGATemplatePage(this);
    m_downloadTemplatePage->setMainObject(m_object);
    m_stack->addWidget(m_downloadTemplatePage);
    m_downloadOptionsPage = new DownloadOptionsPage(this);
    m_downloadOptionsPage->setMainObject(m_object);
    m_stack->addWidget(m_downloadOptionsPage);
    m_logOutPage = new LogOutPage(this);
    m_logOutPage->setMainObject(m_object);
    m_stack->addWidget(m_logOutPage);
    m_downloadProgressPage = new DownloadProgressPage(this);
    m_downloadProgressPage->setMainObject(m_object);
    connect(m_downloadProgressPage, &DownloadProgressPage::goBack, this, &HubWidget::backToDownloadOptions);
    connect(m_downloadProgressPage, &DownloadProgressPage::goNext, this, &HubWidget::nextToUploadOptions);
    m_stack->addWidget(m_downloadProgressPage);
    m_uploadOptionsPage = new UploadOptionsPage(this);
    m_uploadOptionsPage->setMainObject(m_object);
    m_stack->addWidget(m_uploadOptionsPage);
    connect(m_uploadOptionsPage, &UploadOptionsPage::customiseRatings, this, &HubWidget::onCustomiseRatings);
    m_customRatingsPage = new CustomRatingsPage(this);
    m_customRatingsPage->setMainObject(m_object);
    m_stack->addWidget(m_customRatingsPage);
    connect(m_customRatingsPage, &CustomRatingsPage::applyCustomRatings, this, &HubWidget::onApplyCustomRatings);
    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->addWidget(m_stack);

    connect(m_object, &MainObject::setsMTGAHDownloaded, this, &HubWidget::onSetsMTGAHDownloaded);
    connect(m_object, &MainObject::loggedIn, this, &HubWidget::onLoggedIn);
    connect(m_object, &MainObject::loggedOut, this, &HubWidget::onLoggedOut);
    connect(m_object, &MainObject::customRatingTemplate, this, &HubWidget::onCustomRatingsTemplateDown);
    connect(m_object, &MainObject::startProgress, this, &HubWidget::onStartProgress);
    QMetaObject::invokeMethod(this, &HubWidget::retranslateUi, Qt::QueuedConnection);
}

void HubWidget::onCustomiseRatings()
{
    m_stack->setCurrentIndex(spCustomRatingsPage);
}

void HubWidget::onSetsMTGAHDownloaded()
{
    m_logInPage->reset();
    m_object->fetchLoginInfos();
    m_stack->setCurrentIndex(spLogInPage);
}

void HubWidget::onLoggedIn()
{
    m_stack->setCurrentIndex(spRtgTemplatePage);
}
void HubWidget::onCustomRatingsTemplateDown()
{
    m_object->fetchDownloadData();
    m_stack->setCurrentIndex(spDownloadOptPage);
}
void HubWidget::onLoggedOut()
{
    m_logInPage->reset();
    m_object->fetchLoginInfos();
    m_stack->setCurrentIndex(spLogInPage);
}

void HubWidget::onStartProgress(MainObject::Operations op)
{
    switch (op) {
    case MainObject::opLogOut:
        return onAttemptLogOut();
    case MainObject::opDownload17Ratings:
        m_downloadProgressPage->reset();
        m_stack->setCurrentIndex(spDownloadProgressPage);
    default:
        return;
    }
}

void HubWidget::onAttemptLogOut()
{
    m_logOutPage->reset();
    m_stack->setCurrentIndex(spLogOutPage);
    m_object->logOut();
}

void HubWidget::backToDownloadOptions()
{
    onCustomRatingsTemplateDown();
}
void HubWidget::nextToUploadOptions()
{
    m_stack->setCurrentIndex(spUploadOptPage);
}

void HubWidget::onApplyCustomRatings()
{
    m_stack->setCurrentIndex(spUploadOptPage);
}
void HubWidget::retranslateUi()
{
    m_object->retranslateModels();
}

HubWidget::~HubWidget() = default;
