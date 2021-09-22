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

#include "mainwindow.h"
#include "worker.h"
#include <QEvent>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QListView>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QPushButton>

void MainWindow::doLogin()
{
    m_worker->tryLogin(m_usernameEdit->text(), m_pwdEdit->text());
}

void MainWindow::fillSets(const QStringList &sets)
{
    m_setsModel->removeRows(0,m_setsModel->rowCount());
    m_setsModel->insertRows(0,sets.size());
    for(int i=sets.size()-1;i>=0;--i){
        auto item = new QStandardItem;
        item->setData(sets.at(i),Qt::UserRole);
        item->setData(i==0 ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    }
    m_worker->downloadSetsScryfall(sets);
}

void MainWindow::fillSetNames(const QHash<QString,QString>& setNames)
{
    for(int i=0, iEnd = m_setsModel->rowCount();i<iEnd;++i){
        const QString setToFind = m_setsModel->index(i,0).data(Qt::UserRole).toString();
        auto nameIter = setNames.constFind(setToFind);
        if(nameIter==setNames.constEnd())
            m_setsModel->setData(m_setsModel->index(i,0),setToFind);
        else
            m_setsModel->setData(m_setsModel->index(i,0),nameIter.value());
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    m_worker = new Worker(this);

    m_mtgahelperGroup = new QGroupBox(this);
    m_usernameLabel = new QLabel(this);
    m_usernameEdit = new QLineEdit(this);
    m_pwdLabel = new QLabel(this);
    m_pwdEdit = new QLineEdit(this);
    m_pwdEdit->setEchoMode(QLineEdit::Password);
    m_pwdEdit->setInputMethodHints(Qt::ImhHiddenText | Qt::ImhSensitiveData | Qt::ImhNoAutoUppercase | Qt::ImhNoPredictiveText | Qt::ImhNoEditMenu);
    m_mtgahLoginButton = new QPushButton(this);
    QHBoxLayout *mtgahelperLay = new QHBoxLayout(m_mtgahelperGroup);
    mtgahelperLay->addWidget(m_usernameLabel);
    mtgahelperLay->addWidget(m_usernameEdit);
    mtgahelperLay->addWidget(m_pwdLabel);
    mtgahelperLay->addWidget(m_pwdEdit);
    mtgahelperLay->addWidget(m_mtgahLoginButton);

    m_downloadGroup = new QGroupBox(this);
    m_formatLabel = new QLabel(this);
    m_formatCombo = new QComboBox(this);
    m_formatCombo->addItem(QString(), QStringLiteral("PremierDraft"));
    m_formatCombo->addItem(QString(), QStringLiteral("QuickDraft"));
    m_formatCombo->addItem(QString(), QStringLiteral("TradDraft"));
    m_formatCombo->addItem(QString(), QStringLiteral("Sealed"));
    m_formatCombo->addItem(QString(), QStringLiteral("TradSealed"));
    m_setsLabel = new QLabel(this);
    m_setsView = new QListView(this);
    m_setsModel = new QStandardItemModel(0,1,this);
    m_setsView->setModel(m_setsModel);
    QGridLayout *downloadLay = new QGridLayout(m_downloadGroup);
    downloadLay->addWidget(m_formatLabel,0,0);
    downloadLay->addWidget(m_formatCombo,0,1);
    downloadLay->addWidget(m_setsLabel,1,0,1,2);
    downloadLay->addWidget(m_setsView,2,0,1,2);

    m_startButton = new QPushButton(this);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->addWidget(m_mtgahelperGroup);
    mainLay->addWidget(m_downloadGroup);
    mainLay->addWidget(m_startButton);
    retranslateUi();

    connect(m_mtgahLoginButton,&QPushButton::clicked,this,&MainWindow::doLogin);
    connect(m_worker,&Worker::loginFalied,this,[](){qDebug("Login Failed");});
    connect(m_worker,&Worker::loggedIn,this,[](){qDebug("Login Success");});
    connect(m_worker,&Worker::downloadSetsMTGAHFailed,this,[](){qDebug("downloadSetsMTGAH Failed");});
    connect(m_worker,&Worker::downloadSetsScryfallFailed,this,[](){qDebug("downloadSetsScryfall Failed");});
    connect(m_worker,&Worker::setsMTGAH,this,&MainWindow::fillSets);
    connect(m_worker,&Worker::setsScryfall,this,&MainWindow::fillSetNames);
    m_worker->downloadSetsMTGAH();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    else
        QWidget::changeEvent(event);
}

void MainWindow::retranslateUi()
{
    setWindowTitle(tr("17 Helper"));
    m_mtgahelperGroup->setTitle(tr("MTGAHelper"));
    m_usernameLabel->setText(tr("Email"));
    m_pwdLabel->setText(tr("Password"));
    m_mtgahLoginButton->setText(tr("Login"));
    m_downloadGroup->setTitle(tr("Data to Download"));
    m_formatLabel->setText(tr("Format"));
    m_formatCombo->setItemText(0, tr("Premier Draft"));
    m_formatCombo->setItemText(1, tr("Quick Draft"));
    m_formatCombo->setItemText(2, tr("Traditional Draft"));
    m_formatCombo->setItemText(3, tr("Sealed"));
    m_formatCombo->setItemText(4, tr("Traditional Sealed"));
    m_setsLabel->setText(tr("Sets"));
    m_startButton->setText(tr("Start"));
}


