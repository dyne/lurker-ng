/* A Bison parser, made by GNU Bison 1.875a.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     tAGO = 258,
     tDST = 259,
     tDAY = 260,
     tDAY_UNIT = 261,
     tDAYZONE = 262,
     tHOUR_UNIT = 263,
     tLOCAL_ZONE = 264,
     tMERIDIAN = 265,
     tMINUTE_UNIT = 266,
     tMONTH = 267,
     tMONTH_UNIT = 268,
     tSEC_UNIT = 269,
     tYEAR_UNIT = 270,
     tZONE = 271,
     tSNUMBER = 272,
     tUNUMBER = 273
   };
#endif
#define tAGO 258
#define tDST 259
#define tDAY 260
#define tDAY_UNIT 261
#define tDAYZONE 262
#define tHOUR_UNIT 263
#define tLOCAL_ZONE 264
#define tMERIDIAN 265
#define tMINUTE_UNIT 266
#define tMONTH 267
#define tMONTH_UNIT 268
#define tSEC_UNIT 269
#define tYEAR_UNIT 270
#define tZONE 271
#define tSNUMBER 272
#define tUNUMBER 273




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 154 "getdate.y"
typedef union YYSTYPE {
  int intval;
  textint textintval;
} YYSTYPE;
/* Line 1240 of yacc.c.  */
#line 78 "y.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





