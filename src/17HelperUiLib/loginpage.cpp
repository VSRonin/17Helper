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

#include "loginpage.h"
#include "ui_loginpage.h"
#include <mainobject.h>
LogInPage::LogInPage(QWidget *parent)
    : TranslatableWidgetInterface(parent)
    , ui(new Ui::LogInPage)
    , m_object(nullptr)
{
    ui->setupUi(this);
    reset();
    connect(ui->remembePwdCheck, &QCheckBox::stateChanged, this, &LogInPage::onRememberPass);
    connect(ui->loginButton, &QPushButton::clicked, this, &LogInPage::doLogin);
    connect(ui->pwdEdit, &QLineEdit::textChanged, this, &LogInPage::checkLoginEnabled);
    connect(ui->usernameEdit, &QLineEdit::textChanged, this, &LogInPage::checkLoginEnabled);
}
void LogInPage::reset()
{
    ui->errorLabel->hide();
    ui->savePwdWarningLabel->hide();
    ui->remembePwdCheck->setChecked(false);
    enableAll(true);
}
void LogInPage::onLoadUserPass(const QString &userName, const QString &password)
{
    ui->usernameEdit->setText(userName);
    ui->pwdEdit->setText(password);
    ui->remembePwdCheck->setChecked(true);
}

void LogInPage::onLoginError(const QString &error)
{
    ui->errorLabel->setText(tr("Login Failed! Check your username, password or internet connection\n%1").arg(error));
    ui->errorLabel->show();
    enableAll(true);
}

void LogInPage::doLogin()
{
    enableAll(false);
    if (m_object)
        m_object->tryLogin(ui->usernameEdit->text(), ui->pwdEdit->text(), ui->remembePwdCheck->checkState() == Qt::Checked);
    ui->pwdEdit->clear();
}

void LogInPage::onLogin()
{
    ui->errorLabel->hide();
    enableAll(true);
}

void LogInPage::enableAll(bool enable)
{
    QWidget *widToEnable[] = {ui->remembePwdCheck, ui->usernameEdit, ui->pwdEdit};
    for (QWidget *wid : widToEnable)
        wid->setEnabled(enable);
    if (enable)
        ui->loginButton->setEnabled(enable);
    else
        checkLoginEnabled();
}

void LogInPage::checkLoginEnabled()
{
    ui->loginButton->setEnabled(m_object && !ui->usernameEdit->text().isEmpty() && !ui->pwdEdit->text().isEmpty());
}

void LogInPage::retranslateUi()
{
    ui->retranslateUi(this);
    if (ui->savePwdWarningLabel->isVisible() && m_object)
        ui->savePwdWarningLabel->setText(tr("Your password will be stored in plain text in %1").arg(m_object->configPath()));
}

MainObject *LogInPage::mainObject() const
{
    return m_object;
}

void LogInPage::setMainObject(MainObject *newObject)
{
    if (m_object == newObject)
        return;
    for (auto &&i : qAsConst(m_objectConnections))
        disconnect(i);
    m_objectConnections.clear();
    m_object = newObject;
    if (!m_object)
        return;
    m_objectConnections = QVector<QMetaObject::Connection>{connect(m_object, &MainObject::loadUserPass, this, &LogInPage::onLoadUserPass),
                                                           connect(m_object, &MainObject::loggedIn, this, &LogInPage::onLogin),
                                                           connect(m_object, &MainObject::loginFalied, this, &LogInPage::onLoginError)};
    checkLoginEnabled();
    onRememberPass(ui->remembePwdCheck->checkState());
}

void LogInPage::onRememberPass(int state)
{
    ui->savePwdWarningLabel->setVisible(state == Qt::Checked);
    if (m_object)
        ui->savePwdWarningLabel->setText(tr("Your password will be stored in plain text in %1").arg(m_object->configPath()));
}

LogInPage::~LogInPage()
{
    delete ui;
}
