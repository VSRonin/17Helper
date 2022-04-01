#include "hubwidget.h"
#include "loginpage.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <mainobject.h>
#include <QCoreApplication>
#include <QMessageBox>
#include "initialisationpage.h"
#include "downloadmtgatemplatepage.h"
#include "downloadoptionspage.h"
#include <QPushButton>
HubWidget::HubWidget(QWidget *parent) :
    TranslatableWidgetInterface(parent),
    m_object(new MainObject(this))
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

    QVBoxLayout* mainLay= new QVBoxLayout(this);
    mainLay->addWidget(m_stack);

    connect(m_object, &MainObject::setsMTGAHDownloaded, this, &HubWidget::onSetsMTGAHDownloaded);
    connect(m_object, &MainObject::loggedIn, this, &HubWidget::onLoggedIn);
    connect(m_object, &MainObject::customRatingTemplate, this, &HubWidget::onCustomRatingsTemplateDown);
    QMetaObject::invokeMethod(this,&HubWidget::retranslateUi,Qt::QueuedConnection);
}

void HubWidget::onSetsMTGAHDownloaded(){
    m_stack->setCurrentIndex(spLogInPage);
}

void HubWidget::onLoggedIn(){
    m_stack->setCurrentIndex(spRtgTemplatePage);
}
void HubWidget::onCustomRatingsTemplateDown(){
    m_stack->setCurrentIndex(spDownloadOptPage);
}
void HubWidget::retranslateUi()
{
   m_object->retranslateModels();
}

HubWidget::~HubWidget()=default;
