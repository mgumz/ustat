#ifndef STR_H
#define STR_H

extern unsigned int str_copy();
extern int str_diff();
extern int str_diffn(const char* s, const char *t, unsigned int len);
extern unsigned int str_len(const char* s);
extern unsigned int str_chr();
extern unsigned int str_rchr();
extern int str_start(const char* s, const char* t);

#define str_equal(s,t) (!str_diff((s),(t)))

#endif
