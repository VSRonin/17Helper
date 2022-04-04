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

#include "downloadoptionspage.h"
#include "ui_downloadoptionspage.h"
#include <mainobject.h>
#include <QButtonGroup>
DownloadOptionsPage::DownloadOptionsPage(QWidget *parent)
    : TranslatableWidgetInterface(parent)
    , ui(new Ui::DownloadOptionsPage)
{
    ui->setupUi(this);
    QButtonGroup *timeGroup = new QButtonGroup(this);
    timeGroup->addButton(ui->noLimitRadio);
    timeGroup->addButton(ui->timeSpanRadio);
    timeGroup->addButton(ui->lastPeriodRadio);
    const QDate currDate = QDate::currentDate();
    ui->toEdit->setMaximumDate(currDate);
    ui->toEdit->setDate(currDate);
    ui->fromEdit->setMaximumDate(currDate.addDays(-1));
    ui->periodCombo->addItem(QString(), GEnums::RatingTimeScale::rtsDays);
    ui->periodCombo->addItem(QString(), GEnums::RatingTimeScale::rtsWeeks);
    ui->periodCombo->addItem(QString(), GEnums::RatingTimeScale::rtsMonths);
    ui->periodCombo->addItem(QString(), GEnums::RatingTimeScale::rtsYears);

    connect(ui->fromEdit, &QDateEdit::dateChanged, this, &DownloadOptionsPage::onFromEditChanged);
    connect(ui->noLimitRadio, &QRadioButton::toggled, this, &DownloadOptionsPage::onTimeRadioChanged);
    connect(ui->timeSpanRadio, &QRadioButton::toggled, this, &DownloadOptionsPage::onTimeRadioChanged);
    connect(ui->lastPeriodRadio, &QRadioButton::toggled, this, &DownloadOptionsPage::onTimeRadioChanged);
    connect(ui->todayCheck, &QCheckBox::stateChanged, this, &DownloadOptionsPage::onTodayCheckChanged);
    connect(ui->allSetsButton, &QPushButton::clicked, this, &DownloadOptionsPage::selectAllSets);
    connect(ui->noSetsButton, &QPushButton::clicked, this, &DownloadOptionsPage::selectNoSets);
    connect(ui->downloadButton, &QPushButton::clicked, this, &DownloadOptionsPage::onDownloadClicked);

    QMetaObject::invokeMethod(this, &DownloadOptionsPage::retranslateUi, Qt::QueuedConnection);
}
void DownloadOptionsPage::selectAllSets()
{
    if (m_object)
        m_object->setAllSetsSelection(Qt::Checked);
}

void DownloadOptionsPage::selectNoSets()
{
    if (m_object)
        m_object->setAllSetsSelection(Qt::Unchecked);
}
DownloadOptionsPage::~DownloadOptionsPage()
{
    delete ui;
}
MainObject *DownloadOptionsPage::mainObject() const
{
    return m_object;
}

void DownloadOptionsPage::setMainObject(MainObject *newObject)
{
    if (m_object == newObject)
        return;
    for (auto &&i : qAsConst(m_objectConnections))
        disconnect(i);
    m_objectConnections.clear();
    m_object = newObject;
    if (m_object) {
        ui->setsView->setModel(m_object->setsModel());
        ui->formatCombo->setModel(m_object->formatsModel());
        m_objectConnections = QVector<QMetaObject::Connection>{
                connect(m_object, &MainObject::loadDownloadFormat, this, &DownloadOptionsPage::onLoadDownloadFormat),
                connect(m_object, &MainObject::showOnlyDraftableSetsChanged, this, &DownloadOptionsPage::onShowOnlyDraftableSetsChanged),
                connect(ui->draftableSetsCheck, &QCheckBox::clicked, m_object, &MainObject::showOnlyDraftableSets),
                connect(m_object->setsModel(), &QAbstractItemModel::dataChanged, this, &DownloadOptionsPage::checkDownloadButtonEnabled)};
    }
    checkDownloadButtonEnabled();
}

