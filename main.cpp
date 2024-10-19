extern "C" {
#include "magidoor/MagiDoor.h"
}
#include "Game.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>

int main(int argc, char **argv) {
    int socket = -1;
    char *dropfile;
    if (argc > 2) {
        socket = atoi(argv[2]);
        dropfile = argv[1];
    } else if (argc > 1) {
        dropfile = argv[1];
    } else {
        printf("Usage: wordelle DROPFILE [SOCKET]\n");
        exit(-1);
    }

    srand(time(NULL));
    Game g;

    if (!g.load()) {
        printf("Error loading wordlists\n");
        exit(-1);
    }

    md_init(dropfile, socket);
    g.check_new_day();
    
    md_clr_scr();
    md_sendfile("wordelle.ans", 0);

    g.show_scores_last_month();
    g.score_this_month();
    g.show_scores_today();

    if (g.check_played()) {
        md_set_cursor(21, 10);
        md_printf("Sorry, you've already played today! Come back tomorrow!");
        md_getc();
    } else {
        md_set_cursor(21, 10);
        md_printf("Press SPACEBAR to play! Any other key to quit...");
        if (md_getc() == ' ') {
            md_clr_scr();
            g.play_game();
        }
    }
    md_exit(0);
}