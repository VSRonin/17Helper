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
#include "worker.h"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QHeaderView>
#include <QIdentityProxyModel>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
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
    ui->loginButton->setEnabled(false);
    ui->usernameEdit->setEnabled(false);
    ui->pwdEdit->setEnabled(false);
    m_worker->tryLogin(ui->usernameEdit->text(), ui->pwdEdit->text());
    ui->pwdEdit->clear();
}

void MainWindow::doLogout()
{
    ui->logoutButton->setEnabled(false);
    m_worker->logOut();
}

void MainWindow::do17Ldownload()
{
    ui->downloadButton->setEnabled(false);
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
    m_worker->get17LRatings(sets, ui->formatsCombo->currentData().toString());
}

void MainWindow::doMtgahUpload()
{
    ui->uploadButton->setEnabled(false);
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
    m_worker->uploadRatings(sets);
}

void MainWindow::onAllRatingsUploaded()
{
    ui->uploadButton->setEnabled(true);
    ui->progressBar->setVisible(false);
    ui->progressLabel->setVisible(false);
}

void MainWindow::fillSets(const QStringList &sets)
{
    m_error &= ~MTGAHSetsError;
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
    retranslateUi();
}

void MainWindow::fillSetNames(const QHash<QString, QString> &setNames)
{
    for (int i = 0, iEnd = m_setsModel->rowCount(); i < iEnd; ++i) {
        const QString setToFind = m_setsModel->index(i, 0).data(Qt::UserRole).toString();
        auto nameIter = setNames.constFind(setToFind);
        if (nameIter != setNames.constEnd())
            m_setsModel->setData(m_setsModel->index(i, 0), nameIter.value());
    }
    retranslateUi();
}

void MainWindow::onDownloaded17LRatings(const QString &set, const QSet<SeventeenCard> &ratings)
{
    Q_ASSERT(!ratings.isEmpty());
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
    }
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
    int setCount = 0;
    for (int i = 0, iEnd = m_setsModel->rowCount(); i < iEnd; ++i) {
        const QModelIndex &idx = m_setsModel->index(i, 0);
        if (idx.data(Qt::CheckStateRole).toInt() == Qt::Checked)
            ++setCount;
    }
    ui->progressBar->setValue(ui->progressBar->maximum() - progress);
}

void MainWindow::fillMetrics()
{
    for (int i = 0; i < SLCount; ++i) {
        QStandardItem *item = new QStandardItem;
        item->setData(i, Qt::UserRole);
        if (i == SLdrawn_win_rate || i == SLavg_pick)
            item->setData(Qt::Checked, Qt::CheckStateRole);
        else
            item->setData(Qt::Unchecked, Qt::CheckStateRole);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        m_SLMetricsModel->setItem(i, 0, item);
    }
}

void MainWindow::onLogoutError()
{
    m_error |= LogoutError;
    ui->logoutButton->setEnabled(true);
    retranslateUi();
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
    m_worker->downloadSetsMTGAH();
}

void MainWindow::retryTemplateDownload()
{
    ui->retryTemplateButton->setEnabled(false);
    m_worker->getCustomRatingTemplate();
}

void MainWindow::onCustomRatingsTemplateDownloaded()
{
    m_error &= ~RatingTemplateFailed;
    m_ratingsModel->setRatingsTemplate(m_worker->ratingsTemplate());
    retranslateUi();
}

