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

#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include "translatablewidgetinterface.h"

namespace Ui {
class LogInPage;
}
class MainObject;
class LogInPage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(LogInPage)
public:
    explicit LogInPage(QWidget *parent = nullptr);
    ~LogInPage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);
    void reset();
private slots:
    void onRememberPass(int state);
    void onLogin();
    void doLogin();
    void onLoginError(const QString &error);
    void onLoadUserPass(const QString &userName, const QString &password);
    void checkLoginEnabled();

protected:
    void retranslateUi() override;

private:
    void enableAll(bool enable);
    Ui::LogInPage *ui;
    MainObject *m_object = nullptr;
    QVector<QMetaObject::Connection> m_objectConnections;
};

#endif // LOGINPAGE_H
