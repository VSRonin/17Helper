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

#include "uploadoptionspage.h"
#include "ui_uploadoptionspage.h"
#include <mainobject.h>
#include <nocheckproxy.h>
UploadOptionsPage::UploadOptionsPage(QWidget *parent)
    : TranslatableWidgetInterface(parent)
    , ui(new Ui::UploadOptionsPage)
    , m_SLMetricsProxy(new NoCheckProxy(this))
{
    ui->setupUi(this);
    ui->ratingBasedCombo->setModel(m_SLMetricsProxy);
    connect(ui->allMetricsButton, &QPushButton::clicked, this, &UploadOptionsPage::selectAllMetrics);
    connect(ui->noMetricsButton, &QPushButton::clicked, this, &UploadOptionsPage::selectNoMetrics);
    connect(ui->customRatingButton, &QPushButton::clicked, this, &UploadOptionsPage::customiseRatings);
    connect(ui->uploadButton, &QPushButton::clicked, this, std::bind(&UploadOptionsPage::doMtgahUpload, this, false));
    connect(ui->clearRatingsButton, &QPushButton::clicked, this, std::bind(&UploadOptionsPage::doMtgahUpload, this, true));
}

UploadOptionsPage::~UploadOptionsPage()
{
    delete ui;
}
MainObject *UploadOptionsPage::mainObject() const
{
    return m_object;
}

void UploadOptionsPage::setMainObject(MainObject *newObject)
{
    if (m_object == newObject)
        return;
    for (auto &&i : qAsConst(m_objectConnections))
        disconnect(i);
    m_objectConnections.clear();
    m_object = newObject;
    if (m_object) {
        m_SLMetricsProxy->setSourceModel(m_object->SLMetricsModel());
        ui->notesView->setModel(m_object->SLMetricsModel());
        ui->ratingBasedCombo->setCurrentIndex(ui->ratingBasedCombo->findData(GEnums::SLever_drawn_win_rate));
        m_objectConnections = QVector<QMetaObject::Connection>{
                connect(m_object, &MainObject::showOnlySLRatiosChanged, this, &UploadOptionsPage::onShowOnlyRatiosChanged),
                connect(ui->onlyRatiosCheck, &QCheckBox::clicked, m_object, &MainObject::showOnlySLRatios),
                connect(m_object, &MainObject::loadUploadRating, this, &UploadOptionsPage::onLoadUploadRating)};
    }
}

void UploadOptionsPage::doMtgahUpload(bool clear)
{
    m_object->uploadMTGAH(ui->ratingBasedCombo->currentData().value<GEnums::SLMetrics>(), locale(), clear);
}

void UploadOptionsPage::onLoadUploadRating(GEnums::SLMetrics ratingBase)
{
    ui->ratingBasedCombo->setCurrentIndex(ui->ratingBasedCombo->findData(ratingBase));
}
void UploadOptionsPage::retranslateUi()
{
    ui->retranslateUi(this);
}
void UploadOptionsPage::onShowOnlyRatiosChanged(bool showOnly)
{
    ui->onlyRatiosCheck->setChecked(showOnly);
}
void UploadOptionsPage::selectAllMetrics()
{
    if (m_object)
        m_object->setAllSLMetricsSelection(Qt::Checked);
}
void UploadOptionsPage::selectNoMetrics()
{
    if (m_object)
        m_object->setAllSLMetricsSelection(Qt::Unchecked);
}
