#include "rc.h"

#include <errno.h>
#include <stdio.h>

#include "linenoise/linenoise.h"
#include "edit.h"

bool editing = 1;

struct cookie {
	char *buffer;
};

void *edit_begin(int fd) {
	struct cookie *c;

	c = ealloc(sizeof *c);
	c->buffer = NULL;
	return c;
}

static void edit_catcher(int sig) {
	write(2, "\n", 1);
	rc_raise(eError);
}

static char *prompt;

char *edit_alloc(void *cookie, size_t *count) {
	struct cookie *c = cookie;
	void (*oldint)(int), (*oldquit)(int);

	oldint = sys_signal(SIGINT, edit_catcher);
	oldquit = sys_signal(SIGQUIT, edit_catcher);

	c->buffer = linenoise(prompt);

	sys_signal(SIGINT, oldint);
	sys_signal(SIGQUIT, oldquit);

	if (c->buffer) {
		*count = strlen(c->buffer);
		if (*count)
			linenoiseHistoryAdd(c->buffer);
		c->buffer[*count] = '\n';
		*count += 1;
	}

	return c->buffer;
}

void edit_prompt(void *cookie, char *pr) {
	prompt = pr;
}

void edit_free(void *cookie) {
	struct cookie *c = cookie;

	efree(c->buffer);
	/* Set c->buffer to NULL, allowing us to "overfree" it.  This
	   is a bit of a kludge, but it's otherwise hard to deal with
	   the case where a signal causes an early return from
	   linenoise. */
	c->buffer = NULL;
}

void edit_end(void *cookie) {
	struct cookie *c = cookie;

	efree(c);
}

void edit_reset(void *cookie) {
	linenoiseClearScreen();
}
