#ifndef BYTE_H
#define BYTE_H

extern unsigned int byte_chr();
extern unsigned int byte_rchr();
extern void byte_copy(char* to, unsigned int n, const char* from);
extern void byte_copyr();
extern int byte_diff();
extern void byte_zero(char* s, unsigned int n);

#define byte_equal(s,n,t) (!byte_diff((s),(n),(t)))

#endif
