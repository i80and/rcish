/* status.c: functions for printing fancy status messages in rc */

#include "rc.h"
#include "sigmsgs.h"
#include "statval.h"
#include "wait.h"

/* status == the wait() value of the last command in the pipeline, or the last command */

static int statuses[512];
static int pipelength = 1;

/*
   Test to see if rc's status is true. According to td, status is true
   if and only if every pipe-member has an exit status of zero.
*/

extern int istrue() {
	int i;
	for (i = 0; i < pipelength; i++)
		if (statuses[i] != 0)
			return FALSE;
	return TRUE;
}

/*
   Return the status as an integer. A status which has low-bits set is
   a signal number, whereas a status with high bits set is a value set
   from exit(). The presence of a signal just sets status to 1. Also,
   a pipeline with nonzero exit statuses in it just sets status to 1.
*/

extern int getstatus() {
	int s;
	if (pipelength > 1)
		return !istrue();
	s = statuses[0];
	if (WIFSIGNALED(s))
		return 1;
	return WEXITSTATUS(s);
}

extern void set(bool code) {
	setstatus(-1, code ? STATUS0 : STATUS1);
}

/* take a pipeline and store the exit statuses. Check to see whether any of the children dumped core */

extern void setpipestatus(int stats[], int num) {
	int i;
	for (i = 0; i < (pipelength = num); i++) {
		statprint(-1, stats[i]);
		statuses[i] = stats[i];
	}
}

/* set a simple status, as opposed to a pipeline */

extern void setstatus(pid_t pid, int i) {
	pipelength = 1;
	statuses[0] = i;
	statprint(pid, i);
}

/* print a message if termination was with a signal, and if the child dumped core. exit on error if -e is set */

extern void statprint(pid_t pid, int i) {
	if (WIFSIGNALED(i)) {
		int t = WTERMSIG(i);
		char *msg = ((t > 0) && (t < NUMOFSIGNALS) ? signals[WTERMSIG(i)].msg : "");
		if (pid != -1)
			fprint(2, "%ld: ", (long)pid);
		if (myWIFDUMPED(i)) {
			if (*msg == '\0')
				fprint(2, "core dumped\n");
			else
				fprint(2, "%s--core dumped\n", msg);
		} else if (*msg != '\0')
			fprint(2, "%s\n", msg);
	}
	if (i != 0 && dashee && !cond)
		rc_exit(getstatus());
}

/* prepare a list to be passed back. Used whenever $status is dereferenced */

extern List *sgetstatus() {
	List *r = NULL;
	int i;

	for (i = 0; i < pipelength; i++) {
		List *q = nnew(List);
		q->w = strstatus(statuses[i]);
		q->m = NULL;
		q->n = r;
		r = q;
	}

	return r;
}

/* return status as a string (used above and for bqstatus) */

extern char *strstatus(int s) {
	if (WIFSIGNALED(s)) {
		int t = WTERMSIG(s);
		const char *core = myWIFDUMPED(s) ? "+core" : "";
		if ((t > 0) && (t < NUMOFSIGNALS) && *signals[t].name != '\0')
			return nprint("%s%s", signals[t].name, core);
		else
			return nprint("-%d%s", t, core); /* unknown signals are negated */
	} else
		return nprint("%d", WEXITSTATUS(s));
}

extern void ssetstatus(char **av) {
	int i, j, k, l;
	bool found;
	for (l = 0; av[l] != NULL; l++)
		; /* count up array length */
	--l;
	for (i = 0; av[i] != NULL; i++) {
		j = a2u(av[i]);
		if (j >= 0) {
			statuses[l - i] = j << 8;
			continue;
		}
		found = FALSE;
		for (k = 0; k < NUMOFSIGNALS; k++) {
			if (streq(signals[k].name, av[i])) {
				statuses[l - i] = k;
				found = TRUE;
				break;
			} 
			else {
				size_t len = strlen(signals[k].name);
				if (strncmp(signals[k].name, av[i], len) == 0 && streq(av[i] + len, "+core")) {
					statuses[l - i] = k + 0x80;
					found = TRUE;
					break;
				}
			}
		}
		if (!found) {
			fprint(2, "bad status\n");
			set(FALSE);
			return;
		}
	}
	pipelength = i;
}
