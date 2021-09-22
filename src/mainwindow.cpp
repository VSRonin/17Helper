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
#include <QEvent>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QListView>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QPushButton>
void MainWindow::fillSets()
{
    m_setsModel->insertColumn(0);
    m_setsModel->insertRows(0,4);
    for(int i=0;i<m_setsModel->rowCount();++i)
        m_setsModel->setItem(i,0,new QStandardItem);
    int i=0;
    m_setsModel->item(i++)->setData(QStringLiteral("MID"), Qt::UserRole);
    m_setsModel->item(i++)->setData(QStringLiteral("AFR"), Qt::UserRole);
    m_setsModel->item(i++)->setData(QStringLiteral("STX"), Qt::UserRole);
    m_setsModel->item(i++)->setData(QStringLiteral("KHM"), Qt::UserRole);
    for(i=0;i<m_setsModel->rowCount();++i){
        QStandardItem* item = m_setsModel->item(i);
        item->setData(i==0 ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    }
}

void MainWindow::translateSets()
{
    int i=0;
    m_setsModel->item(i++)->setData(tr("Innistrad: Midnight Hunt"), Qt::DisplayRole);
    m_setsModel->item(i++)->setData(tr("D&D: Adventures in the Forgotten Realms"), Qt::DisplayRole);
    m_setsModel->item(i++)->setData(tr("Strixhaven: School of Mages"), Qt::DisplayRole);
    m_setsModel->item(i++)->setData(tr("Kaldheim"), Qt::DisplayRole);
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    m_mtgahelperGroup = new QGroupBox(this);
    m_tokenLabel = new QLabel(this);
    m_tokenEdit = new QLineEdit(this);
    QFormLayout *mtgahelperLay = new QFormLayout(m_mtgahelperGroup);
    mtgahelperLay->addRow(m_tokenLabel, m_tokenEdit);

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
    m_setsModel = new QStandardItemModel(this);
    fillSets();
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
    m_tokenLabel->setText(tr("MTGAHelper Token"));
    m_downloadGroup->setTitle(tr("Data to Download"));
    m_formatLabel->setText(tr("Format"));
    m_formatCombo->setItemText(0, tr("Premier Draft"));
    m_formatCombo->setItemText(1, tr("Quick Draft"));
    m_formatCombo->setItemText(2, tr("Traditional Draft"));
    m_formatCombo->setItemText(3, tr("Sealed"));
    m_formatCombo->setItemText(4, tr("Traditional Sealed"));
    m_setsLabel->setText(tr("Sets"));
    translateSets();
    m_startButton->setText(tr("Start"));
}


