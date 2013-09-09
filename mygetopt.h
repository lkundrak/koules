
#ifndef _MYGETOPT_H
#define _MYGETOPT_H 1


extern char    *myoptarg;


extern int      myoptind;


extern int      myopterr;


extern int      myoptopt;
extern int      mygetopt (int argc, char **argv, char *shortopts);

#endif
