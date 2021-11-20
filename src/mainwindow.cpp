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
#include "textdatedelegate.h"
#include "ratingsmodel.h"
#include "slratingsmodel.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QHeaderView>
#include <QIdentityProxyModel>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QUrl>
#include <QDebug>
#include <QDir>
#include <QCompleter>
#include "decimaldelegate.h"
#include "percentagedelegate.h"
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

ProgressElement::ProgressElement()
    : ProgressElement(MainObject::opNoOperation, QString(), 0, 0, 0)
{ }

ProgressElement::ProgressElement(MainObject::Operations operation, const QString &description, int max, int min, int val)
    : m_operation(operation)
    , m_description(description)
    , m_min(min)
    , m_max(max)
    , m_val(val)
{ }

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
    toggleLoginLogoutButtons();
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
    m_object->download17Lands(ui->formatsCombo->currentData().toString());
}

void MainWindow::doMtgahUpload(bool clear)
{
    m_object->uploadMTGAH(ui->ratingBasedCombo->currentData().value<Worker::SLMetrics>(), locale(), clear);
}

void MainWindow::updatedUploadedLabel(const QString &card)
{
    ui->uploadedLabel->setText(tr("Uploaded: %1").arg(card));
}

void MainWindow::onMTGAHSetsError()
{
    m_error |= MTGAHSetsError;
    ui->retryBasicDownloadButton->setEnabled(true);
    retranslateUi();
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
    retranslateUi();
}

void MainWindow::updateRatingsFiler()
{
    QStringList sets;
    for (int i = 0, iEnd = m_object->setsModel()->rowCount(); i != iEnd; ++i) {
        if (m_object->setsModel()->index(i, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked) {
            sets.append(m_object->setsModel()->index(i, 1).data().toString());
        }
    }
    m_object->filterRatings(ui->searchCardEdit->text(), sets);
}

void MainWindow::onLogout()
{
    m_error &= ~LogoutError;
    ui->usernameEdit->setEnabled(true);
    ui->pwdEdit->setEnabled(true);
    toggleLoginLogoutButtons();
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
    ui->uploadedLabel->hide();
    ui->formatsCombo->setModel(m_object->formatsModel());
    ui->ratingsView->setModel(m_object->ratingsModel());
    ui->ratingsView->setColumnHidden(RatingsModel::rmcArenaId, true);
    ui->ratingsView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->ratingsView->sortByColumn(RatingsModel::rmcName, Qt::AscendingOrder);
    ui->ratingsView->setItemDelegateForColumn(RatingsModel::rmcRating, new RatingsDelegate(this));
    ui->slRatingsView->setModel(m_object->seventeenLandsRatingsModel());
    ui->slRatingsView->setItemDelegateForColumn(SLRatingsModel::slmLastUpdate, new TextDateDelegate(this));
    PercentDelegate *percentDelegate = new PercentDelegate(this);
    for (int metric : {Worker::SLwin_rate, Worker::SLopening_hand_win_rate, Worker::SLdrawn_win_rate, Worker::SLever_drawn_win_rate,
                       Worker::SLnever_drawn_win_rate, Worker::SLdrawn_improvement_win_rate})
        ui->slRatingsView->setItemDelegateForColumn(metric + 2, percentDelegate);
    DecimalDelegate *decimalDelegate = new DecimalDelegate(this);
    for (int metric : {Worker::SLavg_seen, Worker::SLavg_pick})
        ui->slRatingsView->setItemDelegateForColumn(metric + 2, decimalDelegate);
    QCompleter *searchCompleter = new QCompleter(this);
    searchCompleter->setModel(m_object->ratingsModel());
    searchCompleter->setCompletionColumn(RatingsModel::rmcName);
    ui->searchCardEdit->setCompleter(searchCompleter);
    ui->notesView->setModel(m_object->SLMetricsModel());
    auto SLMetricsProxy = new NoCheckProxy(this);
    SLMetricsProxy->setSourceModel(m_object->SLMetricsModel());
    ui->ratingBasedCombo->setModel(SLMetricsProxy);
    ui->ratingBasedCombo->setCurrentIndex(Worker::SLdrawn_win_rate);
    retranslateUi();

    connect(ui->ratingBasedButton, &QPushButton::clicked, this,
            []() { QDesktopServices::openUrl(QUrl::fromUserInput(QStringLiteral("https://www.17lands.com/metrics_definitions"))); });
    connect(ui->remembePwdCheck, &QAbstractButton ::clicked, ui->savePwdWarningLabel, &QWidget::setVisible);
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::doLogin);
    connect(ui->logoutButton, &QPushButton::clicked, this, &MainWindow::doLogout);
    connect(ui->retryBasicDownloadButton, &QPushButton::clicked, this, &MainWindow::retrySetsDownload);
    connect(ui->retryTemplateButton, &QPushButton::clicked, this, &MainWindow::retryTemplateDownload);
    connect(ui->downloadButton, &QPushButton::clicked, this, &MainWindow::do17Ldownload);
    connect(ui->uploadButton, &QPushButton::clicked, this, std::bind(&MainWindow::doMtgahUpload, this, false));
    connect(ui->clearRatingsButton, &QPushButton::clicked, this, std::bind(&MainWindow::doMtgahUpload, this, true));
    connect(ui->allSetsButton, &QPushButton::clicked, this, &MainWindow::selectAllSets);
    connect(ui->noSetButton, &QPushButton::clicked, this, &MainWindow::selectNoSets);
    connect(ui->searchCardEdit, &QLineEdit::textChanged, this, &MainWindow::updateRatingsFiler);
    connect(ui->draftableSetsCheck, &QCheckBox::clicked, m_object, &MainObject::showOnlyDraftableSets);
    connect(m_object, &MainObject::ratingUploaded, this, &MainWindow::updatedUploadedLabel);
    connect(m_object, &MainObject::startProgress, this, &MainWindow::onStartProgress);
    connect(m_object, &MainObject::updateProgress, this, &MainWindow::onUpdateProgress);
    connect(m_object, &MainObject::increaseProgress, this, &MainWindow::onIncreaseProgress);
    connect(m_object, &MainObject::endProgress, this, &MainWindow::onEndProgress);
    connect(m_object, &MainObject::loggedIn, this, &MainWindow::onLogin);
    connect(m_object, &MainObject::loginFalied, this, &MainWindow::onLoginError);
    connect(m_object, &MainObject::loggedOut, this, &MainWindow::onLogout);
    connect(m_object, &MainObject::logoutFailed, this, &MainWindow::onLogoutError);
    connect(m_object->setsModel(), &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &, const QModelIndex &, const QVector<int> &roles) {
                if (roles.isEmpty() || roles.contains(Qt::CheckStateRole))
                    updateRatingsFiler();
            });
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
    ui->retranslateUi(this);
    ui->savePwdWarningLabel->setText(
            tr("Your password will be stored in plain text in %1").arg(appDataPath() + QDir::separator() + QLatin1String("17helperconfig.json")));
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

