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

#include "centralwidget.h"
#include "ratingsdelegate.h"
#include "textdatedelegate.h"
#include "ratingsmodel.h"
#include "slratingsmodel.h"
#include "ui_centralwidget.h"
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
#include <QMessageBox>
#include "decimaldelegate.h"
#include "percentagedelegate.h"
#include "customratingmodel.h"
#include "faileduploadsdialog.h"
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

void CentralWidget::doLogin()
{
    QWidget *widToDisable[] = {ui->loginButton, ui->remembePwdCheck, ui->usernameEdit, ui->pwdEdit};
    for (QWidget *wid : widToDisable)
        wid->setEnabled(false);
    m_object->tryLogin(ui->usernameEdit->text(), ui->pwdEdit->text(), ui->remembePwdCheck->checkState() == Qt::Checked);
    ui->pwdEdit->clear();
}

void CentralWidget::onLogin()
{
    m_error &= ~LoginError;
    toggleLoginLogoutButtons();
    retranslateUi();
}

void CentralWidget::onLoginError(const QString &error)
{
    qDebug() << error;
    m_error |= LoginError;
    QWidget *widToEnable[] = {ui->loginButton, ui->remembePwdCheck, ui->usernameEdit, ui->pwdEdit};
    for (QWidget *wid : widToEnable)
        wid->setEnabled(true);
    retranslateUi();
}

void CentralWidget::doLogout()
{
    ui->logoutButton->setEnabled(false);
    m_object->logOut();
}

void CentralWidget::onLogoutError(const QString &error)
{
    qDebug() << error;
    m_error |= LogoutError;
    ui->logoutButton->setEnabled(true);
    retranslateUi();
}

void CentralWidget::onLogout()
{
    m_error &= ~LogoutError;
    ui->usernameEdit->setEnabled(true);
    ui->pwdEdit->setEnabled(true);
    toggleLoginLogoutButtons();
    retranslateUi();
}

void CentralWidget::do17Ldownload()
{
    QDate fromDt, toDt;
    if (ui->ratingTimeGroup->isChecked()) {
        fromDt = ui->ratingFromEdit->date();
        if (!ui->toTodayCheck->isChecked())
            toDt = ui->ratingToEdit->date();
    }
    m_object->download17Lands(ui->formatsCombo->currentData().toString(), fromDt, toDt);
}

void CentralWidget::on17LandsDownloadFinished()
{
    m_error &= ~SLDownloadError;
    retranslateUi();
}

void CentralWidget::on17Lerror()
{
    m_error |= SLDownloadError;
    retranslateUi();
}

void CentralWidget::onUploadedRatings()
{
    emit updatedUploadedStatus(QString());
    const QStringList failedCardsist = m_object->failedUploadCards();
    if (!failedCardsist.isEmpty()) {
        FailedUploadsDialog *failedDialog = new FailedUploadsDialog(this);
        failedDialog->setAttribute(Qt::WA_DeleteOnClose);
        failedDialog->setCardList(failedCardsist);
        failedDialog->show();
    }
    ui->cancelUploadButton->hide();
    retranslateUi();
}

void CentralWidget::onLoadUserPass(const QString &userName, const QString &password)
{
    ui->usernameEdit->setText(userName);
    ui->pwdEdit->setText(password);
    ui->remembePwdCheck->setChecked(true);
}

void CentralWidget::onLoadDownloadFormat(const QString &format, const QDate &fromDt, const QDate &toDt)
{
    ui->formatsCombo->setCurrentIndex(ui->formatsCombo->findData(format, Qt::UserRole));
    ui->ratingTimeGroup->setChecked(!fromDt.isNull());
    ui->toTodayCheck->setChecked(toDt.isNull());
    if (toDt.isNull())
        ui->ratingFromEdit->setDate(QDate::currentDate());
    else
        ui->ratingFromEdit->setDate(toDt);
    if (!fromDt.isNull()) {
        ui->ratingToEdit->setEnabled(toDt.isNull());
        ui->ratingFromEdit->setDate(fromDt);
    } else {
        ui->ratingFromEdit->setDate(QDate(2000, 1, 1));
    }
}

void CentralWidget::onLoadUploadRating(GEnums::SLMetrics ratingBase)
{
    ui->ratingBasedCombo->setCurrentIndex(int(ratingBase));
}

void CentralWidget::onInitialisationFailed()
{
    QMessageBox::critical(this, tr("Critical Error"), tr("Initialisation Failed, insufficient permissions"));
    QCoreApplication::quit();
}

