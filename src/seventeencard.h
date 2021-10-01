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

#ifndef SEVENTEENCARD_H
#define SEVENTEENCARD_H
#include <QString>
class SeventeenCard
{
public:
    SeventeenCard();
    explicit SeventeenCard(const QString &name);
    SeventeenCard(const SeventeenCard &other) = default;
    SeventeenCard &operator=(const SeventeenCard &other) = default;
    bool operator==(const SeventeenCard &other) const;
    bool operator!=(const SeventeenCard &other) const { return !operator==(other); }

public:
    int seen_count;
    double avg_seen;
    int pick_count;
    double avg_pick;
    int game_count;
    double win_rate;
    int opening_hand_game_count;
    double opening_hand_win_rate;
    int drawn_game_count;
    double drawn_win_rate;
    int ever_drawn_game_count;
    double ever_drawn_win_rate;
    int never_drawn_game_count;
    double never_drawn_win_rate;
    double drawn_improvement_win_rate;
    QString name;
};
size_t qHash(const SeventeenCard &card, size_t seed = 0);
#endif
