/*
 * file: $RCSfile$
 * author: Pawel S. Veselov
 * created: 2002/10/06
 * last modified: $Date$
 * version: $Revision$
 */

#include "clines/sysi.h"
#include "clines/main.h"
#include "clines/board.h"
#include "clines/render.h"
#include "clines/play.h"

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
int color_mode = CM_AUTO;
int * chips_colors;
char * hi_score_who = (char*)NULL;
char * my_user = (char*)NULL;
char * command_codes = (char*)"hjkl ";
char * color_font = (char*)"IOo";
char * bw_font = (char*)NULL;
int is_debug = 0;
FILE * debug_file = 0;

#ifdef ALLOW_HI_SCORES
static int saved_hi_score = 0;
static int saved = 0;
int allow_hi_score = 1;
#else
int allow_hi_score = 0;
#endif

static int arg_width = 9;
static int arg_height = 9;
static int arg_cpt = 3;
static int arg_colors = 5;
static int arg_first_cpt = 3;
static int arg_cpl = 5;
static int arg_so = 0;
static char * arg_color_set = NULL;

static void mysig(int);
static void load_hi_score(void);
static void save_hi_score(void);
static void who_am_i(void);
static int parse_options(int, char**);
static void prargs(void);
static int color_set_checksout(void);

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

    if (color_set_checksout()) {
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

    game->board = (int*)fmalloc(game->w*game->h*sizeof(int));
    bzero(game->board, game->w * game->h * sizeof(int));
    game->set = snew(pset);
    game->rec = snew(pset);
    game->path = snew(pset);
    game->rec->len = 0;
    game->rec->path = (int*)fmalloc(sizeof(int) * (game->nev>game->first_nev?
            game->nev:game->first_nev));
    game->set->path = (int*)fmalloc(sizeof(int) * game->w*game->h);
    game->path->path = (int*)fmalloc(sizeof(int) * game->w*game->h);

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

    if (debug_file) {
        fclose(debug_file);
    }
    
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

#ifdef ALLOW_HI_SCORES

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
        full_path = (char*)fmalloc(slen);
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
#endif /* ALLOW_HI_SCORES */
}

void who_am_i() {

    uid_t uid = getuid();
    struct passwd * me = getpwuid(uid);

    my_user = (char*)NULL;

    if (me) {
        my_user = me->pw_name;
    }

    if (strchr(my_user, '/')) {
        // I don't want to allow username with slashes.
        my_user = (char*)NULL;
    }
}

void save_hi_score() {

#ifdef ALLOW_HI_SCORES
    
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

    full_path = (char*)fmalloc(path_len);

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

#endif /* ALLOW_HI_SCORES */

}

void prargs() {

    fprintf(stdout,
"clines [-hlv] [-w<N>] [-t<N>] [-c<N>] [-i<N>] [-C<S>]\n"
"       [-f<S>] [-F<S>] [-m{c|b}] [-n<N>] [-N<N>] [-o<C>]\n"
"\n"
"  -h           See this help and exit\n"
#ifdef ALLOW_HI_SCORES
"  -l           List high scores and exit\n"
#endif /* ALLOW_HI_SCORES */
"  -v           Print version number and exit\n"
"  -w<width>    Specify alternative width (default is 9)\n"
"  -t<height>   Specify alternative height (default is 9)\n"
"  -c<colors>   Specify how many colors to use (default is 5)\n"
"  -i<limit>    Specify how long a line is popped (default is 5)\n"
"  -C<controls> Specify characters to expect for controls, in the order of\n"
"               left down up right action. Arrow keys will always work\n"
"               default is \"hjkl \".\n"
"  -f<chars>    Specify characters to use to draw chips and cursor in color\n"
"               mode. Three characters accepted, first is used to draw cursor\n"
"               second to draw the chips, and the third is to draw selected\n"
"               chip in mini mode. By default this is \"IOo\"\n"
"  -F<chars>    Specify characters to use to draw chips and cursor in B&W\n"
"               mode. First character is always for the cursor then as many\n"
"               characters as there are chip colors for the drawing chips\n"
"               and then again for drawing jumped chips in mini mode\n"
"               By default it's \"*OVA.Zova'z\"\n"
"  -n<number>   How many chips appear after every move (default is 3)\n"
"  -N<number>   How many chips appear the first time (default is 3)\n"
"  -o<colors>   Specify comma separated color numbers to use for the chips.\n"
"               By default it's 7,1,2,3,4,5. The first color is always a\n"
"               cursor color. The rest is for the chips. If the amount of\n"
"               different chips is changed, then, by default, the colors are\n"
"               assigned in the same manner, white (7) assigned for cursor\n"
"               and then colors starting from 1 are assigned for chips\n"
"               if the assigned color is white (7), it's skipped over.\n"
"  -m{c|b}      Force color or black and white mode.\n"
"\n"
"If board is customized, hi scores will not be loaded or saved\n");
}

