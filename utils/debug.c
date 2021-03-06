#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <syslog.h>

#include <libmicro/debug.h>

int debug_time = 0;
int debug_level  = 0;
int debug_syslog = 0;
char *debug_file = NULL;
FILE *debugFP;

void print_time()
{
        if (debug_time == 0)
                return;

        time_t t = time(NULL);
        char *tbuf = ctime(&t);
        tbuf[strlen(tbuf) - 1] = 0;
        fprintf(debugFP, "%s - ", tbuf);
}

void debug_init()
{
        if (debug_file) {
                if ((debugFP = fopen(debug_file, "a")) == NULL) {
                        printf("Failed to open Debuglogfile\n");
                        exit(EXIT_FAILURE);
                }
                debug_time = 1;
        } else if (debug_syslog) {
                openlog("cand", LOG_CONS | LOG_PID | LOG_NDELAY, 0);

        } else
                debugFP = stderr;
}

void debug_close()
{
        if (debugFP != NULL) {
                fflush(debugFP);
                fclose(debugFP);
        }
        if (debug_syslog)
                closelog();
}

void debug(int level, char *format, ...)
{
        va_list ap;

        if (debug_level < level) {
                return;
        }

        print_time();

        va_start(ap, format);
        vfprintf(debugFP, format, ap);
        fprintf(debugFP, "\n");
        fflush(debugFP);
        va_end(ap);
}

void debug_perror(int level, char *format, ...)
{
        va_list ap;

        if (debug_level < level)
                return;

        print_time();

        //debug
        //fprintf(debugFP, "1: %i\n", level);
        //fprintf(debugFP, "2: %c, %s\n", format, format);

        va_start(ap, format);
        vfprintf(debugFP, format, ap);
        fprintf(debugFP, " (%s)\n", strerror(errno));
        fflush(debugFP);
        va_end(ap);
}

void debug_assert(int test, char *format, ...)
{
        if (test)
                return;

        va_list ap;

        print_time();

        va_start(ap, format);
        fprintf(debugFP, "ERROR: debug_assert(..) failed: ");
        vfprintf(debugFP, format, ap);
        fprintf(debugFP, " (%s)\n", strerror(errno));
        va_end(ap);

        debug_close();
        exit(EXIT_FAILURE);
}

