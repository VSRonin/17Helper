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
#include "ui_mainwindow.h"
#include <QStandardItemModel>
void MainWindow::doLogin()
{
    ui->loginButton->setEnabled(false);
    ui->usernameEdit->setEnabled(false);
    ui->pwdEdit->setEnabled(false);
    m_worker->tryLogin(ui->usernameEdit->text(), ui->pwdEdit->text());
}

void MainWindow::doLogout()
{
    ui->logoutButton->setEnabled(false);
    m_worker->logOut();
}

void MainWindow::fillSets(const QStringList &sets)
{
    m_error &= ~MTGAHSetsError;
    m_setsModel->removeRows(0,m_setsModel->rowCount());
    Qt::CheckState checkState = Qt::Checked;
    for(int i=sets.size()-1;i>=0;--i){
        auto item = new QStandardItem;
        item->setData(sets.at(i),Qt::UserRole);
        item->setData(checkState, Qt::CheckStateRole);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        m_setsModel->insertRow(m_setsModel->rowCount(),item);
        checkState = Qt::Unchecked;
    }
    m_worker->downloadSetsScryfall();
    retranslateUi();
}

void MainWindow::fillSetNames(const QHash<QString,QString>& setNames)
{
    m_error &= ~ScryfallSetsError;
    for(int i=0, iEnd = m_setsModel->rowCount();i<iEnd;++i){
        const QString setToFind = m_setsModel->index(i,0).data(Qt::UserRole).toString();
        auto nameIter = setNames.constFind(setToFind);
        if(nameIter==setNames.constEnd())
            m_setsModel->setData(m_setsModel->index(i,0),setToFind);
        else
            m_setsModel->setData(m_setsModel->index(i,0),nameIter.value());
    }
    retranslateUi();
}

void MainWindow::onLogoutError()
{
    m_error |= LogoutError;
    ui->logoutButton->setEnabled(true);
    retranslateUi();
}

void MainWindow::onMTGAHSetsError()
{
    m_error |= MTGAHSetsError;
    ui->retryBasicDownloadButton->setEnabled(true);
    retranslateUi();
}

void MainWindow::onScryfallSetsError()
{
    m_error |= ScryfallSetsError;
    ui->retryBasicDownloadButton->setEnabled(true);
    retranslateUi();
}

void MainWindow::retrySetsDownload()
{
    ui->retryBasicDownloadButton->setEnabled(false);
    m_worker->downloadSetsMTGAH();
}

void MainWindow::onLogout()
{
    m_error &= ~LogoutError;
    ui->usernameEdit->setEnabled(true);
    ui->pwdEdit->setEnabled(true);
    toggleLoginLogoutButtons();
    disableSetsSection();
    retranslateUi();
}

void MainWindow::onLogin()
{
    m_error &= ~LoginError;
    toggleLoginLogoutButtons();
    enableSetsSection();
    retranslateUi();
}

void MainWindow::onLoginError()
{
    m_error |= LoginError;
    ui->loginButton->setEnabled(true);
    ui->usernameEdit->setEnabled(true);
    ui->pwdEdit->setEnabled(true);
    retranslateUi();
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , m_error(NoError)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_worker = new Worker(this);
    m_setsModel = new QStandardItemModel(this);
    m_setsModel->insertColumn(0);
    ui->setsView->setModel(m_setsModel);
    ui->logoutButton->hide();
    ui->errorLabel->hide();
    ui->retryBasicDownloadButton->hide();
    ui->formatsCombo->addItem(QString(), QStringLiteral("PremierDraft"));
    ui->formatsCombo->addItem(QString(), QStringLiteral("QuickDraft"));
    ui->formatsCombo->addItem(QString(), QStringLiteral("TradDraft"));
    ui->formatsCombo->addItem(QString(), QStringLiteral("Sealed"));
    ui->formatsCombo->addItem(QString(), QStringLiteral("TradSealed"));
    disableSetsSection();
    retranslateUi();

    connect(ui->loginButton,&QPushButton::clicked,this,&MainWindow::doLogin);
    connect(ui->logoutButton,&QPushButton::clicked,this,&MainWindow::doLogout);
    connect(ui->retryBasicDownloadButton,&QPushButton::clicked,this,&MainWindow::retrySetsDownload);
    connect(ui->allSetsButton,&QPushButton::clicked,this,&MainWindow::selectAllSets);
    connect(ui->noSetButton,&QPushButton::clicked,this,&MainWindow::selectNoSets);
    connect(m_worker,&Worker::setsMTGAH,this,&MainWindow::fillSets);
    connect(m_worker,&Worker::setsScryfall,this,&MainWindow::fillSetNames);
    connect(m_worker,&Worker::loggedIn,this,&MainWindow::onLogin);
    connect(m_worker,&Worker::loginFalied,this,&MainWindow::onLoginError);
    connect(m_worker,&Worker::loggedOut,this,&MainWindow::onLogout);
    connect(m_worker,&Worker::logoutFailed,this,&MainWindow::onLogoutError);


    connect(m_worker,&Worker::downloadSetsMTGAHFailed,this,[](){qDebug("downloadSetsMTGAH Failed");});
    connect(m_worker,&Worker::downloadSetsScryfallFailed,this,[](){qDebug("downloadSetsScryfall Failed");});

    m_worker->downloadSetsMTGAH();
}

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow::CurrentErrors MainWindow::errors() const {
    return m_error;
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
    ui->formatsCombo->setItemText(0, tr("Premier Draft"));
    ui->formatsCombo->setItemText(1, tr("Quick Draft"));
    ui->formatsCombo->setItemText(2, tr("Traditional Draft"));
    ui->formatsCombo->setItemText(3, tr("Sealed"));
    ui->formatsCombo->setItemText(4, tr("Traditional Sealed"));
    ui->retranslateUi(this);
    ui->errorLabel->setVisible(m_error != NoError);
    ui->retryBasicDownloadButton->setVisible(m_error & (MTGAHSetsError | ScryfallSetsError));
    QStringList errorStrings;
    if(m_error & LoginError)
        errorStrings.append(tr("Login Failed! Check your username, password or internet connection"));
    if(m_error & LogoutError)
        errorStrings.append(tr("Logout Failed! Check your internet connection"));
    ui->errorLabel->setText(errorStrings.join(QChar(QLatin1Char('\n'))));
}

void MainWindow::setSetsSectionEnabled(bool enabled)
{
    ui->setsGroup->setEnabled(enabled);
}

void MainWindow::setAllSetsSelection(Qt::CheckState check)
{
    for(int i=0, iEnd = m_setsModel->rowCount();i<iEnd;++i)
        m_setsModel->item(i)->setCheckState(check);
}

void MainWindow::toggleLoginLogoutButtons()
{
    for(QPushButton* button : {ui->loginButton, ui->logoutButton}){
        button->setVisible(!button->isVisible());
        button->setEnabled(true);
    }
}


