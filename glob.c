/* glob.c: rc's (ugly) globber. This code is not elegant, but it works */

#include "rc.h"
#include "stat.h"

/* Lifted from autoconf documentation.*/
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

static List *dmatch(char *, char *, char *);
static List *doglob(char *, char *);
static List *lglob(List *, char *, char *, size_t);
static List *sort(List *);

/*
   Matches a list of words s against a list of patterns p. Returns true iff
   a pattern in p matches a word in s. () matches (), but otherwise null
   patterns match nothing.
*/

extern bool lmatch(List *s, List *p) {
	List *q;
	if (s == NULL) {
		if (p == NULL) /* null matches null */
			return TRUE;
		for (; p != NULL; p = p->n) /* one or more stars match null */
			if (strspn(p->w, "*") == strlen(p->w) &&
			    p->m != NULL && strlen(p->m) == strlen(p->w))
				return TRUE;
		return FALSE;
	}
	for (; s != NULL; s = s->n)
		for (q = p; q != NULL; q = q->n)
			if (match(q->w, q->m, s->w))
				return TRUE;
	return FALSE;
}

/*
   Globs a list; checks to see if each element in the list has a metacharacter. If it
   does, it is globbed, and the output is sorted.
*/

extern List *glob(List *s) {
	List *top, *r;
	bool meta;
	for (r = s, meta = FALSE; r != NULL; r = r->n)
		if (r->m != NULL)
			meta = TRUE;
	if (!meta)
		return s; /* don't copy lists with no metacharacters in them */
	for (top = r = NULL; s != NULL; s = s->n) {
		if (s->m == NULL) { /* no metacharacters; just tack on to the return list */
			if (top == NULL)
				top = r = nnew(List);
			else
				r = r->n = nnew(List);
			r->w = s->w;
		} else {
			if (top == NULL)
				top = r = sort(doglob(s->w, s->m));
			else
				r->n = sort(doglob(s->w, s->m));
			while (r->n != NULL)
				r = r->n;
		}
	}
	r->n = NULL;
	return top;
}

/* Matches a pattern p against the contents of directory d */

static List *dmatch(char *d, char *p, char *m) {
	bool matched;
	List *top, *r;
	static DIR *dirp;
	static struct dirent *dp;
	static struct stat s;
	int i;

	/*
 	   return a match if there are no metacharacters; allows globbing through
	   directories with no read permission. make sure the file exists, though.
	 */
	matched = TRUE;
	if (m != NULL)
		for (i = 0; p[i] != '\0'; i++)
			if (m[i]) {
				matched = FALSE;
				break;
			}

	if (matched) {
		char *path = nprint("%s/%s", d, p);
		if (lstat(path, &s) < 0)
			return NULL;
		r = nnew(List);
		r->w = ncpy(p);
		r->m = NULL;
		r->n = NULL;
		return r;
	}

	top = r = NULL;
	if (*d == '\0') d = "/";
	if ((dirp = opendir(d)) == NULL)
		return NULL;
	/* opendir succeeds on regular files on some systems, so the stat() call is necessary (sigh) */
	if (stat(d, &s) < 0 || (s.st_mode & S_IFMT) != S_IFDIR) {
		closedir(dirp);
		return NULL;
	}
	while ((dp = readdir(dirp)) != NULL)
		if ((*dp->d_name != '.' || *p == '.') && match(p, m, dp->d_name)) { /* match ^. explicitly */
			matched = TRUE;
			if (top == NULL)
				top = r = nnew(List);
			else
				r = r->n = nnew(List);
			r->w = ncpy(dp->d_name);
			r->m = NULL;
		}
	closedir(dirp);
	if (!matched)
		return NULL;
	r->n = NULL;
	return top;
}

/*
   lglob() globs a pattern against a list of directory roots. e.g., (/tmp /usr /bin) "*"
   will return a list with all the files in /tmp, /usr, and /bin. NULL on no match.
   slashcount indicates the number of slashes to stick between the directory and the
   matched name. e.g., for matching ////tmp/////foo*
*/

static List *lglob(List *s, char *p, char *m, size_t slashcount) {
	List *q, *r, *top, foo;
	static struct {
		List l;
		size_t size;
	} slash;
	if (slashcount+1 > slash.size) {
		slash.size = 2*(slashcount+1);
		slash.l.w = erealloc(slash.l.w, slash.size);
	}
	slash.l.w[slashcount] = '\0';
	while (slashcount > 0)
		slash.l.w[--slashcount] = '/';
	for (top = r = NULL; s != NULL; s = s->n) {
		q = dmatch(s->w, p, m);
		if (q != NULL) {
			foo.w = s->w;
			foo.m = NULL;
			foo.n = NULL;
			if (!(s->w[0] == '/' && s->w[1] == '\0')) /* need to separate */
				q = concat(&slash.l, q);	  /* dir/name with slash */
			q = concat(&foo, q);
			if (r == NULL)
				top = r = q;
			else
				r->n = q;
			while (r->n != NULL)
				r = r->n;
		}
	}
	return top;
}

/*
   Doglob globs a pathname in pattern form against a unix path. Returns the original
   pattern (cleaned of metacharacters) on failure, or the globbed string(s).
*/

static List *doglob(char *w, char *m) {
	static char *dir = NULL, *pattern = NULL, *metadir = NULL, *metapattern = NULL;
	static size_t dsize = 0;
	char *d, *p, *md, *mp;
	size_t psize;
	char *s = w;
	List firstdir;
	List *matched;
	if ((psize = strlen(w) + 1) > dsize || dir == NULL) {
		efree(dir); efree(pattern); efree(metadir); efree(metapattern);
		dir = ealloc(psize);
		pattern = ealloc(psize);
		metadir = ealloc(psize);
		metapattern = ealloc(psize);
		dsize = psize;
	}
	d = dir;
	p = pattern;
	md = metadir;
	mp = metapattern;
	while (*s != '/' && *s != '\0') {
		*d++ = *s++; /* get first directory component */
		*md++ = *m++;
	}
	*d = '\0';
	/*
	   Special case: no slashes in the pattern, i.e., open the current directory.
	   Remember that w cannot consist of slashes alone (the other way *s could be
	   zero) since doglob gets called iff there's a metacharacter to be matched
	*/
	if (*s == '\0') {
		matched = dmatch(".", dir, metadir);
		goto end;
	}
	if (*w == '/') {
		firstdir.w = dir;
		firstdir.m = metadir;
		firstdir.n = NULL;
		matched = &firstdir;
	} else {
		/*
		   we must glob against current directory,
		   since the first character is not a slash.
		*/
		matched = dmatch(".", dir, metadir);
	}
	do {
		size_t slashcount;
		sigchk();
		for (slashcount = 0; *s == '/'; s++, m++)
			slashcount++; /* skip slashes */
		while (*s != '/' && *s != '\0')
			*p++ = *s++, *mp++ = *m++; /* get pattern */
		*p = '\0';
		matched = lglob(matched, pattern, metapattern, slashcount);
		p = pattern, mp = metapattern;
	} while (*s != '\0');
end:	if (matched == NULL) {
		matched = nnew(List);
		matched->w = w;
		matched->m = NULL;
		matched->n = NULL;
	}
	return matched;
}

static List *sort(List *s) {
	size_t nel = listnel(s);
	if (nel > 1) {
		char **a;
		List *t;
		qsort(a = list2array(s, FALSE), nel, sizeof(char *), starstrcmp);
		for (t = s; t != NULL; t = t->n)
			t->w = *a++;
	}
	return s;
}
