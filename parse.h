/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ANDAND = 258,
    BACKBACK = 259,
    BANG = 260,
    CASE = 261,
    COUNT = 262,
    DUP = 263,
    ELSE = 264,
    END = 265,
    FLAT = 266,
    FN = 267,
    FOR = 268,
    IF = 269,
    IN = 270,
    OROR = 271,
    PIPE = 272,
    REDIR = 273,
    SREDIR = 274,
    SUB = 275,
    SUBSHELL = 276,
    SWITCH = 277,
    TWIDDLE = 278,
    WHILE = 279,
    WORD = 280,
    HUH = 281,
    TRY = 282
  };
#endif
/* Tokens.  */
#define ANDAND 258
#define BACKBACK 259
#define BANG 260
#define CASE 261
#define COUNT 262
#define DUP 263
#define ELSE 264
#define END 265
#define FLAT 266
#define FN 267
#define FOR 268
#define IF 269
#define IN 270
#define OROR 271
#define PIPE 272
#define REDIR 273
#define SREDIR 274
#define SUB 275
#define SUBSHELL 276
#define SWITCH 277
#define TWIDDLE 278
#define WHILE 279
#define WORD 280
#define HUH 281
#define TRY 282

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 29 "./parse.y" /* yacc.c:1909  */

	struct Node *node;
	struct Redir redir;
	struct Pipe pipe;
	struct Dup dup;
	struct Word word;
	char *keyword;

#line 117 "y.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
