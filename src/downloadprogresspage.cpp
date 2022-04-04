#include "downloadprogresspage.h"
#include "ui_downloadprogresspage.h"
#include <QStringListModel>
DownloadProgressPage::DownloadProgressPage(QWidget *parent)
    : TranslatableWidgetInterface(parent)
    , m_noRatingsModel(new QStringListModel(this))
    , ui(new Ui::DownloadProgressPage)

{
    ui->setupUi(this);
    ui->noRatingsView->setModel(m_noRatingsModel);
    reset();
    connect(ui->backButton, &QPushButton::clicked, this, &DownloadProgressPage::goBack);
}

void DownloadProgressPage::reset()
{
    ui->downloadingLabel->show();
    ui->downloadProgress->hide();
    ui->backButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->downloadCompleteLabel->hide();
    ui->errorLabel->hide();
    ui->noRatinglabel->hide();
    ui->noRatingsView->hide();
}

void DownloadProgressPage::retranslateUi()
{
    ui->retranslateUi(this);
}

void DownloadProgressPage::onStartProgress(MainObject::Operations op, const QString &description, int max, int min)
{
    if (op != MainObject::opDownload17Ratings)
        return;
    ui->downloadProgress->setRange(min, max);
    ui->downloadProgress->setValue(min);
    ui->downloadProgress->show();
}

void DownloadProgressPage::onUpdateProgress(MainObject::Operations op, int val)
{
    if (op != MainObject::opDownload17Ratings)
        return;
    ui->downloadProgress->setValue(val);
}

void DownloadProgressPage::onIncreaseProgress(MainObject::Operations op, int val)
{
    if (op != MainObject::opDownload17Ratings)
        return;
    ui->downloadProgress->setValue(val + ui->downloadProgress->value());
}

void DownloadProgressPage::onEndProgress(MainObject::Operations op)
{
    if (op != MainObject::opDownload17Ratings)
        return;
    ui->downloadProgress->hide();
}

void DownloadProgressPage::onSLDownloadFinished()
{
    ui->downloadingLabel->hide();
    ui->downloadCompleteLabel->show();
    ui->backButton->setEnabled(true);
    ui->nextButton->setEnabled(true);
}

void DownloadProgressPage::onSLDownloadFailed()
{
    ui->downloadingLabel->hide();
    ui->backButton->setEnabled(true);
    ui->errorLabel->show();
}

void DownloadProgressPage::onNo17LRating(const QStringList &sets)
{
    m_noRatingsModel->setStringList(sets);
    ui->noRatinglabel->show();
    ui->noRatingsView->show();
}

DownloadProgressPage::~DownloadProgressPage()
{
    delete ui;
}
MainObject *DownloadProgressPage::mainObject() const
{
    return m_object;
}

void DownloadProgressPage::setMainObject(MainObject *newObject)
{
    if (m_object == newObject)
        return;
    for (auto &&i : qAsConst(m_objectConnections))
        disconnect(i);
    m_objectConnections.clear();
    m_object = newObject;
    if (!m_object)
        return;
    m_objectConnections =
            QVector<QMetaObject::Connection>{connect(m_object, &MainObject::startProgress, this, &DownloadProgressPage::onStartProgress),
                                             connect(m_object, &MainObject::updateProgress, this, &DownloadProgressPage::onUpdateProgress),
                                             connect(m_object, &MainObject::increaseProgress, this, &DownloadProgressPage::onIncreaseProgress),
                                             connect(m_object, &MainObject::endProgress, this, &DownloadProgressPage::onEndProgress),
                                             connect(m_object, &MainObject::SLDownloadFinished, this, &DownloadProgressPage::onSLDownloadFinished),
                                             connect(m_object, &MainObject::SLDownloadFailed, this, &DownloadProgressPage::onSLDownloadFailed),
                                             connect(m_object, &MainObject::no17LRating, this, &DownloadProgressPage::onNo17LRating)};
}
