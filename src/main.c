/*
 * file: $RCSfile$
 * author: Pawel S. Veselov
 * created: 2002/10/06
 * last modified: $Date$
 * version: $Revision$
 */

#include <clines/sysi.h>
#include <clines/main.h>
#include <clines/board.h>
#include <clines/render.h>
#include <clines/play.h>

#ifndef CLINES_DATA_DIR
#define CLINES_DATA_DIR "/var/tmp"
#endif

#define CLINES_MAGIC 0xc001ceed
#define CLINES_SCORE_VERSION 1

#ifndef PACKAGE_STRING
#define PACKAGE_STRING "clines 1.0.4 (not verified)"
#endif

int score = 0;
int hi_score = 0;
int my_hi_score = 0;
char * hi_score_who = (char*)NULL;
char * my_user = (char*)NULL;
int allow_hi_score = 1;
char * command_codes = "hjkl ";
char * color_font = "IOo";

static int saved_hi_score = 0;
static int saved = 0;

static int arg_width = 9;
static int arg_height = 9;
static int arg_cpt = 3;
static int arg_colors = 5;
static int arg_first_cpt = 3;
static int arg_cpl = 5;
static char * arf_bwode;
static int arg_so = 0;

static void mysig(int);
static void load_hi_score(void);
static void save_hi_score(void);
static void who_am_i(void);
static int parse_options(int, char**);
static void prargs(void);

static struct itimerval timer = {
    { 0, 200000 },
    { 0, 200000 } 
};

static struct itimerval stoptimer = {
    { 0, 0 },
    { 0, 0 }
};

int main(int argc, char ** argv) {

    board * game;

    if (parse_options(argc, argv)) {
        exit(1);
    }

    who_am_i();

    load_hi_score();

    if (arg_so) {
        exit(0);
    }

#ifndef NO_SAVE_TTY
    savetty();
#endif

    signal(SIGQUIT, mysig);
    signal(SIGINT, mysig);
    signal(SIGALRM, mysig);
    signal(SIGWINCH, mysig);
    resume_timer();

    game = snew(board);

    game->w = arg_width;
    game->h = arg_height;
    game->nev = arg_cpt;
    game->mc = arg_colors;
    game->ml = arg_cpl;
    game->first_nev = arg_first_cpt;
    game->played = 0;

    srand(time(NULL));

    game->board = (int*)malloc(game->w*game->h*sizeof(int));
    bzero(game->board, game->w * game->h * sizeof(int));
    game->set = snew(pset);
    game->rec = snew(pset);
    game->path = snew(pset);
    game->rec->len = 0;
    game->rec->path = (int*)malloc(sizeof(int) * (game->nev>game->first_nev?
            game->nev:game->first_nev));
    game->set->path = (int*)malloc(sizeof(int) * game->w*game->h);
    game->path->path = (int*)malloc(sizeof(int) * game->w*game->h);

    reset(game);
    rinit(game, 1);
    rborder(game);
    rscore(game);

    score = 0;
    play(game);

    getch();

    quit();

    return 0;
}

void mysig(int s) {

    if (s == SIGALRM) {
	if (c_board && c_board->sel >= 0) {
            c_board->jst++;
            render1(c_board, c_board->sel, -1);
            refresh();
            doupdate();
	}
	signal(s, mysig);
	return;
    }

    if (s == SIGWINCH) {
        if (c_board) {
            rinit(c_board, 0);
        }
        signal(s, mysig);
        return;
    }
    
    quit();
}

void fatal(char * fmt, ...) {

    va_list argp;

    rfini();

    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);

    exit(1);
}

void quit() {

    rfini();

    save_hi_score();
    
    fflush(stdout);
    fprintf(stdout, "Scores earned : %d\n", score);
    fflush(stdout);
    exit(0);
}

void resume_timer() {
    if (!timer.it_value.tv_usec) {
	timer.it_value.tv_usec++;
    }
    setitimer(ITIMER_REAL, &timer, NULL);
}

void suspend_timer() {
    setitimer(ITIMER_REAL, &stoptimer, NULL);
}

