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

#include "logoutpage.h"
#include "ui_logoutpage.h"
#include <mainobject.h>
LogOutPage::LogOutPage(QWidget *parent)
    : TranslatableWidgetInterface(parent)
    , ui(new Ui::LogOutPage)
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
    if (m_object == newObject)
        return;
    for (auto &&i : qAsConst(m_objectConnections))
        disconnect(i);
    m_objectConnections.clear();
    m_object = newObject;
    if (!m_object)
        return;
    m_objectConnections = QVector<QMetaObject::Connection>{connect(m_object, &MainObject::logoutFailed, this, &LogOutPage::onLogOutFailed)};
}

void LogOutPage::reset()
{
    ui->logoutErrorLabel->hide();
    ui->retryLogoutButton->hide();
    ui->logoutLabel->show();
}

void LogOutPage::onLogOutFailed(const QString &err)
{
    ui->logoutErrorLabel->setText(tr("Logout Failed! Check your internet connection\n%1").arg(err));
    ui->logoutErrorLabel->show();
    ui->retryLogoutButton->show();
    ui->logoutLabel->hide();
}
void LogOutPage::retryLogout()
{
    reset();
    if (m_object)
        m_object->logOut();
}
