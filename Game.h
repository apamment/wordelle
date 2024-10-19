#pragma once

#include <vector>
#include <string>
#include "sqlite3.h"

class Game {
public:
    bool load();
    void check_new_day();
    bool check_played();
    void play_game();
    void show_scores_today();
    void show_scores_last_month();
    void score_this_month();

private:
    std::vector<std::string> answers;
    std::vector<std::string> words;

    std::string todays_word;

    void record_score(int score);
    bool open_database(sqlite3 **db);
};