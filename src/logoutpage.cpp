#include "logoutpage.h"
#include "ui_logoutpage.h"
#include <mainobject.h>
LogOutPage::LogOutPage(QWidget *parent) :
    TranslatableWidgetInterface(parent),
    ui(new Ui::LogOutPage)
{
    ui->setupUi(this);
    reset();
    connect(ui->retryLogoutButton, &QPushButton::clicked, this, &LogOutPage::retryLogout);
}

LogOutPage::~LogOutPage()
{
    delete ui;
}

void LogOutPage::retranslateUi()
{
    ui->retranslateUi(this);
}

MainObject *LogOutPage::mainObject() const
{
    return m_object;
}

void LogOutPage::setMainObject(MainObject *newObject)
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
        connect(m_object, &MainObject::logoutFailed, this, &LogOutPage::onLogOutFailed)
    };
}

void LogOutPage::reset()
{
    ui->logoutErrorLabel->hide();
    ui->retryLogoutButton->hide();
    ui->logoutLabel->show();
}

void LogOutPage::onLogOutFailed(const QString& err){
    ui->logoutErrorLabel->setText(tr("Logout Failed! Check your internet connection\n%1").arg(err));
    ui->logoutErrorLabel->show();
    ui->retryLogoutButton->show();
    ui->logoutLabel->hide();
}
void LogOutPage::retryLogout(){
    reset();
    if(m_object)
        m_object->logOut();
}
