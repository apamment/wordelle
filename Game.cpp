#include "Game.h"
#include "sqlite3.h"
extern "C" {
    #include "magidoor/MagiDoor.h"
}
#include <fstream>
#include <string>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <cstring>

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

struct player_s {
    std::string player;
    int score;
};


void Game::score_this_month() {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    static const char *sql = "SELECT player, score FROM scores WHERE month = ? AND year = ?";

    if (!open_database(&db)) {
        md_exit(-1);
    }
    time_t now = time(NULL);
    struct tm now_tm;
#ifdef _MSC_VER
    localtime_s(&now_tm, &now);
#else
    localtime_r(&now, &now_tm);
#endif
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        md_printf("%s\r\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        md_exit(-1);
    }

    int month = now_tm.tm_mon + 1;
    int year = now_tm.tm_year + 1900;

    sqlite3_bind_int(stmt, 1, month);
    sqlite3_bind_int(stmt, 2, year);

    std::vector<struct player_s> scores;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        bool found = false;
        for (size_t i = 0; i < scores.size(); i++) {
            if (scores.at(i).player == std::string((const char *)sqlite3_column_text(stmt, 0))) {
                scores.at(i).score += sqlite3_column_int(stmt, 1);
                found = true;
                break;
            }
        }

        if (!found) {
            struct player_s p;
            p.player = std::string((const char *)sqlite3_column_text(stmt, 0));
            p.score = sqlite3_column_int(stmt, 1);

            scores.push_back(p);
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    auto comp = [] (struct player_s a, struct player_s b) {
        return a.score > b.score;
    };

    std::sort(scores.begin(), scores.end(), comp);

    for (size_t i = 0; i < scores.size(); i++) {
        if (scores.at(i).player == std::string(mdcontrol.user_alias)) {
            md_set_cursor(9, 46);
            md_printf("You are ranked no. %d", i + 1);
            md_set_cursor(10, 46);
            md_printf("Your score so far: %d", scores.at(i).score);
        }
    }
}

void Game::show_scores_last_month() {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    static const char *sql = "SELECT player, score FROM scores WHERE month = ? AND year = ?";

    if (!open_database(&db)) {
        md_exit(-1);
    }
    time_t now = time(NULL);
    struct tm now_tm;
#ifdef _MSC_VER
    localtime_s(&now_tm, &now);
#else
    localtime_r(&now, &now_tm);
#endif
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        md_printf("%s\r\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        md_exit(-1);
    }

    int month = now_tm.tm_mon + 1;
    int year = now_tm.tm_year + 1900;
    if (month == 1) {
        month = 12;
        year--;
    } else {
        month--;
    }

    sqlite3_bind_int(stmt, 2, month);
    sqlite3_bind_int(stmt, 3, year);

    std::vector<struct player_s> scores;

    

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        bool found = false;
        for (size_t i = 0; i < scores.size(); i++) {
            if (scores.at(i).player == std::string((const char *)sqlite3_column_text(stmt, 0))) {
                scores.at(i).score += sqlite3_column_int(stmt, 1);
                found = true;
                break;
            }
        }

        if (!found) {
            struct player_s p;
            p.player = std::string((const char *)sqlite3_column_text(stmt, 0));
            p.score = sqlite3_column_int(stmt, 1);

            scores.push_back(p);
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    auto comp = [] (struct player_s a, struct player_s b) {
        return a.score > b.score;
    };
    
    std::sort(scores.begin(), scores.end(), comp);

    for (int i = 0; i < 10 && i < scores.size(); i ++) {
        md_set_cursor(9 + i, 11);
        md_printf("%-25.25s %d", scores.at(i).player.c_str(), scores.at(i).score);
    }
}

void Game::show_scores_today() {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    static const char *sql = "SELECT player, score FROM scores WHERE day = ? AND month = ? AND year = ? ORDER by score DESC LIMIT 5";

    if (!open_database(&db)) {
        md_exit(-1);
    }
    time_t now = time(NULL);
    struct tm now_tm;
#ifdef _MSC_VER
    localtime_s(&now_tm, &now);
#else
    localtime_r(&now, &now_tm);
#endif
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        md_printf("%s\r\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        md_exit(-1);
    }
    sqlite3_bind_int(stmt, 1, now_tm.tm_mday);
    sqlite3_bind_int(stmt, 2, now_tm.tm_mon + 1);
    sqlite3_bind_int(stmt, 3, now_tm.tm_year + 1900);

    int r = 0;

    while(sqlite3_step(stmt) == SQLITE_ROW) {
        md_set_cursor(14 + r, 49);
        r++;
        md_printf("%-22.22s %d", sqlite3_column_text(stmt, 0), sqlite3_column_int(stmt, 1));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return;    
}

void Game::record_score(int score) {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    bool ret = false;
    static const char *sql = "INSERT INTO scores (day, month, year, player, score) VALUES(?, ?, ?, ?, ?)";

    if (!open_database(&db)) {
        md_exit(-1);
    }
    time_t now = time(NULL);
    struct tm now_tm;
#ifdef _MSC_VER
    localtime_s(&now_tm, &now);
#else
    localtime_r(&now, &now_tm);
#endif
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        md_exit(-1);
    }
    sqlite3_bind_int(stmt, 1, now_tm.tm_mday);
    sqlite3_bind_int(stmt, 2, now_tm.tm_mon + 1);
    sqlite3_bind_int(stmt, 3, now_tm.tm_year + 1900);
    sqlite3_bind_text(stmt, 4, mdcontrol.user_alias, -1, NULL);
    sqlite3_bind_int(stmt, 5, score);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return;
}

void Game::play_game() {
    char guess[6];

    int score = 6;

    md_printf("`bright white`Welcome to wordelle!`white`\r\n\r\n");

    md_printf("You have 6 guesses to guess the correct 5 letter word.\r\n");
    md_printf("Green means correct letter correct spot. Yellow means\r\n");
    md_printf("correct letter wrong spot. Grey means letter is not in\r\n");
    md_printf("the word.\r\n\r\n");
    md_printf("Enter \"quit\" to quit and receive a score of 0\r\n\r\n");

    for (int i = 0; i < 6; ) {
        memset(guess, 0, 6);
        md_printf("Guess %d: ", i + 1);
        if (md_getstring(guess, 5, 'A', 'z') < 5) {
            if (strcasecmp(guess, "quit") == 0) {
                score = 0;
                break;
            }            
            md_printf("\r\nGuess too short...\r\n");
            continue;
        }

        for (int l = 0; l < 5; l++) {
            guess[l] = tolower(guess[l]);
        }



        bool ok = false;
        for (size_t c = 0; c < words.size(); c++) {
            if (words.at(c) == guess) {
                ok = true;
                break;
            }
        }

        if (ok == false) {
            md_printf("\r\nThat word is not in my database!\r\n");
            continue;
        }
        md_printf("\r\n");

        int checked[5] = {0,0,0,0,0};
        int answer[5] = {0,0,0,0,0};
        for(int l = 0; l < 5; l++) {
            if (guess[l] == todays_word.at(l)) {
                checked[l] = 2;
                answer[l] = 2;
            }
        }

        for (int l = 0; l < 5; l++) {
            if (todays_word.find(guess[l]) != std::string::npos) {
                for (int w = 0; w < 5; w++) {
                    if (checked[w] == 0 && todays_word.at(w) == guess[l]) {
                        checked[w] = 1;
                        answer[l] = 1;
                        break;
                    }
                }
            }
        }

        for (int l = 0; l < 5; l++) {
            if (answer[l] == 2) {
                md_printf("`bright green`%c", toupper(guess[l]));
            } else if (answer[l] == 1) {
                md_printf("`bright yellow`%c", toupper(guess[l]));
            } else {
                md_printf("`white`%c", toupper(guess[l]));
            }
        }

        md_printf("`white`\r\n");

        if (todays_word == guess) {
            md_printf("You got it!\r\n");
            break;
        }
        score--;
        i++;
    }

    md_printf("Your score: %d/6\r\n\r\n", score);
    record_score(score);

    md_printf("`bright white`Press a key!`white`");
    md_getc();
}

bool Game::load() {
    std::ifstream file;
    file.open("answerlist.txt");

    if (file.is_open()) {
        std::string sa;

        while(getline(file, sa)) {
            answers.push_back(sa);
        }

        file.close();
    } else {
        return false;
    }

    file.open("wordlist.txt");
    if (file.is_open()) {
        std::string sa;

        while(getline(file, sa)) {
            words.push_back(sa);
        }
        file.close();
    } else {
        return false;
    }

    return true;
}

bool Game::check_played() {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    bool ret = false;
    static const char *sql = "SELECT score FROM scores WHERE day = ? AND MONTH = ? AND year = ? AND player = ?";

    if (!open_database(&db)) {
        md_exit(-1);
    }
    time_t now = time(NULL);
    struct tm now_tm;
#ifdef _MSC_VER
    localtime_s(&now_tm, &now);
#else
    localtime_r(&now, &now_tm);
#endif
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        md_exit(-1);
    }
    sqlite3_bind_int(stmt, 1, now_tm.tm_mday);
    sqlite3_bind_int(stmt, 2, now_tm.tm_mon + 1);
    sqlite3_bind_int(stmt, 3, now_tm.tm_year + 1900);
    sqlite3_bind_text(stmt, 4, mdcontrol.user_alias, -1, NULL);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        ret = true;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return ret;
}

void Game::check_new_day() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    static const char *sql = "SELECT word FROM words WHERE day = ? AND month = ? AND year = ?";
    static const char *sql2 = "INSERT INTO words (day, month, year, word) VALUES(?, ?, ?, ?)";
    if (!open_database(&db)) {
        md_exit(-1);
    }
    time_t now = time(NULL);
    struct tm now_tm;
#ifdef _MSC_VER
    localtime_s(&now_tm, &now);
#else
    localtime_r(&now, &now_tm);
#endif

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        md_exit(-1);
    }
    sqlite3_bind_int(stmt, 1, now_tm.tm_mday);
    sqlite3_bind_int(stmt, 2, now_tm.tm_mon + 1);
    sqlite3_bind_int(stmt, 3, now_tm.tm_year + 1900);

    if (sqlite3_step(stmt) == SQLITE_ROW) {

        todays_word = std::string((const char *)sqlite3_column_text(stmt, 0));
        sqlite3_finalize(stmt);
    } else {
        sqlite3_finalize(stmt);
        todays_word = answers.at(rand() % answers.size());

        if (sqlite3_prepare_v2(db, sql2, -1, &stmt, NULL) != SQLITE_OK) {
            sqlite3_close(db);
            md_exit(-1);
        }
        sqlite3_bind_int(stmt, 1, now_tm.tm_mday);
        sqlite3_bind_int(stmt, 2, now_tm.tm_mon + 1);
        sqlite3_bind_int(stmt, 3, now_tm.tm_year + 1900);
        sqlite3_bind_text(stmt, 4, todays_word.c_str(), -1, NULL);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_close(db);
            md_exit(-1);
        }

        sqlite3_finalize(stmt);
    }

    sqlite3_close(db);
}

bool Game::open_database(sqlite3 **db) {
    static const char *sql1 = "CREATE TABLE IF NOT EXISTS words(day INTEGER, month INTEGER, year INTEGER, word TEXT)";
    static const char *sql2 = "CREATE TABLE IF NOT EXISTS scores(day INTEGER, month INTEGER, year INTEGER, player TEXT, score INTEGER)";
    if (sqlite3_open("wordelle.sqlite3", db) != SQLITE_OK) {
        return false;
    }
    sqlite3_busy_timeout(*db, 5000);

    int rc = sqlite3_exec(*db, sql1, 0, 0, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(*db);
        return false;
    }
    rc = sqlite3_exec(*db, sql2, 0, 0, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(*db);
        return false;
    }
    return true;
}