void DownloadOptionsPage::checkDownloadButtonEnabled()
{
    if (m_object)
        ui->downloadButton->setEnabled(m_object->oneSetSelected());
    else
        ui->downloadButton->setEnabled(false);
}

void DownloadOptionsPage::onDownloadClicked()
{
    if (m_object) {
        if (ui->noLimitRadio->isChecked())
            m_object->download17Lands(ui->formatCombo->currentData().toString());
        else if (ui->lastPeriodRadio->isChecked())
            m_object->download17Lands(ui->formatCombo->currentData().toString(), ui->periodCombo->currentData().value<GEnums::RatingTimeScale>(),
                                      ui->periodSpin->value());
        else if (ui->timeSpanRadio->isChecked())
            m_object->download17Lands(ui->formatCombo->currentData().toString(), ui->fromEdit->date(),
                                      ui->todayCheck->isChecked() ? QDate() : ui->toEdit->date());
        else
            Q_UNREACHABLE();
    }
}

void DownloadOptionsPage::retranslateUi()
{
    ui->periodCombo->setItemText(ui->periodCombo->findData(GEnums::rtsDays), tr("Days"));
    ui->periodCombo->setItemText(ui->periodCombo->findData(GEnums::rtsWeeks), tr("Weeks"));
    ui->periodCombo->setItemText(ui->periodCombo->findData(GEnums::rtsMonths), tr("Months"));
    ui->periodCombo->setItemText(ui->periodCombo->findData(GEnums::rtsYears), tr("Years"));
    ui->retranslateUi(this);
}

void DownloadOptionsPage::onLoadDownloadFormat(const QString &format, GEnums::RatingTimeMethod timeMethod, const QDate &from, const QDate &to,
                                               GEnums::RatingTimeScale timeScale, int timeSpan)
{
    ui->formatCombo->setCurrentIndex(ui->formatCombo->findData(format, Qt::UserRole));
    switch (timeMethod) {
    case GEnums::rtmPastPeriod:
        ui->lastPeriodRadio->toggle();
        ui->periodCombo->setCurrentIndex(ui->periodCombo->findData(timeScale));
        ui->periodSpin->setValue(timeSpan);
        break;
    case GEnums::rtmBetweenDates:
        ui->timeSpanRadio->toggle();
        ui->fromEdit->setDate(from);
        ui->toEdit->setDate(to.isValid() ? to : QDate::currentDate());
        ui->todayCheck->setChecked(!to.isValid());
        break;
    case GEnums::rtmAnytime:
    default:
        ui->noLimitRadio->toggle();
    }
}

void DownloadOptionsPage::onShowOnlyDraftableSetsChanged(bool showOnly)
{
    ui->draftableSetsCheck->setChecked(showOnly);
}
void DownloadOptionsPage::onFromEditChanged(const QDate fromDate)
{
    ui->toEdit->setMinimumDate(fromDate.addDays(1));
}

void DownloadOptionsPage::onTimeRadioChanged()
{
    if (ui->noLimitRadio->isChecked()) {
        ui->toEdit->setEnabled(false);
        ui->fromEdit->setEnabled(false);
        ui->todayCheck->setEnabled(false);
        ui->periodCombo->setEnabled(false);
        ui->periodSpin->setEnabled(false);
        return;
    }
    if (ui->lastPeriodRadio->isChecked()) {
        ui->toEdit->setEnabled(false);
        ui->fromEdit->setEnabled(false);
        ui->todayCheck->setEnabled(false);
        ui->periodCombo->setEnabled(true);
        ui->periodSpin->setEnabled(true);
        return;
    }
    if (ui->timeSpanRadio->isChecked()) {
        ui->toEdit->setEnabled(!ui->todayCheck->isChecked());
        ui->fromEdit->setEnabled(true);
        ui->todayCheck->setEnabled(true);
        ui->periodCombo->setEnabled(false);
        ui->periodSpin->setEnabled(false);
        return;
    }
    Q_UNREACHABLE();
}
void DownloadOptionsPage::onTodayCheckChanged()
{
    ui->toEdit->setEnabled(!ui->todayCheck->isChecked());
}
