#include "initialisationpage.h"
#include "ui_initialisationpage.h"
#include <mainobject.h>
InitialisationPage::InitialisationPage(QWidget *parent) :
    TranslatableWidgetInterface(parent),
    ui(new Ui::InitialisationPage)
{
    ui->setupUi(this);
    reset();
    connect(ui->retrySetsButton, &QPushButton::clicked, this, &InitialisationPage::retrySetsDownload);
}

InitialisationPage::~InitialisationPage()
{
    delete ui;
}

void InitialisationPage::retranslateUi()
{
    ui->retranslateUi(this);
}

MainObject *InitialisationPage::mainObject() const
{
    return m_object;
}

void InitialisationPage::setMainObject(MainObject *newObject)
{
    if(m_object == newObject)
        return;
    for(auto&& i : qAsConst(m_objectConnections))
        disconnect(i);
    m_objectConnections.clear();
    m_object = newObject;
    if(!m_object)
        return;
    m_objectConnections = QVector<QMetaObject::Connection>{
        connect(m_object, &MainObject::initialisationFailed, this, &InitialisationPage::onInitialisationFailed)
     ,connect(m_object, &MainObject::downloadSetsMTGAHFailed, this, &InitialisationPage::onDownloadSetsMTGAHFailed)
};
}

void InitialisationPage::reset()
{
    ui->setsErrorLabel->hide();
    ui->retrySetsButton->hide();
    ui->initErrorLabel->hide();
    ui->initLabel->show();
}

void InitialisationPage::onInitialisationFailed(){
    ui->initErrorLabel->show();
    ui->initLabel->hide();
}
void InitialisationPage::onDownloadSetsMTGAHFailed(){
    ui->setsErrorLabel->show();
    ui->retrySetsButton->show();
    ui->initLabel->hide();
}
void InitialisationPage::retrySetsDownload(){
    reset();
    if(m_object)
        m_object->downloadSetsMTGAH();
}
