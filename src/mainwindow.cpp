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
#include "ratingsdelegate.h"
#include "ratingsmodel.h"
#include "ui_mainwindow.h"
#include "mainobject.h"
#include "globals.h"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QHeaderView>
#include <QIdentityProxyModel>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QUrl>
#include <QDebug>
class NoCheckProxy : public QIdentityProxyModel
{
    Q_DISABLE_COPY_MOVE(NoCheckProxy)
public:
    using QIdentityProxyModel::QIdentityProxyModel;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (role == Qt::CheckStateRole)
            return QVariant();
        return QIdentityProxyModel::data(index, role);
    }
};

void MainWindow::doLogin()
{
    QWidget *widToDisable[] = {ui->loginButton, ui->remembePwdCheck, ui->usernameEdit, ui->pwdEdit};
    for (QWidget *wid : widToDisable)
        wid->setEnabled(false);
    m_object->tryLogin(ui->usernameEdit->text(), ui->pwdEdit->text(), ui->remembePwdCheck->checkState() == Qt::Checked);
    ui->pwdEdit->clear();
}

void MainWindow::onLogin()
{
    m_error &= ~LoginError;
    // m_worker->getCustomRatingTemplate();
    toggleLoginLogoutButtons();
    enableSetsSection();
    retranslateUi();
}

void MainWindow::onLoginError(const QString &error)
{
    qDebug() << error;
    m_error |= LoginError;
    QWidget *widToEnable[] = {ui->loginButton, ui->remembePwdCheck, ui->usernameEdit, ui->pwdEdit};
    for (QWidget *wid : widToEnable)
        wid->setEnabled(true);
    retranslateUi();
}

void MainWindow::doLogout()
{
    ui->logoutButton->setEnabled(false);
    m_object->logOut();
}

void MainWindow::onLogoutError(const QString &error)
{
    qDebug() << error;
    m_error |= LogoutError;
    ui->logoutButton->setEnabled(true);
    retranslateUi();
}

void MainWindow::do17Ldownload()
{
    /* ui->downloadButton->setEnabled(false);
     ui->setsGroup->setEnabled(false);
     QStringList sets;
     for (int i = 0, iEnd = m_setsModel->rowCount(); i < iEnd; ++i) {
         const QModelIndex &idx = m_setsModel->index(i, 0);
         if (idx.data(Qt::CheckStateRole).toInt() == Qt::Checked)
             sets.append(idx.data(Qt::UserRole).toString());
     }
     ui->progressBar->setVisible(true);
     ui->progressLabel->setVisible(true);
     ui->progressBar->setRange(0, sets.size());
     ui->progressBar->setValue(0);
     ui->progressLabel->setText(tr("Downloading 17 Lands Data"));
     m_worker->get17LRatings(sets, ui->formatsCombo->currentData().toString());*/
}

void MainWindow::doMtgahUpload()
{
    /*ui->uploadButton->setEnabled(false);
    ui->progressBar->setVisible(true);
    ui->progressBar->setRange(0, 1);
    ui->progressBar->setValue(0);
    ui->progressLabel->setVisible(true);
    ui->progressLabel->setText(tr("Uploading"));
    QStringList sets;
    for (int i = 0, iEnd = m_setsModel->rowCount(); i < iEnd; ++i) {
        const QModelIndex &idx = m_setsModel->index(i, 0);
        if (idx.data(Qt::CheckStateRole).toInt() == Qt::Checked)
            sets.append(idx.data(Qt::UserRole).toString());
    }
    m_worker->uploadRatings(sets);*/
}

void MainWindow::onAllRatingsUploaded()
{
    ui->uploadButton->setEnabled(true);
    ui->progressBar->setVisible(false);
    ui->progressLabel->setVisible(false);
}

