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

#ifndef GLOBALS_H
#define GLOBALS_H
#include <QObject>
#include <QString>
#include <QSqlDatabase>
QString appDataPath();
QString appSettingsPath();
QSqlDatabase openDb(const QString &dbName);
namespace GEnums {
enum SetType {
    stcore = 0x1,
    stexpansion = 0x2,
    stmasters = 0x4,
    stmasterpiece = 0x8,
    stfrom_the_vault = 0x10,
    stspellbook = 0x20,
    stpremium_deck = 0x40,
    stduel_deck = 0x80,
    stdraft_innovation = 0x100,
    sttreasure_chest = 0x200,
    stcommander = 0x400,
    stplanechase = 0x800,
    starchenemy = 0x1000,
    stvanguard = 0x2000,
    stfunny = 0x4000,
    ststarter = 0x8000,
    stbox = 0x10000,
    stpromo = 0x20000,
    sttoken = 0x40000,
    stmemorabilia = 0x80000,

    stEnd = 0x100000
};
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
}
Q_DECLARE_METATYPE(GEnums::SLMetrics)
#ifdef QT_DEBUG
extern std::atomic_bool dtFailInit;
extern std::atomic_bool dtFailLogin;
extern std::atomic_bool dtFailLogout;
extern std::atomic_bool dtFailCustomRatingTemplate;
extern std::atomic_bool dtFail17LRatings;
extern std::atomic_bool dtFailUploadRating;
extern std::atomic_bool dtFailRatingCalculation;
extern std::atomic_bool dtFailSetsMTGAH;
extern std::atomic_bool dtFailSetsScryfall;
#endif
#endif
