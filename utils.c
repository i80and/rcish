/* utils.c: functions of general utility */

#include "rc.h"

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <setjmp.h>

#include "jbwrap.h"

/* print error with line number on noninteractive shells (i.e., scripts) */

extern void pr_error(char *s, int offset) {
	if (s != NULL) {
		if (interactive)
			fprint(2, RC "%s\n", s);
		else
			fprint(2, RC "line %d: %s\n", lineno + offset, s);
	}
}

/* our perror */

extern void uerror(const char *s) {
	char *err;

	err = strerror(errno);
	if (!err) err = "unknown error";

	if (s)
		fprint(2, RC "%s: %s\n", s, err);
	else
		fprint(2, RC "%s\n", err);
}

/* Die horribly. This should never get called. Please let me know if it does. */

#define PANICMSG "rc panic: "

extern void panic(char *s) {
	write(2, PANICMSG, conststrlen(PANICMSG));
	write(2, s, strlen(s));
	write(2, "!\n", 2);
	exit(1);
}

/* ascii -> unsigned conversion routines. -1 indicates conversion error. */

extern int n2u(char *s, unsigned int base) {
	unsigned int i;
	for (i = 0; *s != '\0'; s++) {
		unsigned int j = (unsigned int) *s - '0';
		if (j >= base) /* small hack with unsigned ints -- one compare for range test */
			return -1;
		i = i * base + j;
	}
	return (int) i;
}

/* The last word in portable ANSI: a strcmp wrapper for qsort */

extern int starstrcmp(const void *s1, const void *s2) {
	return strcmp(*(char * const *)s1, *(char * const *)s2);
}

/* tests to see if pathname begins with "/", "./", or "../" */

extern bool isabsolute(char *path) {
	return path[0] == '/' || (path[0] == '.' && (path[1] == '/' || (path[1] == '.' && path[2] == '/')));
}


/* duplicate a fd and close the old one only if necessary */

extern int mvfd(int i, int j) {
	if (i != j) {
		int s = dup2(i, j);
		close(i);
		return s;
	}
	return 0;
}

// From OpenBSD
#define MUL_NO_OVERFLOW (1UL << (sizeof(size_t) * 4))
extern void* reallocarray(void* optr, size_t nmemb, size_t size) {
	if((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) &&
	   nmemb > 0 && SIZE_MAX / nmemb < size) {
		errno = ENOMEM;
		return NULL;
	}

	return realloc(optr, size * nmemb);
}

static char* varbuf = NULL;
static size_t varbufLen = 0;
extern char* getLocalName(scope_t scope, const char* rawname) {
	if(varbuf == NULL) {
		varbufLen = 128;
		varbuf = calloc(varbufLen, sizeof(char));
	}

	while(TRUE) {
		size_t written = snprintf(varbuf, varbufLen, LOCAL_PREFIX "%lu_%s", scope, rawname);
		if(written >= varbufLen) {
			char* newvarbuf = reallocarray(varbuf, varbufLen*2, sizeof(char));
			if(newvarbuf == NULL) {
				panic("out of memory");
			}
			varbuf = newvarbuf;
		} else {
			return varbuf;
		}
	}
}
