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

#include "initialisationpage.h"
#include "ui_initialisationpage.h"
#include <mainobject.h>
InitialisationPage::InitialisationPage(QWidget *parent)
    : TranslatableWidgetInterface(parent)
    , ui(new Ui::InitialisationPage)
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
    if (m_object == newObject)
        return;
    for (auto &&i : qAsConst(m_objectConnections))
        disconnect(i);
    m_objectConnections.clear();
    m_object = newObject;
    if (!m_object)
        return;
    m_objectConnections = QVector<QMetaObject::Connection>{
            connect(m_object, &MainObject::initialisationFailed, this, &InitialisationPage::onInitialisationFailed),
            connect(m_object, &MainObject::downloadSetsMTGAHFailed, this, &InitialisationPage::onDownloadSetsMTGAHFailed)};
}

void InitialisationPage::reset()
{
    ui->setsErrorLabel->hide();
    ui->retrySetsButton->hide();
    ui->initErrorLabel->hide();
    ui->initLabel->show();
}

void InitialisationPage::onInitialisationFailed()
{
    ui->initErrorLabel->show();
    ui->initLabel->hide();
}
void InitialisationPage::onDownloadSetsMTGAHFailed()
{
    ui->setsErrorLabel->show();
    ui->retrySetsButton->show();
    ui->initLabel->hide();
}
void InitialisationPage::retrySetsDownload()
{
    reset();
    if (m_object)
        m_object->downloadSetsMTGAH();
}