void MainWindow::setAllSetsSelection(Qt::CheckState check)
{
    for (int i = 0, iEnd = m_object->setsModel()->rowCount(); i < iEnd; ++i)
        m_object->setsModel()->setData(m_object->setsModel()->index(i, 0), check, Qt::CheckStateRole);
}

void MainWindow::onStartProgress(MainObject::Operations op, const QString &description, int max, int min)
{
#ifdef QT_DEBUG
    for (auto i = progressQueue.cbegin(), iEnd = progressQueue.cend(); i != iEnd; ++i)
        Q_ASSERT(i->m_operation != op);
#endif
    if (progressQueue.isEmpty()) {
        enableAll(false);
        ui->progressBar->setRange(min, max);
        ui->progressBar->setValue(min);
        ui->progressLabel->setText(description);
        ui->progressBar->show();
        ui->progressLabel->show();
        ui->uploadedLabel->setVisible(op == MainObject::opUploadMTGAH);
    }
    progressQueue.append(ProgressElement(op, description, max, min, min));
}

void MainWindow::onUpdateProgress(MainObject::Operations op, int val)
{
    const auto pBegin = progressQueue.begin();
    for (auto i = pBegin, iEnd = progressQueue.end(); i != iEnd; ++i) {
        if (i->m_operation == op) {
            i->m_val = val;
            if (i == pBegin)
                ui->progressBar->setValue(i->m_val);
        }
    }
}

void MainWindow::onIncreaseProgress(MainObject::Operations op, int val)
{
    const auto pBegin = progressQueue.begin();
    for (auto i = pBegin, iEnd = progressQueue.end(); i != iEnd; ++i) {
        if (i->m_operation == op) {
            i->m_val += val;
            if (i == pBegin)
                ui->progressBar->setValue(i->m_val);
        }
    }
}

void MainWindow::onEndProgress(MainObject::Operations op)
{
    const auto pBegin = progressQueue.begin();
    for (auto i = pBegin, iEnd = progressQueue.end(); i != iEnd; ++i) {
        if (i->m_operation == op) {
            if (i == pBegin) {
                ui->uploadedLabel->hide();
                i = progressQueue.erase(i);
                if (i != progressQueue.end()) {
                    ui->progressBar->setRange(i->m_min, i->m_max);
                    ui->progressBar->setValue(i->m_val);
                    ui->progressLabel->setText(i->m_description);
                }
            } else {
                i = progressQueue.erase(i);
            }
            if (i == progressQueue.end()) {
                ui->progressBar->hide();
                ui->progressLabel->hide();
                enableAll(true);
            }
            return;
        }
    }
    Q_UNREACHABLE();
}

void MainWindow::toggleLoginLogoutButtons()
{
    for (QPushButton *button : {ui->loginButton, ui->logoutButton}) {
        button->setVisible(!button->isVisible());
        button->setEnabled(true);
    }
    checkUploadButtonEnabled();
}

void MainWindow::checkUploadButtonEnabled()
{
    const bool enabled = !ui->loginButton->isVisible() && m_object->oneSetSelected();
    ui->uploadButton->setEnabled(enabled);
    ui->clearRatingsButton->setEnabled(enabled);
}

void MainWindow::enableAll(bool enable)
{
    QWidget *widToChange[] = {ui->MTGAHelperGroup, ui->customRatingsGroup, ui->setsGroup,
                              ui->uploadButton,    ui->clearRatingsButton, ui->downloadButton};
    for (QWidget *wid : widToChange)
        wid->setEnabled(enable);
    if (enable)
        checkUploadButtonEnabled();
}