void load_hi_score() {

    DIR * dir;
    struct dirent * next;
    int min_len;
    char * full_path;
    int cdd_len;
    int aux, fd, slen;
    int namlen;

    if (!allow_hi_score) {
        return;
    }

    dir = opendir(CLINES_DATA_DIR);
    min_len = strlen(CSCORE_EXT) + 1;
    cdd_len = strlen(CLINES_DATA_DIR);

    if (!dir) { return; }

    while (1) {

        char * whos_hi_score;

        next = readdir(dir);

        if (!next) { break; }

        if (!next->d_name || (namlen = strlen(next->d_name)) < min_len ||
                strncmp(CSCORE_EXT,
                    next->d_name + namlen - min_len + 1, min_len)) {
            continue;
        }

        slen = namlen + cdd_len + 2;
        full_path = (char*)malloc(slen);
        if (!full_path) { break; } // really sad that is
        bzero(full_path, slen);
        snprintf(full_path, slen, "%s/%s", CLINES_DATA_DIR, next->d_name);

        fd = open(full_path, O_RDONLY);

        if (fd < 0) {
            free(full_path);
            continue;
        }

        if (read(fd, &aux, sizeof(aux)) != sizeof(aux) ||
                ntohl(aux) != CLINES_MAGIC ||
                read(fd, &aux, sizeof(aux)) != sizeof(aux) ||
                ntohl(aux) != CLINES_SCORE_VERSION ||
                read(fd, &aux, sizeof(aux)) != sizeof(aux)) {

            free(full_path);
            close(fd);
            continue;
        }

        close(fd);

        aux = ntohl(aux);

        whos_hi_score = strdup(next->d_name);
        *(whos_hi_score + namlen - min_len + 1) = 0;
        if (strlen(whos_hi_score) > 8) {
            whos_hi_score[8] = 0;
        }

        if (arg_so) {
            int c = ' ';
            if (my_user && !strcmp(whos_hi_score, my_user)) {
                c = '*';
            }
            fprintf(stdout, "%c %-8s\t%d\n", c, whos_hi_score, aux);
            free(whos_hi_score);
            continue;
        }

        if (hi_score < aux) {
            hi_score = aux;
            if (hi_score_who) {
                free(hi_score_who);
            }
            hi_score_who = strdup(whos_hi_score);
        }

        if (my_user && !strcmp(whos_hi_score, my_user)) {
            saved_hi_score = my_hi_score = aux;
        }

        free(full_path);
        free(whos_hi_score);
    }

    closedir(dir);

}

void who_am_i() {

    uid_t uid = getuid();
    struct passwd * me = getpwuid(uid);

    my_user = (char*)NULL;

    if (me) {
        my_user = me->pw_name;
    }
    
}

void save_hi_score() {

    int path_len;
    char * full_path;
    int fd;
    int aux;

    if (!allow_hi_score || saved || score <= saved_hi_score) {
        return;
    }
    
    saved = 1;

    if (!my_user) {
        who_am_i(); // try again
        if (!my_user) {
            fprintf(stdout, "Can't save scores : can't find your uid !\n");
            perror("getpwuid()");
            return;
        }
    }

    path_len = strlen(CLINES_DATA_DIR) + 2 +
        strlen(my_user) + strlen(CSCORE_EXT);

    full_path = (char*)malloc(path_len);

    if (!full_path) {
        // you think fprintf will work ? :)
        fprintf(stdout, "Can't save your scores : out of memory\n");
        return;
    }

    snprintf(full_path, path_len, "%s/%s%s",
            CLINES_DATA_DIR, my_user, CSCORE_EXT);

    // ok now...
    
    unlink(full_path);  // just in case
    fd = open(full_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        fprintf(stdout, "Can't create score file %s\n", full_path);
        perror("open");
        free(full_path);
        return;
    }

    aux = htonl(CLINES_MAGIC);
    write(fd, &aux, sizeof(aux));
    aux = htonl(CLINES_SCORE_VERSION);    // version, kinda
    write(fd, &aux, sizeof(aux));
    aux = htonl(score);
    write(fd, &aux, sizeof(aux));

    fchmod(fd, 0644);   // ignore the umask

    close(fd);

    if (score > hi_score) {
        fprintf(stdout, "New record saved!\n");
    } else {
        fprintf(stdout, "New personal record saved!\n");
    }
}