void MainWindow::fillSets(const QStringList &sets)
{
    /*m_error &= ~MTGAHSetsError;
    m_setsModel->removeRows(0, m_setsModel->rowCount());
    Qt::CheckState checkState = Qt::Checked;
    for (int i = sets.size() - 1; i >= 0; --i) {
        auto item = new QStandardItem;
        item->setData(sets.at(i), Qt::DisplayRole);
        item->setData(sets.at(i), Qt::UserRole);
        item->setData(checkState, Qt::CheckStateRole);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        m_setsModel->insertRow(m_setsModel->rowCount(), item);
        checkState = Qt::Unchecked;
    }
    m_worker->downloadSetsScryfall();
    retranslateUi();*/
}

void MainWindow::fillSetNames(const QHash<QString, QString> &setNames)
{
    /*for (int i = 0, iEnd = m_setsModel->rowCount(); i < iEnd; ++i) {
        const QString setToFind = m_setsModel->index(i, 0).data(Qt::UserRole).toString();
        auto nameIter = setNames.constFind(setToFind);
        if (nameIter != setNames.constEnd())
            m_setsModel->setData(m_setsModel->index(i, 0), nameIter.value());
    }
    retranslateUi();*/
}

void MainWindow::onDownloaded17LRatings(const QString &set, const QSet<SeventeenCard> &ratings)
{
    /*Q_ASSERT(!ratings.isEmpty());
    const auto ratingComparison = [this](const SeventeenCard &a, const SeventeenCard &b) -> bool { return ratingValue(a) < ratingValue(b); };
    const auto minMaxRtg = std::minmax_element(ratings.cbegin(), ratings.cend(), ratingComparison);
    const double minRtgValue = ratingValue(*minMaxRtg.first);
    double ratingDenominator = ratingValue(*minMaxRtg.second) - minRtgValue;
    if (ratingDenominator == 0.0)
        ratingDenominator = 1.0;
    for (int i = 0, iEnd = m_ratingsModel->rowCount(); i < iEnd; ++i) {
        QCoreApplication::processEvents();
        if (m_ratingsModel->index(i, RatingsModel::rmcSet).data().toString() != set)
            continue;
        const auto rtgIter = ratings.constFind(SeventeenCard(m_ratingsModel->index(i, RatingsModel::rmcName).data().toString()));
        if (rtgIter == ratings.constEnd())
            continue;
        m_ratingsModel->setData(m_ratingsModel->index(i, RatingsModel::rmcRating),
                                qRound(10.0 * (ratingValue(*rtgIter) - minRtgValue) / ratingDenominator));
        m_ratingsModel->setData(m_ratingsModel->index(i, RatingsModel::rmcNote), commentString(*rtgIter));
    }*/
}

void MainWindow::onDownloadedAll17LRatings()
{
    ui->downloadButton->setEnabled(true);
    ui->setsGroup->setEnabled(true);
    ui->progressBar->setVisible(false);
    ui->progressLabel->setVisible(false);
}

void MainWindow::onDownload17LRatingsProgress(int progress)
{
    /*int setCount = 0;
    for (int i = 0, iEnd = m_setsModel->rowCount(); i < iEnd; ++i) {
        const QModelIndex &idx = m_setsModel->index(i, 0);
        if (idx.data(Qt::CheckStateRole).toInt() == Qt::Checked)
            ++setCount;
    }
    ui->progressBar->setValue(ui->progressBar->maximum() - progress);*/
}

void MainWindow::onMTGAHSetsError()
{
    m_error |= MTGAHSetsError;
    ui->retryBasicDownloadButton->setEnabled(true);
    retranslateUi();
}

void MainWindow::onScryfallSetsError()
{
    fillSetNames(QHash<QString, QString>());
}

void MainWindow::onTemplateDownloadFailed()
{
    m_error |= RatingTemplateFailed;
    ui->retryTemplateButton->setEnabled(true);
    retranslateUi();
}

void MainWindow::retrySetsDownload()
{
    ui->retryBasicDownloadButton->setEnabled(false);
    // m_worker->downloadSetsMTGAH();
}

void MainWindow::retryTemplateDownload()
{
    ui->retryTemplateButton->setEnabled(false);
    // m_worker->getCustomRatingTemplate();
}

void MainWindow::onCustomRatingsTemplateDownloaded()
{
    m_error &= ~RatingTemplateFailed;
    // m_ratingsModel->setRatingsTemplate(m_worker->ratingsTemplate());
    retranslateUi();
}

