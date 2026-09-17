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
     sySkip = 258,
     syRoutine = 259,
     sySimpleRoutine = 260,
     sySubsystem = 261,
     syKernelUser = 262,
     syKernelServer = 263,
     syMsgOption = 264,
     syMsgSeqno = 265,
     syWaitTime = 266,
     syNoWaitTime = 267,
     syErrorProc = 268,
     syServerPrefix = 269,
     syUserPrefix = 270,
     syServerDemux = 271,
     syRCSId = 272,
     syImport = 273,
     syUImport = 274,
     sySImport = 275,
     syIImport = 276,
     syDImport = 277,
     syIn = 278,
     syOut = 279,
     syInOut = 280,
     syUserImpl = 281,
     syServerImpl = 282,
     syRequestPort = 283,
     syReplyPort = 284,
     sySReplyPort = 285,
     syUReplyPort = 286,
     syType = 287,
     syArray = 288,
     syStruct = 289,
     syOf = 290,
     syInTran = 291,
     syOutTran = 292,
     syDestructor = 293,
     syCType = 294,
     syCUserType = 295,
     syUserTypeLimit = 296,
     syOnStackLimit = 297,
     syCServerType = 298,
     syPointerTo = 299,
     syPointerToIfNot = 300,
     syValueOf = 301,
     syCString = 302,
     syUserSecToken = 303,
     syServerSecToken = 304,
     syColon = 305,
     sySemi = 306,
     syComma = 307,
     syPlus = 308,
     syMinus = 309,
     syStar = 310,
     syDiv = 311,
     syLParen = 312,
     syRParen = 313,
     syEqual = 314,
     syCaret = 315,
     syTilde = 316,
     syLAngle = 317,
     syRAngle = 318,
     syLBrack = 319,
     syRBrack = 320,
     syBar = 321,
     syError = 322,
     syNumber = 323,
     sySymbolicType = 324,
     syIdentifier = 325,
     syString = 326,
     syQString = 327,
     syFileName = 328,
     syIPCFlag = 329
   };
#endif
#define sySkip 258
#define syRoutine 259
#define sySimpleRoutine 260
#define sySubsystem 261
#define syKernelUser 262
#define syKernelServer 263
#define syMsgOption 264
#define syMsgSeqno 265
#define syWaitTime 266
#define syNoWaitTime 267
#define syErrorProc 268
#define syServerPrefix 269
#define syUserPrefix 270
#define syServerDemux 271
#define syRCSId 272
#define syImport 273
#define syUImport 274
#define sySImport 275
#define syIImport 276
#define syDImport 277
#define syIn 278
#define syOut 279
#define syInOut 280
#define syUserImpl 281
#define syServerImpl 282
#define syRequestPort 283
#define syReplyPort 284
#define sySReplyPort 285
#define syUReplyPort 286
#define syType 287
#define syArray 288
#define syStruct 289
#define syOf 290
#define syInTran 291
#define syOutTran 292
#define syDestructor 293
#define syCType 294
#define syCUserType 295
#define syUserTypeLimit 296
#define syOnStackLimit 297
#define syCServerType 298
#define syPointerTo 299
#define syPointerToIfNot 300
#define syValueOf 301
#define syCString 302
#define syUserSecToken 303
#define syServerSecToken 304
#define syColon 305
#define sySemi 306
#define syComma 307
#define syPlus 308
#define syMinus 309
#define syStar 310
#define syDiv 311
#define syLParen 312
#define syRParen 313
#define syEqual 314
#define syCaret 315
#define syTilde 316
#define syLAngle 317
#define syRAngle 318
#define syLBrack 319
#define syRBrack 320
#define syBar 321
#define syError 322
#define syNumber 323
#define sySymbolicType 324
#define syIdentifier 325
#define syString 326
#define syQString 327
#define syFileName 328
#define syIPCFlag 329




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 202 "parser.y"
typedef union YYSTYPE {
    u_int number;
    identifier_t identifier;
    string_t string;
    statement_kind_t statement_kind;
    ipc_type_t *type;
    struct
    {
	u_int innumber;		/* msgt_name value, when sending */
	string_t instr;
	u_int outnumber;	/* msgt_name value, when receiving */
	string_t outstr;
	u_int size;		/* 0 means there is no default size */
    } symtype;
    routine_t *routine;
    arg_kind_t direction;
    argument_t *argument;
    ipc_flags_t flag;
} YYSTYPE;
/* Line 1240 of yacc.c.  */
#line 205 "parser.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