void CentralWidget::doMtgahUpload(bool clear)
{
    m_object->uploadMTGAH(ui->ratingBasedCombo->currentData().value<GEnums::SLMetrics>(), locale(), clear);
}

void CentralWidget::onRememberPass(int state)
{
    ui->savePwdWarningLabel->setVisible(state == Qt::Checked);
}

void CentralWidget::onDownloadSetsMTGAHFailed()
{
    m_error |= MTGAHSetsError;
    ui->retryBasicDownloadButton->setEnabled(true);
    checkDownloadButtonEnabled();
    retranslateUi();
}

void CentralWidget::onSetsMTGAHDownloaded()
{
    m_error &= ~MTGAHSetsError;
    checkDownloadButtonEnabled();
    retranslateUi();
}

void CentralWidget::checkDownloadButtonEnabled()
{
    ui->downloadButton->setEnabled(!(m_error & MTGAHSetsError) && m_object->oneSetSelected());
}

void CentralWidget::onShowOnlyDraftableSetsChanged(bool showOnly)
{
    ui->draftableSetsCheck->setChecked(showOnly);
}

void CentralWidget::onRatingTimeGroupChecked(bool checked)
{
    if (!checked)
        return;
    onToTodayCheckChecked(ui->toTodayCheck->isChecked());
}

void CentralWidget::onToTodayCheckChecked(bool checked)
{
    ui->ratingToEdit->setEnabled(!checked);
}

void CentralWidget::onCustomRatingTemplateFailed()
{
    m_error |= RatingTemplateFailed;
    ui->retryTemplateButton->setEnabled(true);
    retranslateUi();
}

void CentralWidget::onRatingsCalculationFailed()
{
    m_error |= RatingCalculationError;
    retranslateUi();
}
void CentralWidget::onRatingsCalculated()
{
    m_error &= ~RatingCalculationError;
    retranslateUi();
}

void CentralWidget::retrySetsDownload()
{
    ui->retryBasicDownloadButton->setEnabled(false);
    m_object->downloadSetsMTGAH();
}

void CentralWidget::retryTemplateDownload()
{
    ui->retryTemplateButton->setEnabled(false);
    m_object->getCustomRatingTemplate();
}

void CentralWidget::onCustomRatingsTemplateDownloaded()
{
    m_error &= ~RatingTemplateFailed;
    retranslateUi();
}

void CentralWidget::updateRatingsFiler()
{
    m_object->filterRatings(ui->searchCardEdit->text());
}

