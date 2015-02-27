The `rcish` Shell
================

An evolution, not a revolution.

Additional Features
===================

  * Lexical block scoping
    * New scopes are introduced within `{ }` blocks. Nesting works as expected.
  * Local variables
    * Destroyed at the end of their encosing scope.
    * Very antiuseful outside of functions.
    * *Not* exported.
  * `try` blocks
    * If any command returns a non-zero status code outside of a condition,
      the block immediately exits with that status code.
  * Function copying
    * `fn copy = orig` creates a new function `copy` with the exact contents of
      `orig`. Useful for extending the functionality of already-defined functions.
  * Integrated Linenoise REPL
  * Fixed to compile with musl libc

Original
========

See COPYING for copying information.  All files are derived from those belonging to

   Copyright 1991, 2001, 2002, 2003, 2014 Byron Rakitzis.

More information on the original Unix port of rc can be found at this web page.

    http://tobold.org/article/rc
