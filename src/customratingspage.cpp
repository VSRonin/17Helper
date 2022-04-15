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

#include "customratingspage.h"
#include "ui_customratingspage.h"
#include <mainobject.h>
#include <customratingmodel.h>
#include "customratingdelegate.h"
#include <QCompleter>
CustomRatingsPage::CustomRatingsPage(QWidget *parent)
    : TranslatableWidgetInterface(parent)
    , ui(new Ui::CustomRatingsPage)
{
    ui->setupUi(this);
    searchCompleter = new QCompleter(this);
    searchCompleter->setCompletionColumn(CustomRatingModel::crmName);
    ui->searchEdit->setCompleter(searchCompleter);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &CustomRatingsPage::updateRatingsFiler);
    connect(ui->applyButton, &QPushButton::clicked, this, &CustomRatingsPage::applyCustomRatings);
}

CustomRatingsPage::~CustomRatingsPage()
{
    delete ui;
}

MainObject *CustomRatingsPage::mainObject() const
{
    return m_object;
}
void CustomRatingsPage::retranslateUi()
{
    ui->retranslateUi(this);
}

void CustomRatingsPage::setMainObject(MainObject *newObject)
{
    if (m_object == newObject)
        return;
    for (auto &&i : qAsConst(m_objectConnections))
        disconnect(i);
    m_objectConnections.clear();
    m_object = newObject;
    if (m_object) {
        ui->customRatingView->setModel(m_object->customRatingsModel());
        ui->customRatingView->setColumnHidden(CustomRatingModel::crmIdArena, true);
        ui->customRatingView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->customRatingView->sortByColumn(CustomRatingModel::crmName, Qt::AscendingOrder);
        CustomRatingsDelegate *const customRatingsDelegate = new CustomRatingsDelegate(this);
        ui->customRatingView->setItemDelegateForColumn(CustomRatingModel::crmRating, customRatingsDelegate);
        connect(customRatingsDelegate, &CustomRatingsDelegate::clicked, this, &CustomRatingsPage::onCustomRatingCleared);
        searchCompleter->setModel(m_object->customRatingsModel());
        m_objectConnections = QVector<QMetaObject::Connection>{
                connect(m_object->setsModel(), &QAbstractItemModel::dataChanged, this, &CustomRatingsPage::onSetsFilter)};
        updateRatingsFiler();
    }
}

void CustomRatingsPage::onCustomRatingCleared(const QModelIndex &idx)
{
    Q_ASSERT(idx.isValid());
    Q_ASSERT(idx.model() == m_object->customRatingsModel());
    Q_ASSERT(idx.column() == CustomRatingModel::crmRating);
    m_object->customRatingsModel()->setData(idx, QVariant());
}

void CustomRatingsPage::updateRatingsFiler()
{
    if (m_object)
        m_object->filterRatings(ui->searchEdit->text());
}
void CustomRatingsPage::onSetsFilter(const QModelIndex &, const QModelIndex &, const QVector<int> &roles)
{
    if (roles.isEmpty() || roles.contains(Qt::CheckStateRole))
        updateRatingsFiler();
}
