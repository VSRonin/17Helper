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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QWidget>
#include <QMultiHash>
namespace Ui { class MainWindow; }
class QStandardItemModel;
class Worker;
class RatingsModel;
class QSortFilterProxyModel;
class SeventeenCard;
class MainWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MainWindow)
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    enum CurrentError{
        NoError=0x0
        , LoginError=0x1
        , LogoutError=0x2
        , MTGAHSetsError=0x4
        , RatingTemplateFailed=0x8
    };
    Q_DECLARE_FLAGS(CurrentErrors, CurrentError)
    CurrentErrors errors() const;
protected:
    void changeEvent(QEvent *event) override;
    void retranslateUi();
private:
    CurrentErrors m_error;
    QStandardItemModel* m_setsModel;
    QStandardItemModel* m_SLMetricsModel;
    RatingsModel* m_ratingsModel;
    QSortFilterProxyModel* m_ratingsProxy;
    Worker *m_worker;
    Ui::MainWindow *ui;
    void setSetsSectionEnabled(bool enabled);
    void setAllSetsSelection(Qt::CheckState check);
    enum SLMetrics{
        SLseen_count
        ,SLavg_seen
        ,SLpick_count
        ,SLavg_pick
        ,SLgame_count
        ,SLwin_rate
        ,SLopening_hand_game_count
        ,SLopening_hand_win_rate
        ,SLdrawn_game_count
        ,SLdrawn_win_rate
        ,SLever_drawn_game_count
        ,SLever_drawn_win_rate
        ,SLnever_drawn_game_count
        ,SLnever_drawn_win_rate
        ,SLdrawn_improvement_win_rate

        , SLCount
    };
    QStringList SLcodes;
    QString commentString(const SeventeenCard& card) const;
    double ratingValue(const SeventeenCard& card) const;
private slots:
    void toggleLoginLogoutButtons();
    void doLogin();
    void doLogout();
    void do17Ldownload();
    void fillSets(const QStringList& sets);
    void fillSetNames(const QHash<QString,QString>& setNames);
    void onDownloaded17LRatings(const QString& set, const QSet<SeventeenCard> &ratings);
    void fillMetrics();
    void enableSetsSection(){setSetsSectionEnabled(true);}
    void disableSetsSection(){setSetsSectionEnabled(false);}
    void onLogin();
    void onLoginError();
    void onLogout();
    void onLogoutError();
    void onMTGAHSetsError();
    void onScryfallSetsError();
    void onTemplateDownloadFailed();
    void selectAllSets(){setAllSetsSelection(Qt::Checked);}
    void selectNoSets(){setAllSetsSelection(Qt::Unchecked);}
    void retrySetsDownload();
    void retryTemplateDownload();
    void onCustomRatingsTemplateDownloaded();
    void updateRatingsFiler();
};
Q_DECLARE_OPERATORS_FOR_FLAGS(MainWindow::CurrentErrors);
#endif
