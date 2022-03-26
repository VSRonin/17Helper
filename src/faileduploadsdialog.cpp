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
#include "faileduploadsdialog.h"
#include "ui_faileduploadsdialog.h"
#include <QStringListModel>
#include <QClipboard>
#include <QApplication>
FailedUploadsDialog::FailedUploadsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FailedUploadsDialog)
{
    ui->setupUi(this);

    m_cardListModel = new QStringListModel(this);
    ui->failedCardsView->setModel(m_cardListModel);
    ui->failedCardsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->copyButton, &QPushButton::clicked, this, &FailedUploadsDialog::copyCardsToClipboard);
    QStyle *style = ui->iconLabel->style();
    int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, ui->iconLabel);
    QIcon tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, 0, ui->iconLabel);
    if (!tmpIcon.isNull()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        ui->iconLabel->setPixmap(tmpIcon.pixmap(QSize(iconSize, iconSize), ui->iconLabel->devicePixelRatio()));
#else
        QWindow *window = nullptr;
        if (auto nativeParent = ui->iconLabel->nativeParentWidget())
            window = nativeParent->windowHandle();
        ui->iconLabel->setPixmap(tmpIcon.pixmap(window, QSize(iconSize, iconSize)));
#endif
    }
}

FailedUploadsDialog::~FailedUploadsDialog()
{
    delete ui;
}

void FailedUploadsDialog::setCardList(const QStringList &cardList)
{
    m_cardListModel->setStringList(cardList);
}

void FailedUploadsDialog::copyCardsToClipboard()
{
    QApplication::clipboard()->setText(m_cardListModel->stringList().join(QChar::LineFeed));
}