int parse_options(int argc, char ** argv) {

    int c;

    while ( (c = getopt(argc, argv, "hlvw:t:c:i:C:f:F:m:n:N:o:D")) != -1) {

        switch (c) {

            case 'h':   // help
                prargs();
                exit(0);
#ifdef ALLOW_HI_SCORES
            case 'l':   // list scores
                arg_so = 1;
                allow_hi_score = 1;
                return 0;
#endif /* ALLOW_HI_SCORES */
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
                    if (optarg && (aux = atoi(optarg)) >= 0 &&
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
                if (!optarg) {
                    fprintf(stdout, "Black & white font not specified\n");
                    return 1;
                }
                bw_font = strdup(optarg);
                break;
            case 'm':   // lock mode
                if (!optarg) {
                    fprintf(stdout, "No color mode specified for -m\n");
                    return 1;
                }

                switch (*optarg) {
                    case 'c':
                    case 'C':
                        color_mode = CM_COLOR;
                        break;
                    case 'b':
                    case 'B':
                        color_mode = CM_BW;
                        break;
                    default:
                        fprintf(stdout, "Unknown color mode %s\n", optarg);
                        return 1;
                }
                break;
            case 'o':
                if (!optarg) {
                    fprintf(stdout, "No colors specified for -b\n");
                    return 1;
                }
                arg_color_set = strdup(optarg);
                break;
            case 'D':
                debug_file = fopen("clines.debug", "w");
                if (!debug_file) {
                    perror("create clines.debug");
                    return 1;
                }
                is_debug = 1;
                break;
            default:
                return 1;
        }
    }
    return 0;
}

int color_set_checksout() {

    chips_colors = (int*)fmalloc(sizeof(int) * (arg_colors+1));

    if (!arg_color_set) {
        int i;
        int color = 1;
        
        chips_colors[0] = COLOR_WHITE;
        for (i=1; i<arg_colors+1; i++, color++) {
            if (color == COLOR_WHITE) { color++; }
            chips_colors[i] = color;
        }

        return 0;

    } else {

        int idx = 0;
        char * ptr = arg_color_set;
        char * aptr;
        char * nptr;
        int had_dups = 0;
        char * aux = strdup(arg_color_set);
        int i;

        while (1) {

            nptr = strchr(ptr, ',');

            if (nptr) { *nptr = 0; }

            while (*ptr && (*ptr < '0'||*ptr > '9')) {
                ptr++;
            }

            if (!*ptr) {
                break;
            }

            chips_colors[idx] = strtol(ptr, &aptr, 10);

            if (ptr == nptr) {
                fprintf(stdout, "Failed to parse colors in %s\n", aux);
                free(aux);
                return 1;
            }

            if (chips_colors[idx] == COLOR_BLACK) {
                fprintf(stdout, "Can't use black color for chips : "
                        "(parsing %s)\n", aux);
                free(aux);
                return 1;
            }

            for (i=0; i<idx; i++) {
                if (chips_colors[idx] == chips_colors[i]) {
                    fprintf(stdout, "Duplicate color %d, in positions "
                            "%d and %d (parsing %s)\n",
                            chips_colors[idx], idx+1, i+1, aux);
                    had_dups = 1;
                    break;
                }
            }

            idx++;

            if (idx == arg_colors + 1) {
                break;
            }

            if (!nptr) { break; }

            ptr = nptr + 1;
        }

        if (idx < arg_colors + 1) {
            fprintf(stdout, "Not enough colors given (%d), "
                    "need %d (parsing %s)\n",
                    idx, arg_colors + 1, aux);
            free(aux);
            return 1;
        }

        if (had_dups) {
            char ara[20];
            fprintf(stdout, "Press ENTER to continue, Ctrl-C to stop...");
            fgets(ara, 15, stdin);
        }

        free(aux);
        return 0;

    }
}

void * frealloc(void * ptr, size_t sz) {
    ptr = realloc(ptr, sz);
    if (!ptr) {
        fprintf(stderr, "out of memory");
        _exit(2);
    }
    return ptr;
}

void * fmalloc(size_t sz) {

    void * r = malloc(sz);
    if (!r) {
        fprintf(stderr, "out of memory");
        _exit(2);
    }

    return r;

}
