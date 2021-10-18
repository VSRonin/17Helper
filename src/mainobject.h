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

#ifndef MAINOBJECT_H
#define MAINOBJECT_H
#include <QObject>
#include "worker.h"
class QThread;
class QStandardItemModel;
class QAbstractItemModel;
class SeventeenCard;
class QSqlQueryModel;
class CheckableProxy;
class MainObject : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MainObject)
public:
    explicit MainObject(QObject *parent = nullptr);
    ~MainObject();
    enum SLMetrics {
        SLseen_count,
        SLavg_seen,
        SLpick_count,
        SLavg_pick,
        SLgame_count,
        SLwin_rate,
        SLopening_hand_game_count,
        SLopening_hand_win_rate,
        SLdrawn_game_count,
        SLdrawn_win_rate,
        SLever_drawn_game_count,
        SLever_drawn_win_rate,
        SLnever_drawn_game_count,
        SLnever_drawn_win_rate,
        SLdrawn_improvement_win_rate,

        SLCount
    };
    enum DraftFormats {
        dfPremierDraft,
        dfQuickDraft,
        dfTradDraft,
        dfSealed,
        dfTradSealed,

        dfCount
    };
    enum { DraftableSet = Worker::stcore | Worker::stexpansion | Worker::stdraft_innovation };
    QAbstractItemModel *SLMetricsModel() const;
    QAbstractItemModel *setsModel() const;
    QAbstractItemModel *formatsModel() const;
public slots:
    void tryLogin(const QString &userName, const QString &password, bool rememberMe = false);
    void logOut();
    void retranslateModels();
private slots:
    void onWorkerInit();
    void onLoggedIn();
    void onSetsScryfall(bool needsUpdate);
signals:
    void loggedIn();
    void loginFalied(const QString &error);
    void loggedOut();
    void logoutFailed(const QString &error);
    void initialisationFailed();
    void setsReady();

private:
    double ratingValue(const SeventeenCard &card, SLMetrics method) const;
    QString commentString(const SeventeenCard &card, const QLocale &locale) const;
    void fillMetrics();
    void fillFormats();
    void selectSetsModel();
    Worker *m_worker;
    QThread *m_workerThread;
    const QString m_objectDbName;
    QStringList SLcodes;
    QStandardItemModel *m_SLMetricsModel;
    QStandardItemModel *m_formatsModel;
    QSqlQueryModel *m_setsModel;
    CheckableProxy *m_setsProxy;
};

#endif