CentralWidget::CentralWidget(QWidget *parent)
    : QWidget(parent)
    , m_error(NoError)
    , ui(new Ui::CentralWidget)
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
    ui->cancelUploadButton->hide();
    ui->formatsCombo->setModel(m_object->formatsModel());
    ui->ratingsView->setModel(m_object->ratingsModel());
    ui->ratingsView->setColumnHidden(RatingsModel::rmcArenaId, true);
    ui->ratingsView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->ratingsView->sortByColumn(RatingsModel::rmcName, Qt::AscendingOrder);
    ui->ratingsView->setItemDelegateForColumn(RatingsModel::rmcRating, new RatingsDelegate(this));
    ui->slRatingsView->setModel(m_object->seventeenLandsRatingsModel());
    ui->slRatingsView->setItemDelegateForColumn(SLRatingsModel::slmLastUpdate, new TextDateDelegate(this));
    ui->customRatingsView->setModel(m_object->customRatingsModel());
    ui->customRatingsView->setColumnHidden(CustomRatingModel::crmIdArena, true);
    ui->customRatingsView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->customRatingsView->sortByColumn(CustomRatingModel::crmName, Qt::AscendingOrder);
    ui->customRatingsView->setItemDelegateForColumn(CustomRatingModel::crmRating, new RatingsDelegate(this));
    PercentDelegate *percentDelegate = new PercentDelegate(this);
    for (int metric : {GEnums::SLwin_rate, GEnums::SLopening_hand_win_rate, GEnums::SLdrawn_win_rate, GEnums::SLever_drawn_win_rate,
                       GEnums::SLnever_drawn_win_rate, GEnums::SLdrawn_improvement_win_rate})
        ui->slRatingsView->setItemDelegateForColumn(metric + 2, percentDelegate);
    DecimalDelegate *decimalDelegate = new DecimalDelegate(this);
    for (int metric : {GEnums::SLavg_seen, GEnums::SLavg_pick})
        ui->slRatingsView->setItemDelegateForColumn(metric + 2, decimalDelegate);
    QCompleter *searchCompleter = new QCompleter(this);
    searchCompleter->setModel(m_object->ratingsModel());
    searchCompleter->setCompletionColumn(RatingsModel::rmcName);
    ui->searchCardEdit->setCompleter(searchCompleter);
    ui->notesView->setModel(m_object->SLMetricsModel());
    auto SLMetricsProxy = new NoCheckProxy(this);
    SLMetricsProxy->setSourceModel(m_object->SLMetricsModel());
    ui->ratingBasedCombo->setModel(SLMetricsProxy);
    ui->ratingBasedCombo->setCurrentIndex(GEnums::SLever_drawn_win_rate);
    ui->ratingToEdit->setDate(QDate::currentDate());
    retranslateUi();

    connect(ui->ratingBasedButton, &QPushButton::clicked, this,
            []() { QDesktopServices::openUrl(QUrl::fromUserInput(QStringLiteral("https://www.17lands.com/metrics_definitions"))); });
    connect(ui->remembePwdCheck, &QCheckBox::stateChanged, this, &CentralWidget::onRememberPass);
    connect(ui->loginButton, &QPushButton::clicked, this, &CentralWidget::doLogin);
    connect(ui->logoutButton, &QPushButton::clicked, this, &CentralWidget::doLogout);
    connect(ui->retryBasicDownloadButton, &QPushButton::clicked, this, &CentralWidget::retrySetsDownload);
    connect(ui->retryTemplateButton, &QPushButton::clicked, this, &CentralWidget::retryTemplateDownload);
    connect(ui->downloadButton, &QPushButton::clicked, this, &CentralWidget::do17Ldownload);
    connect(ui->uploadButton, &QPushButton::clicked, this, std::bind(&CentralWidget::doMtgahUpload, this, false));
    connect(ui->clearRatingsButton, &QPushButton::clicked, this, std::bind(&CentralWidget::doMtgahUpload, this, true));
    connect(ui->allSetsButton, &QPushButton::clicked, this, &CentralWidget::selectAllSets);
    connect(ui->noSetButton, &QPushButton::clicked, this, &CentralWidget::selectNoSets);
    connect(ui->searchCardEdit, &QLineEdit::textChanged, this, &CentralWidget::updateRatingsFiler);
    connect(ui->draftableSetsCheck, &QCheckBox::clicked, m_object, &MainObject::showOnlyDraftableSets);
    connect(ui->cancelUploadButton, &QPushButton::clicked, m_object, &MainObject::cancelUpload);
    connect(ui->ratingTimeGroup, &QGroupBox::toggled, this, &CentralWidget::onRatingTimeGroupChecked);
    connect(ui->toTodayCheck, &QCheckBox::clicked, this, &CentralWidget::onToTodayCheckChecked);
    connect(m_object, &MainObject::initialisationFailed, this, &CentralWidget::onInitialisationFailed);
    connect(m_object, &MainObject::customRatingTemplate, this, &CentralWidget::onCustomRatingsTemplateDownloaded);
    connect(m_object, &MainObject::customRatingTemplateFailed, this, &CentralWidget::onCustomRatingTemplateFailed);
    connect(m_object, &MainObject::ratingsCalculationFailed, this, &CentralWidget::onRatingsCalculationFailed);
    connect(m_object, &MainObject::ratingsCalculated, this, &CentralWidget::onRatingsCalculated);
    connect(m_object, &MainObject::downloadSetsMTGAHFailed, this, &CentralWidget::onDownloadSetsMTGAHFailed);
    connect(m_object, &MainObject::setsMTGAHDownloaded, this, &CentralWidget::onSetsMTGAHDownloaded);
    connect(m_object, &MainObject::ratingsUploaded, this, &CentralWidget::onUploadedRatings);
    connect(m_object, &MainObject::startProgress, this, &CentralWidget::onStartProgress);
    connect(m_object, &MainObject::updateProgress, this, &CentralWidget::onUpdateProgress);
    connect(m_object, &MainObject::increaseProgress, this, &CentralWidget::onIncreaseProgress);
    connect(m_object, &MainObject::endProgress, this, &CentralWidget::onEndProgress);
    connect(m_object, &MainObject::loggedIn, this, &CentralWidget::onLogin);
    connect(m_object, &MainObject::loginFalied, this, &CentralWidget::onLoginError);
    connect(m_object, &MainObject::loggedOut, this, &CentralWidget::onLogout);
    connect(m_object, &MainObject::logoutFailed, this, &CentralWidget::onLogoutError);
    connect(m_object, &MainObject::SLDownloadFinished, this, &CentralWidget::on17LandsDownloadFinished);
    connect(m_object, &MainObject::SLDownloadFailed, this, &CentralWidget::on17Lerror);
    connect(m_object, &MainObject::loadUserPass, this, &CentralWidget::onLoadUserPass);
    connect(m_object, &MainObject::loadDownloadFormat, this, &CentralWidget::onLoadDownloadFormat);
    connect(m_object, &MainObject::loadUploadRating, this, &CentralWidget::onLoadUploadRating);
    connect(m_object, &MainObject::ratingUploaded, this, &CentralWidget::updatedUploadedStatus);
    connect(m_object, &MainObject::showOnlyDraftableSetsChanged, this, &CentralWidget::onShowOnlyDraftableSetsChanged);
    connect(m_object->setsModel(), &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &, const QModelIndex &, const QVector<int> &roles) {
                if (roles.isEmpty() || roles.contains(Qt::CheckStateRole))
                    updateRatingsFiler();
            });
    connect(m_object->setsModel(), &QAbstractItemModel::dataChanged, this, &CentralWidget::checkDownloadButtonEnabled);
}

