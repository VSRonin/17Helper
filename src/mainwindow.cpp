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
#include <QTranslator>
#include <configmanager.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "centralwidget.h"
#include <QMessageBox>
#include <QApplication>
#include <QLocale>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //connect(ui->centralwidget, &CentralWidget::updatedUploadedStatus, this, &MainWindow::onUpdatedUploadedStatus);
    connect(ui->actionEnglish, &QAction::triggered, this, std::bind(&MainWindow::onChangeLanguageAction, this, QLocale(QLocale::English)));
    connect(ui->actionItalian, &QAction::triggered, this, std::bind(&MainWindow::onChangeLanguageAction, this, QLocale(QLocale::Italian)));
    connect(ui->actionAbout_Qt, &QAction::triggered, this, &MainWindow::onAboutQt);
    ConfigManager cfgManager;
    QLocale lang = cfgManager.readLanguage();
    changeLanguage(lang);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onUpdatedUploadedStatus(const QString &card)
{
    if (card.isEmpty())
        ui->statusbar->clearMessage();
    else
        ui->statusbar->showMessage(tr("Uploaded %1").arg(card));
}

void MainWindow::changeLanguage(const QLocale &loc)
{
    for (auto i = translators.cbegin(), iEnd = translators.cend(); i != iEnd; ++i)
        qApp->removeTranslator(i->get());
    translators.clear();
    for (const QString &translationUnit : {QStringLiteral("qt"), QStringLiteral("17Helper"), QStringLiteral("17HelperUI")}) {
        std::shared_ptr<QTranslator> translator = std::make_shared<QTranslator>();
        if (translator->load(loc, translationUnit, QLatin1String("_"), QLatin1String("translations"))) {
            translators.append(translator);
            qApp->installTranslator(translators.last().get());
        }
    }
}

void MainWindow::onAboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::onChangeLanguageAction(const QLocale &loc)
{
    ConfigManager cfgManager;
    cfgManager.writeLanguage(loc);
    changeLanguage(loc);
}
