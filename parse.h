#ifndef YYERRCODE
#define YYERRCODE 256
#endif

#define ANDAND 257
#define BACKBACK 258
#define BANG 259
#define CASE 260
#define COUNT 261
#define DUP 262
#define ELSE 263
#define END 264
#define FLAT 265
#define FN 266
#define FOR 267
#define IF 268
#define IN 269
#define OROR 270
#define PIPE 271
#define REDIR 272
#define SREDIR 273
#define SUB 274
#define SUBSHELL 275
#define SWITCH 276
#define TWIDDLE 277
#define WHILE 278
#define WORD 279
#define HUH 280
typedef union {
	struct Node *node;
	struct Redir redir;
	struct Pipe pipe;
	struct Dup dup;
	struct Word word;
	char *keyword;
} YYSTYPE;
extern YYSTYPE yylval;