CentralWidget::~CentralWidget()
{
    delete ui;
}

CentralWidget::CurrentErrors CentralWidget::errors() const
{
    return m_error;
}

void CentralWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    else
        QWidget::changeEvent(event);
}

void CentralWidget::retranslateUi()
{
    ui->retranslateUi(this);
    ui->savePwdWarningLabel->setText(tr("Your password will be stored in plain text in %1").arg(m_object->configPath()));
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
    if (m_error & SLDownloadError)
        errorStrings.append(tr("Error downloading ratings ratings form 17Lands! Check your internet connection"));
    if (m_error & RatingCalculationError)
        errorStrings.append(tr("Error computing ratings to upload, retry"));
    ui->errorLabel->setText(errorStrings.join(QChar(QLatin1Char('\n'))));
    m_object->retranslateModels();
}

void CentralWidget::setAllSetsSelection(Qt::CheckState check)
{
    for (int i = 0, iEnd = m_object->setsModel()->rowCount(); i < iEnd; ++i)
        m_object->setsModel()->setData(m_object->setsModel()->index(i, 0), check, Qt::CheckStateRole);
}

void CentralWidget::onStartProgress(MainObject::Operations op, const QString &description, int max, int min)
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
        ui->cancelUploadButton->setVisible(op == MainObject::opUploadMTGAH);
    }
    progressQueue.append(ProgressElement(op, description, max, min, min));
}

void CentralWidget::onUpdateProgress(MainObject::Operations op, int val)
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

void CentralWidget::onIncreaseProgress(MainObject::Operations op, int val)
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

void CentralWidget::onEndProgress(MainObject::Operations op)
{
    const auto pBegin = progressQueue.begin();
    for (auto i = pBegin, iEnd = progressQueue.end(); i != iEnd; ++i) {
        if (i->m_operation == op) {
            if (i == pBegin) {
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
#ifdef QT_DEBUG
    qDebug() << op;
#endif
    Q_UNREACHABLE();
}

void CentralWidget::toggleLoginLogoutButtons()
{
    for (QPushButton *button : {ui->loginButton, ui->logoutButton}) {
        button->setVisible(!button->isVisible());
        button->setEnabled(true);
    }
    checkUploadButtonEnabled();
}

void CentralWidget::checkUploadButtonEnabled()
{
    const bool enabled = !ui->loginButton->isVisible() && m_object->oneSetSelected() && (m_error & RatingTemplateFailed) == 0;
    ui->uploadButton->setEnabled(enabled);
    ui->clearRatingsButton->setEnabled(enabled);
}

void CentralWidget::enableAll(bool enable)
{
    QWidget *widToChange[] = {ui->MTGAHelperGroup, ui->customRatingsGroup, ui->setsGroup,
                              ui->uploadButton,    ui->clearRatingsButton, ui->downloadButton};
    for (QWidget *wid : widToChange)
        wid->setEnabled(enable);
    if (enable)
        checkUploadButtonEnabled();
}