void MainWindow::updateRatingsFiler()
{
    /*QString regExpString;
    for (int i = 0, iEnd = m_setsModel->rowCount(); i != iEnd; ++i) {
        const QModelIndex currIdx = m_setsModel->index(i, 0);
        if (currIdx.data(Qt::CheckStateRole).toInt() == Qt::Checked) {
            if (regExpString.isEmpty())
                regExpString = QStringLiteral("(?:");
            else
                regExpString += QLatin1Char('|');
            regExpString += currIdx.data(Qt::UserRole).toString();
        }
    }
    if (!regExpString.isEmpty())
        regExpString += QLatin1Char(')');
    m_ratingsProxy->setFilterRegularExpression(regExpString);*/
}

void MainWindow::onRatingsUploadMaxProgress(int maxRange)
{
    ui->progressBar->setRange(0, maxRange);
}

void MainWindow::onRatingsUploadProgress(int progress)
{
    ui->progressBar->setValue(ui->progressBar->maximum() - progress);
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

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , m_error(NoError)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_object = new MainObject(this);
    ui->setsView->setModel(m_object->setsModel());
    ui->logoutButton->hide();
    ui->errorLabel->hide();
    ui->progressBar->hide();
    ui->progressLabel->hide();
    ui->savePwdWarningLabel->hide();
    ui->retryBasicDownloadButton->hide();
    ui->formatsCombo->addItem(QString(), QStringLiteral("PremierDraft"));
    ui->formatsCombo->addItem(QString(), QStringLiteral("QuickDraft"));
    ui->formatsCombo->addItem(QString(), QStringLiteral("TradDraft"));
    ui->formatsCombo->addItem(QString(), QStringLiteral("Sealed"));
    ui->formatsCombo->addItem(QString(), QStringLiteral("TradSealed"));
    /*-m_ratingsModel = new RatingsModel(this);
    m_ratingsModel->setRatingsTemplate(m_worker->ratingsTemplate());
    m_ratingsProxy = new QSortFilterProxyModel(this);
    m_ratingsProxy->setSourceModel(m_ratingsModel);
    m_ratingsProxy->setFilterKeyColumn(RatingsModel::rmcSet);
    ui->ratingsView->setModel(m_ratingsProxy);*/
    ui->ratingsView->setColumnHidden(RatingsModel::rmcArenaId, true);
    ui->ratingsView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->ratingsView->sortByColumn(RatingsModel::rmcName, Qt::AscendingOrder);
    ui->ratingsView->setItemDelegateForColumn(RatingsModel::rmcRating, new RatingsDelegate(this));
    ui->notesView->setModel(m_object->SLMetricsModel());
    auto SLMetricsProxy = new NoCheckProxy(this);
    SLMetricsProxy->setSourceModel(m_object->SLMetricsModel());
    ui->ratingBasedCombo->setModel(SLMetricsProxy);
    ui->ratingBasedCombo->setCurrentIndex(MainObject::SLdrawn_win_rate);
    disableSetsSection();
    retranslateUi();

    connect(ui->ratingBasedButton, &QPushButton::clicked, this,
            []() { QDesktopServices::openUrl(QUrl::fromUserInput(QStringLiteral("https://www.17lands.com/metrics_definitions"))); });
    connect(ui->remembePwdCheck, &QAbstractButton ::clicked, ui->savePwdWarningLabel, &QWidget::setVisible);
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::doLogin);
    connect(ui->logoutButton, &QPushButton::clicked, this, &MainWindow::doLogout);
    connect(ui->retryBasicDownloadButton, &QPushButton::clicked, this, &MainWindow::retrySetsDownload);
    connect(ui->retryTemplateButton, &QPushButton::clicked, this, &MainWindow::retryTemplateDownload);
    connect(ui->downloadButton, &QPushButton::clicked, this, &MainWindow::do17Ldownload);
    connect(ui->uploadButton, &QPushButton::clicked, this, &MainWindow::doMtgahUpload);
    connect(ui->allSetsButton, &QPushButton::clicked, this, &MainWindow::selectAllSets);
    connect(ui->noSetButton, &QPushButton::clicked, this, &MainWindow::selectNoSets);
    // connect(m_worker, &Worker::setsMTGAH, this, &MainWindow::fillSets);
    // connect(m_worker, &Worker::downloadSetsMTGAHFailed, this, &MainWindow::onMTGAHSetsError);
    // connect(m_worker, &Worker::setsScryfall, this, &MainWindow::fillSetNames);
    // connect(m_worker, &Worker::downloadSetsScryfallFailed, this, &MainWindow::onScryfallSetsError);
    connect(m_object, &MainObject::loggedIn, this, &MainWindow::onLogin);
    connect(m_object, &MainObject::loginFalied, this, &MainWindow::onLoginError);
    connect(m_object, &MainObject::loggedOut, this, &MainWindow::onLogout);
    connect(m_object, &MainObject::logoutFailed, this, &MainWindow::onLogoutError);
    // connect(m_worker, &Worker::customRatingTemplate, this, &MainWindow::onCustomRatingsTemplateDownloaded);
    // connect(m_worker, &Worker::customRatingTemplateFailed, this, &MainWindow::onTemplateDownloadFailed);
    // connect(m_worker, &Worker::downloaded17LRatings, this, &MainWindow::onDownloaded17LRatings);
    // connect(m_worker, &Worker::downloadedAll17LRatings, this, &MainWindow::onDownloadedAll17LRatings);
    // connect(m_worker, &Worker::ratingsUploadMaxProgress, this, &MainWindow::onRatingsUploadMaxProgress);
    // connect(m_worker, &Worker::ratingsUploadProgress, this, &MainWindow::onRatingsUploadProgress);
    // connect(m_worker, &Worker::allRatingsUploaded, this, &MainWindow::onAllRatingsUploaded);
    /*connect(m_setsModel, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &, const QModelIndex &, const QVector<int> &roles) {
        if (roles.isEmpty() || roles.contains(Qt::CheckStateRole))
            updateRatingsFiler();
    });*/
    // connect(m_ratingsModel, &QAbstractItemModel::modelReset, this, &MainWindow::updateRatingsFiler);
    // m_worker->downloadSetsMTGAH();
}

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow::CurrentErrors MainWindow::errors() const
{
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
    ui->savePwdWarningLabel->setText(ui->savePwdWarningLabel->text().arg(appDataPath()));
    ui->errorLabel->setVisible(m_error != NoError);
    ui->retryBasicDownloadButton->setVisible(m_error & MTGAHSetsError);
    ui->retryTemplateButton->setVisible(m_error & RatingTemplateFailed);
    QStringList errorStrings;
    if (m_error & LoginError)
        errorStrings.append(tr("Login Failed! Check your username, password or internet connection"));
    if (m_error & LogoutError)
        errorStrings.append(tr("Logout Failed! Check your internet connection"));
    if (m_error & MTGAHSetsError)
        errorStrings.append(tr("Error downloading sets info! Check your internet connection"));
    if (m_error & RatingTemplateFailed)
        errorStrings.append(tr("Error downloading ratings template! Check your internet connection"));
    ui->errorLabel->setText(errorStrings.join(QChar(QLatin1Char('\n'))));
    ui->ratingsView->update();
    m_object->retranslateModels();
}

void MainWindow::setSetsSectionEnabled(bool enabled)
{
    ui->setsGroup->setEnabled(enabled);
    ui->customRatingsGroup->setEnabled(enabled);
}

void MainWindow::setAllSetsSelection(Qt::CheckState check)
{
    /*for (int i = 0, iEnd = m_setsModel->rowCount(); i < iEnd; ++i)
        m_setsModel->item(i)->setCheckState(check);*/
}

void MainWindow::toggleLoginLogoutButtons()
{
    for (QPushButton *button : {ui->loginButton, ui->logoutButton}) {
        button->setVisible(!button->isVisible());
        button->setEnabled(true);
    }
}
