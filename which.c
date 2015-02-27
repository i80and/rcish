/* which.c: check to see if a file is executable.

   This function was originally written with Maarten Litmaath's which.c as
   a template, but was changed in order to accommodate the possibility of
   rc's running setuid or the possibility of executing files not in the
   primary group. Much of this file has been re-vamped by Paul Haahr.
   I re-re-vamped the functions that Paul supplied to correct minor bugs
   and to strip out unneeded functionality.
*/

#include "rc.h"

#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>

#include "getgroups.h"

#define X_USR 0100
#define X_GRP 0010
#define X_OTH 0001
#define X_ALL (X_USR|X_GRP|X_OTH)

static bool initialized = FALSE;
static uid_t uid;
static gid_t gid;

#if HAVE_GETGROUPS
static int ngroups;
static GETGROUPS_T *gidset;

/* determine whether gid lies in gidset */

static int ingidset(gid_t g) {
	int i;
	for (i = 0; i < ngroups; ++i)
		if (g == gidset[i])
			return 1;
	return 0;
}
#else
#define ingidset(g) (FALSE)
#endif

/*
   A home-grown access/stat. Does the right thing for group-executable files.
   Returns a bool instead of this -1 nonsense.
*/

static bool rc_access(char *path, bool verbose) {
	struct stat st;
	int mask;
	if (stat(path, &st) != 0) {
		if (verbose) /* verbose flag only set for absolute pathname */
			uerror(path);
		return FALSE;
	}
	if (uid == 0)
		mask = X_ALL;
	else if (uid == st.st_uid)
		mask = X_USR;
	else if (gid == st.st_gid || ingidset(st.st_gid))
		mask = X_GRP;
	else
		mask = X_OTH;
	if (((st.st_mode & S_IFMT) == S_IFREG) && (st.st_mode & mask))
		return TRUE;
	errno = EACCES;
	if (verbose)
		uerror(path);
	return FALSE;
}

/* replace non-printing characters with question marks in a freshly
 * allocated string */
static char *protect(char *in) {
	int l = strlen(in);
	char *out = ealloc(l + 1);
	int i;

	for (i = 0; i < l; ++i)
		out[i] = isprint(in[i]) ? in[i] : '?';
	out[i] = '\0';
	return out;
}
		    
/* return a full pathname by searching $path, and by checking the status of the file */

extern char *which(char *name, bool verbose) {
	static char *test = NULL;
	static size_t testlen = 0;
	List *path;
	int len;
	if (name == NULL)	/* no filename? can happen with "> foo" as a command */
		return NULL;
	if (!initialized) {
		initialized = TRUE;
		uid = geteuid();
		gid = getegid();
#if HAVE_GETGROUPS
#if HAVE_POSIX_GETGROUPS
		ngroups = getgroups(0, (GETGROUPS_T *)0);
		if (ngroups < 0) {
			uerror("getgroups");
			rc_exit(1);
		}
#else
		ngroups = NGROUPS;
#endif
		if (ngroups) {	
			gidset = ealloc(ngroups * sizeof(GETGROUPS_T));
			getgroups(ngroups, gidset);
		}
#endif
	}
	if (isabsolute(name)) /* absolute pathname? */
		return rc_access(name, verbose) ? name : NULL;
	len = strlen(name);
	for (path = varlookup("path"); path != NULL; path = path->n) {
		size_t need = strlen(path->w) + len + 2; /* one for null terminator, one for the '/' */
		if (testlen < need) {
			efree(test);
			test = ealloc(testlen = need);
		}
		if (*path->w == '\0') {
			strcpy(test, name);
		} else {
			strcpy(test, path->w);
			if (!streq(test, "/")) /* "//" is special to POSIX */
				strcat(test, "/");
			strcat(test, name);
		}
		if (rc_access(test, FALSE))
			return test;
	}
	if (verbose) {
		char *n = protect(name);
		fprint(2, RC "cannot find `%s'\n", n);
		efree(n);
	}
	return NULL;
}