void prargs() {

    fprintf(stdout,
"clines [-hlv] [-w<N>] [-t<N>] [-c<N>] [-i<N>] [-C<S>]\n"
"       [-f<S>] [-F<S>] [-m{c|b}] [-n<N>] [-N<N>]\n"
"\n"
"  -h           see this help and exit\n"
"  -l           list high scores and exit\n"
"  -v           print version number and exit\n"
"  -w<width>    specify alternative width (default is 9)\n"
"  -t<height>   specify alternative height (default is 9)\n"
"  -c<colors>   specify how many colors to use (default is 5)\n"
"  -i<limit>    specify how long a line is popped (default is 5)\n"
"  -C<controls> specify characters to expect for controls, in the order of\n"
"               left down up right action. Arrow keys will always work\n"
"               default is \"hjkl \".\n"
"  -f<chars>    specify characters to use to draw chips and cursor in color\n"
"               mode. Three characters accepted, first is used to draw cursor\n"
"               second to draw the chips, and the third is to draw selected\n"
"               chip in mini mode. By default this is \"IOo\"\n"
"  -F<chars>    specify characters to use to draw chips and cursor in B&W\n"
"               mode. First character is always for the cursor then as many\n"
"               characters as there are chip colors for the drawing chips\n"
"               and then again for drawing jumped chips in mini mode\n"
"               By default it's \"*OVA.Zova'z\"\n"
"  -n<number>   How many chips appear after every move (default is 3)\n"
"  -N<number>   How many chips appear the first time (default is 3)\n"
"\n"
"If board is customized, hi scores will not be loaded or saved\n");

}

int parse_options(int argc, char ** argv) {

    int c;

    while ( (c = getopt(argc, argv, "hlvw:t:c:i:C:f:F:m:n:N:")) != -1) {

        switch (c) {

            case 'h':   // help
                prargs();
                exit(0);
            case 'l':   // list scores
                arg_so = 1;
                allow_hi_score = 1;
                return 0;
            case 'v':   // version
                fprintf(stdout, "%s\n", PACKAGE_STRING);
                exit(0);
            case 'w':   // width
                {
                    int aux = 0;
                    if (optarg && (aux = atoi(optarg)) > 0 &&
                            arg_width != aux) {
                        arg_width = aux;
                        allow_hi_score = 0;
                    }
                }
                break;
            case 't':   // height
                {
                    int aux = 0;
                    if (optarg && (aux = atoi(optarg)) > 0 &&
                            arg_height != aux) {
                        arg_height = aux;
                        allow_hi_score = 0;
                    }
                }
                break;
            case 'c':   // colors
                {
                    int aux = 0;
                    if (optarg && (aux = atoi(optarg)) > 0 &&
                            arg_colors != aux) {
                        arg_colors = aux;
                        allow_hi_score = 0;
                    }
                }
                break;
            case 'i':   // pop limit
                {
                    int aux = 0;
                    if (optarg && (aux = atoi(optarg)) > 1 &&
                            arg_cpl != aux) {
                        arg_cpl = aux;
                        allow_hi_score = 0;
                    }
                }
                break;
            case 'n':   // new chips
                {
                    int aux = 0;
                    if (optarg && (aux = atoi(optarg)) > 0 &&
                            arg_cpt != aux) {
                        arg_cpt = aux;
                        allow_hi_score = 0;
                    }
                }
                break;
            case 'N':   // new chips at startup
                {
                    int aux = 0;
                    if (optarg && (aux = atoi(optarg)) > 0 &&
                            arg_first_cpt != aux) {
                        arg_first_cpt = aux;
                        allow_hi_score = 0;
                    }
                }
                break;
            case 'C':   // controls
                if (!optarg || strlen(optarg) != 5) {
                    fprintf(stdout, "Expected 5 characters only for -C\n");
                    return 1;
                }
                command_codes = strdup(optarg);
                break;
            case 'f':   // color font
                if (!optarg || strlen(optarg) != 3) {
                    fprintf(stdout, "Expected 3 characters only for -f\n");
                    return 1;
                }
                color_font = strdup(optarg);
                break;
            case 'F':   // b&w font
            case 'm':   // lock mode
            default:
                return 1;

        }

    }
    return 0;
    
}
