#include "seventeencard.h"
#include <QHash>
SeventeenCard::SeventeenCard() : SeventeenCard(QString()) {}

SeventeenCard::SeventeenCard(const QString &nm)
    : seen_count(0), avg_seen(0.0), pick_count(0), avg_pick(0.0), game_count(0),
      win_rate(0.0), opening_hand_game_count(0), opening_hand_win_rate(0.0),
      drawn_game_count(0), drawn_win_rate(0.0), ever_drawn_game_count(0),
      ever_drawn_win_rate(0.0), never_drawn_game_count(0),
      never_drawn_win_rate(0.0), drawn_improvement_win_rate(0.0), name(nm) {}

bool SeventeenCard::operator==(const SeventeenCard &other) const {
  return name == other.name
      /*&& avg_seen == other.avg_seen
      && pick_count == other.pick_count
      && avg_pick == other.avg_pick
      && game_count == other.game_count
      && win_rate == other.win_rate
      && opening_hand_game_count == other.opening_hand_game_count
      && opening_hand_win_rate == other.opening_hand_win_rate
      && drawn_game_count == other.drawn_game_count
      && drawn_win_rate == other.drawn_win_rate
      && ever_drawn_game_count == other.ever_drawn_game_count
      && ever_drawn_win_rate == other.ever_drawn_win_rate
      && never_drawn_game_count == other.never_drawn_game_count
      && never_drawn_win_rate == other.never_drawn_win_rate
      && drawn_improvement_win_rate == other.drawn_improvement_win_rate
      && seen_count == other.seen_count
      */
      ;
}

size_t qHash(const SeventeenCard &card, size_t seed) {
  return qHash(card.name, seed);
}