void MainWindow::updateRatingsFiler()
{
    QString regExpString;
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
    m_ratingsProxy->setFilterRegularExpression(regExpString);
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

void MainWindow::onLogin()
{
    m_error &= ~LoginError;
    m_worker->getCustomRatingTemplate();
    toggleLoginLogoutButtons();
    enableSetsSection();
    retranslateUi();
}

void MainWindow::onLoginError()
{
    m_error |= LoginError;
    ui->loginButton->setEnabled(true);
    ui->usernameEdit->setEnabled(true);
    ui->pwdEdit->setEnabled(true);
    retranslateUi();
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , m_error(NoError)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_worker = new Worker(this);
    m_setsModel = new QStandardItemModel(this);
    m_setsModel->insertColumn(0);
    ui->setsView->setModel(m_setsModel);
    ui->logoutButton->hide();
    ui->errorLabel->hide();
    ui->progressBar->hide();
    ui->progressLabel->hide();
    ui->retryBasicDownloadButton->hide();
    ui->formatsCombo->addItem(QString(), QStringLiteral("PremierDraft"));
    ui->formatsCombo->addItem(QString(), QStringLiteral("QuickDraft"));
    ui->formatsCombo->addItem(QString(), QStringLiteral("TradDraft"));
    ui->formatsCombo->addItem(QString(), QStringLiteral("Sealed"));
    ui->formatsCombo->addItem(QString(), QStringLiteral("TradSealed"));
    m_ratingsModel = new RatingsModel(this);
    m_ratingsModel->setRatingsTemplate(m_worker->ratingsTemplate());
    m_ratingsProxy = new QSortFilterProxyModel(this);
    m_ratingsProxy->setSourceModel(m_ratingsModel);
    m_ratingsProxy->setFilterKeyColumn(RatingsModel::rmcSet);
    ui->ratingsView->setModel(m_ratingsProxy);
    ui->ratingsView->setColumnHidden(RatingsModel::rmcArenaId, true);
    ui->ratingsView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->ratingsView->sortByColumn(RatingsModel::rmcName, Qt::AscendingOrder);
    ui->ratingsView->setItemDelegateForColumn(RatingsModel::rmcRating, new RatingsDelegate(this));
    m_SLMetricsModel = new QStandardItemModel(SLCount, 1, this);
    fillMetrics();
    ui->notesView->setModel(m_SLMetricsModel);
    auto SLMetricsProxy = new NoCheckProxy(this);
    SLMetricsProxy->setSourceModel(m_SLMetricsModel);
    ui->ratingBasedCombo->setModel(SLMetricsProxy);
    ui->ratingBasedCombo->setCurrentIndex(SLdrawn_win_rate);
    disableSetsSection();
    retranslateUi();

    connect(ui->ratingBasedButton, &QPushButton::clicked, this,
            []() { QDesktopServices::openUrl(QUrl::fromUserInput(QStringLiteral("https://www.17lands.com/metrics_definitions"))); });
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::doLogin);
    connect(ui->logoutButton, &QPushButton::clicked, this, &MainWindow::doLogout);
    connect(ui->retryBasicDownloadButton, &QPushButton::clicked, this, &MainWindow::retrySetsDownload);
    connect(ui->retryTemplateButton, &QPushButton::clicked, this, &MainWindow::retryTemplateDownload);
    connect(ui->downloadButton, &QPushButton::clicked, this, &MainWindow::do17Ldownload);
    connect(ui->uploadButton, &QPushButton::clicked, this, &MainWindow::doMtgahUpload);
    connect(ui->allSetsButton, &QPushButton::clicked, this, &MainWindow::selectAllSets);
    connect(ui->noSetButton, &QPushButton::clicked, this, &MainWindow::selectNoSets);
    connect(m_worker, &Worker::setsMTGAH, this, &MainWindow::fillSets);
    connect(m_worker, &Worker::downloadSetsMTGAHFailed, this, &MainWindow::onMTGAHSetsError);
    connect(m_worker, &Worker::setsScryfall, this, &MainWindow::fillSetNames);
    connect(m_worker, &Worker::downloadSetsScryfallFailed, this, &MainWindow::onScryfallSetsError);
    connect(m_worker, &Worker::loggedIn, this, &MainWindow::onLogin);
    connect(m_worker, &Worker::loginFalied, this, &MainWindow::onLoginError);
    connect(m_worker, &Worker::loggedOut, this, &MainWindow::onLogout);
    connect(m_worker, &Worker::logoutFailed, this, &MainWindow::onLogoutError);
    connect(m_worker, &Worker::customRatingTemplate, this, &MainWindow::onCustomRatingsTemplateDownloaded);
    connect(m_worker, &Worker::customRatingTemplateFailed, this, &MainWindow::onTemplateDownloadFailed);
    connect(m_worker, &Worker::downloaded17LRatings, this, &MainWindow::onDownloaded17LRatings);
    connect(m_worker, &Worker::downloadedAll17LRatings, this, &MainWindow::onDownloadedAll17LRatings);
    connect(m_worker, &Worker::ratingsUploadMaxProgress, this, &MainWindow::onRatingsUploadMaxProgress);
    connect(m_worker, &Worker::ratingsUploadProgress, this, &MainWindow::onRatingsUploadProgress);
    connect(m_worker, &Worker::allRatingsUploaded, this, &MainWindow::onAllRatingsUploaded);
    connect(m_setsModel, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &, const QModelIndex &, const QVector<int> &roles) {
        if (roles.isEmpty() || roles.contains(Qt::CheckStateRole))
            updateRatingsFiler();
    });
    connect(m_ratingsModel, &QAbstractItemModel::modelReset, this, &MainWindow::updateRatingsFiler);
    m_worker->downloadSetsMTGAH();
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
    SLcodes = QStringList(SLCount, QString());
    SLcodes[SLseen_count] = tr("#S");
    SLcodes[SLavg_seen] = tr("ALSA");
    SLcodes[SLpick_count] = tr("#P");
    SLcodes[SLavg_pick] = tr("ATA");
    SLcodes[SLgame_count] = tr("#GP");
    SLcodes[SLwin_rate] = tr("GPWR");
    SLcodes[SLopening_hand_game_count] = tr("#OH");
    SLcodes[SLopening_hand_win_rate] = tr("OHWR");
    SLcodes[SLdrawn_game_count] = tr("#GD");
    SLcodes[SLdrawn_win_rate] = tr("GDWR");
    SLcodes[SLever_drawn_game_count] = tr("#GIH");
    SLcodes[SLever_drawn_win_rate] = tr("GIHWR");
    SLcodes[SLnever_drawn_game_count] = tr("#GND");
    SLcodes[SLnever_drawn_win_rate] = tr("GNDWR");
    SLcodes[SLdrawn_improvement_win_rate] = tr("IWD");

    m_SLMetricsModel->item(SLseen_count)->setData(tr("Number Seen (%1)").arg(SLcodes.at(SLseen_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLavg_seen)->setData(tr("Average Last Seen At (%1)").arg(SLcodes.at(SLavg_seen)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLpick_count)->setData(tr("Number Picked (%1)").arg(SLcodes.at(SLpick_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLavg_pick)->setData(tr("Average Taken At (%1)").arg(SLcodes.at(SLavg_pick)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLgame_count)->setData(tr("Number of Games Played (%1)").arg(SLcodes.at(SLgame_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLwin_rate)->setData(tr("Games Played Win Rate (%1)").arg(SLcodes.at(SLwin_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLopening_hand_game_count)
            ->setData(tr("Number of Games in Opening Hand (%1)").arg(SLcodes.at(SLopening_hand_game_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLopening_hand_win_rate)
            ->setData(tr("Opening Hand Win Rate (%1)").arg(SLcodes.at(SLopening_hand_win_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLdrawn_game_count)->setData(tr("Number of Games Drawn (%1)").arg(SLcodes.at(SLdrawn_game_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLdrawn_win_rate)->setData(tr("Games Drawn Win Rate (%1)").arg(SLcodes.at(SLdrawn_win_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLever_drawn_game_count)
            ->setData(tr("Number of Games In Hand (%1)").arg(SLcodes.at(SLever_drawn_game_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLever_drawn_win_rate)->setData(tr("Games in Hand Win Rate (%1)").arg(SLcodes.at(SLever_drawn_win_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLnever_drawn_game_count)
            ->setData(tr("Number of Games Not Drawn (%1)").arg(SLcodes.at(SLnever_drawn_game_count)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLnever_drawn_win_rate)
            ->setData(tr("Games Not Drawn Win Rate (%1)").arg(SLcodes.at(SLnever_drawn_win_rate)), Qt::DisplayRole);
    m_SLMetricsModel->item(SLdrawn_improvement_win_rate)
            ->setData(tr("Improvement When Drawn (%1)").arg(SLcodes.at(SLdrawn_improvement_win_rate)), Qt::DisplayRole);
}

void MainWindow::setSetsSectionEnabled(bool enabled)
{
    ui->setsGroup->setEnabled(enabled);
    ui->customRatingsGroup->setEnabled(enabled);
}

void MainWindow::setAllSetsSelection(Qt::CheckState check)
{
    for (int i = 0, iEnd = m_setsModel->rowCount(); i < iEnd; ++i)
        m_setsModel->item(i)->setCheckState(check);
}

QString MainWindow::commentString(const SeventeenCard &card) const
{
    QStringList result;
    if (m_SLMetricsModel->index(SLseen_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLseen_count) + QLatin1Char(':') + locale().toString(card.seen_count));
    if (m_SLMetricsModel->index(SLavg_seen, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLavg_seen) + QLatin1Char(':') + locale().toString(card.avg_seen, 'f', 2));
    if (m_SLMetricsModel->index(SLpick_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLpick_count) + QLatin1Char(':') + locale().toString(card.pick_count));
    if (m_SLMetricsModel->index(SLavg_pick, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLavg_pick) + QLatin1Char(':') + locale().toString(card.avg_pick, 'f', 2));
    if (m_SLMetricsModel->index(SLgame_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLgame_count) + QLatin1Char(':') + locale().toString(card.game_count));
    if (m_SLMetricsModel->index(SLwin_rate, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLwin_rate) + QLatin1Char(':') + locale().toString(card.win_rate * 100.0, 'f', 2) + locale().percent());
    if (m_SLMetricsModel->index(SLopening_hand_game_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLopening_hand_game_count) + QLatin1Char(':') + locale().toString(card.opening_hand_game_count));
    if (m_SLMetricsModel->index(SLopening_hand_win_rate, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLopening_hand_win_rate) + QLatin1Char(':') + locale().toString(card.opening_hand_win_rate * 100.0, 'f', 2)
                      + locale().percent());
    if (m_SLMetricsModel->index(SLdrawn_game_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLdrawn_game_count) + QLatin1Char(':') + locale().toString(card.drawn_game_count));
    if (m_SLMetricsModel->index(SLdrawn_win_rate, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLdrawn_win_rate) + QLatin1Char(':') + locale().toString(card.drawn_win_rate * 100.0, 'f', 2) + locale().percent());
    if (m_SLMetricsModel->index(SLever_drawn_game_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLever_drawn_game_count) + QLatin1Char(':') + locale().toString(card.ever_drawn_game_count));
    if (m_SLMetricsModel->index(SLever_drawn_win_rate, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLever_drawn_win_rate) + QLatin1Char(':') + locale().toString(card.ever_drawn_win_rate * 100.0, 'f', 2)
                      + locale().percent());
    if (m_SLMetricsModel->index(SLnever_drawn_game_count, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLnever_drawn_game_count) + QLatin1Char(':') + locale().toString(card.never_drawn_game_count));
    if (m_SLMetricsModel->index(SLnever_drawn_win_rate, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLnever_drawn_win_rate) + QLatin1Char(':') + locale().toString(card.never_drawn_win_rate * 100.0, 'f', 2)
                      + locale().percent());
    if (m_SLMetricsModel->index(SLdrawn_improvement_win_rate, 0).data(Qt::CheckStateRole).toInt() == Qt::Checked)
        result.append(SLcodes.at(SLdrawn_improvement_win_rate) + QLatin1Char(':') + locale().toString(card.drawn_improvement_win_rate * 100.0, 'f', 2)
                      + locale().percent());
    return result.join(QLatin1Char(' '));
}

double MainWindow::ratingValue(const SeventeenCard &card) const
{
    switch (ui->ratingBasedCombo->currentData().toInt()) {
    case SLseen_count:
        return card.seen_count;
    case SLavg_seen:
        return card.avg_seen;
    case SLpick_count:
        return card.pick_count;
    case SLavg_pick:
        return card.avg_pick;
    case SLgame_count:
        return card.game_count;
    case SLwin_rate:
        return card.win_rate;
    case SLopening_hand_game_count:
        return card.opening_hand_game_count;
    case SLopening_hand_win_rate:
        return card.opening_hand_win_rate;
    case SLdrawn_game_count:
        return card.drawn_game_count;
    case SLdrawn_win_rate:
        return card.drawn_win_rate;
    case SLever_drawn_game_count:
        return card.ever_drawn_game_count;
    case SLever_drawn_win_rate:
        return card.ever_drawn_win_rate;
    case SLnever_drawn_game_count:
        return card.never_drawn_game_count;
    case SLnever_drawn_win_rate:
        return card.never_drawn_win_rate;
    case SLdrawn_improvement_win_rate:
        return card.drawn_improvement_win_rate;
    default:
        return 0;
    }
}

void MainWindow::toggleLoginLogoutButtons()
{
    for (QPushButton *button : {ui->loginButton, ui->logoutButton}) {
        button->setVisible(!button->isVisible());
        button->setEnabled(true);
    }
}
