#ifndef DEBUG_H
#define DEBUG_H

extern int debug_level;
extern int debug_syslog;   /* use syslog? */
extern char *debug_file;

void debug_init();
void debug_close();

void debug(int level, char *format, ...);
void debug_perror(int level, char *format, ...);
void debug_assert(int test,  char *msg, ...);

#endif
