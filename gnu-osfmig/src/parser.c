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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
#line 187 "parser.y"


#include "lexxer.h"
#include "strdefs.h"
#include "type.h"
#include "routine.h"
#include "statement.h"
#include "global.h"
#include "error.h"

static char *import_name();



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

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
/* Line 191 of yacc.c.  */
#line 258 "parser.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 270 "parser.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   259

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  50
/* YYNRULES -- Number of rules. */
#define YYNRULES  122
/* YYNRULES -- Number of states. */
#define YYNSTATES  241

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   329

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    13,    16,    19,    22,
      25,    28,    31,    34,    37,    40,    43,    46,    49,    51,
      54,    59,    61,    62,    65,    67,    69,    71,    73,    77,
      80,    83,    87,    89,    92,    95,    98,   101,   105,   107,
     109,   111,   113,   115,   119,   122,   126,   128,   137,   146,
     154,   159,   164,   169,   171,   173,   176,   179,   182,   185,
     187,   189,   194,   201,   206,   208,   215,   217,   219,   221,
     225,   227,   232,   238,   246,   252,   258,   263,   270,   272,
     275,   279,   283,   287,   291,   293,   297,   299,   301,   305,
     309,   312,   316,   318,   320,   324,   328,   333,   337,   343,
     344,   346,   348,   350,   352,   354,   356,   358,   360,   362,
     364,   366,   368,   370,   372,   375,   378,   381,   382,   386,
     392,   393,   394
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      76,     0,    -1,    -1,    76,    77,    -1,    78,    51,    -1,
      87,    51,    -1,    84,    51,    -1,    85,    51,    -1,    86,
      51,    -1,    88,    51,    -1,    89,    51,    -1,    90,    51,
      -1,    91,    51,    -1,    95,    51,    -1,   110,    51,    -1,
       3,    51,    -1,    92,    51,    -1,    94,    51,    -1,    51,
      -1,     1,    51,    -1,    79,    80,    82,    83,    -1,     6,
      -1,    -1,    80,    81,    -1,     7,    -1,     8,    -1,    70,
      -1,    68,    -1,   122,     9,    71,    -1,    41,    68,    -1,
      42,    68,    -1,   122,    11,    71,    -1,    12,    -1,    13,
      70,    -1,    14,    70,    -1,    15,    70,    -1,    16,    70,
      -1,   123,    93,    73,    -1,    18,    -1,    19,    -1,    20,
      -1,    21,    -1,    22,    -1,   124,    17,    72,    -1,    32,
      96,    -1,    70,    59,    97,    -1,    98,    -1,    97,    36,
      50,    70,    70,    57,    70,    58,    -1,    97,    37,    50,
      70,    70,    57,    70,    58,    -1,    97,    38,    50,    70,
      57,    70,    58,    -1,    97,    39,    50,    70,    -1,    97,
      40,    50,    70,    -1,    97,    43,    50,    70,    -1,   100,
      -1,   103,    -1,   104,    98,    -1,   105,    98,    -1,    60,
      98,    -1,   106,    98,    -1,   107,    -1,    99,    -1,    44,
      57,   108,    58,    -1,    45,    57,   108,    52,   108,    58,
      -1,    46,    57,   108,    58,    -1,   102,    -1,    57,   102,
      52,   109,   121,    58,    -1,    68,    -1,    69,    -1,   101,
      -1,   101,    66,   101,    -1,    70,    -1,    33,    64,    65,
      35,    -1,    33,    64,    55,    65,    35,    -1,    33,    64,
      55,    50,   109,    65,    35,    -1,    33,    64,   109,    65,
      35,    -1,    34,    64,   109,    65,    35,    -1,    47,    64,
     109,    65,    -1,    47,    64,    55,    50,   109,    65,    -1,
      70,    -1,   108,    70,    -1,   109,    53,   109,    -1,   109,
      54,   109,    -1,   109,    55,   109,    -1,   109,    56,   109,
      -1,    68,    -1,    57,   109,    58,    -1,   111,    -1,   112,
      -1,     4,    70,   113,    -1,     5,    70,   113,    -1,    57,
      58,    -1,    57,   114,    58,    -1,   115,    -1,   116,    -1,
     115,    51,   114,    -1,   116,    51,   114,    -1,   117,    70,
     120,   121,    -1,   119,    70,   120,    -1,   118,    70,   120,
      52,    70,    -1,    -1,    23,    -1,    24,    -1,    25,    -1,
      28,    -1,    29,    -1,    30,    -1,    31,    -1,    11,    -1,
       9,    -1,    27,    -1,    26,    -1,    49,    -1,    48,    -1,
      10,    -1,    50,    70,    -1,    50,    96,    -1,    50,    99,
      -1,    -1,   121,    52,    74,    -1,   121,    52,    74,    64,
      65,    -1,    -1,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   224,   224,   225,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   248,   250,   251,   252,   253,
     257,   270,   282,   283,   286,   297,   305,   308,   311,   328,
     331,   335,   341,   349,   357,   365,   373,   381,   392,   393,
     394,   395,   396,   399,   409,   419,   423,   425,   445,   465,
     480,   494,   503,   515,   517,   519,   521,   523,   525,   527,
     529,   533,   535,   538,   542,   548,   555,   561,   565,   567,
     591,   595,   597,   599,   604,   608,   612,   614,   619,   621,
     625,   627,   629,   631,   633,   635,   640,   641,   644,   648,
     652,   654,   659,   661,   663,   668,   675,   696,   703,   714,
     715,   716,   717,   718,   719,   720,   721,   722,   723,   726,
     727,   730,   731,   732,   735,   741,   743,   748,   749,   756,
     765,   769,   773
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "sySkip", "syRoutine", "sySimpleRoutine", 
  "sySubsystem", "syKernelUser", "syKernelServer", "syMsgOption", 
  "syMsgSeqno", "syWaitTime", "syNoWaitTime", "syErrorProc", 
  "syServerPrefix", "syUserPrefix", "syServerDemux", "syRCSId", 
  "syImport", "syUImport", "sySImport", "syIImport", "syDImport", "syIn", 
  "syOut", "syInOut", "syUserImpl", "syServerImpl", "syRequestPort", 
  "syReplyPort", "sySReplyPort", "syUReplyPort", "syType", "syArray", 
  "syStruct", "syOf", "syInTran", "syOutTran", "syDestructor", "syCType", 
  "syCUserType", "syUserTypeLimit", "syOnStackLimit", "syCServerType", 
  "syPointerTo", "syPointerToIfNot", "syValueOf", "syCString", 
  "syUserSecToken", "syServerSecToken", "syColon", "sySemi", "syComma", 
  "syPlus", "syMinus", "syStar", "syDiv", "syLParen", "syRParen", 
  "syEqual", "syCaret", "syTilde", "syLAngle", "syRAngle", "syLBrack", 
  "syRBrack", "syBar", "syError", "syNumber", "sySymbolicType", 
  "syIdentifier", "syString", "syQString", "syFileName", "syIPCFlag", 
  "$accept", "Statements", "Statement", "Subsystem", "SubsystemStart", 
  "SubsystemMods", "SubsystemMod", "SubsystemName", "SubsystemBase", 
  "MsgOption", "UserTypeLimit", "OnStackLimit", "WaitTime", "Error", 
  "ServerPrefix", "UserPrefix", "ServerDemux", "Import", "ImportIndicant", 
  "RCSDecl", "TypeDecl", "NamedTypeSpec", "TransTypeSpec", "TypeSpec", 
  "NativeTypeSpec", "BasicTypeSpec", "PrimIPCType", "IPCType", 
  "PrevTypeSpec", "VarArrayHead", "ArrayHead", "StructHead", 
  "CStringSpec", "TypePhrase", "IntExp", "RoutineDecl", "Routine", 
  "SimpleRoutine", "Arguments", "ArgumentList", "Argument", "Trailer", 
  "Direction", "TrImplKeyword", "TrExplKeyword", "ArgumentType", 
  "IPCFlags", "LookString", "LookFileName", "LookQString", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    75,    76,    76,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      78,    79,    80,    80,    81,    81,    82,    83,    84,    85,
      86,    87,    87,    88,    89,    90,    91,    92,    93,    93,
      93,    93,    93,    94,    95,    96,    97,    97,    97,    97,
      97,    97,    97,    98,    98,    98,    98,    98,    98,    98,
      98,    99,    99,    99,   100,   100,   101,   101,   102,   102,
     103,   104,   104,   104,   105,   106,   107,   107,   108,   108,
     109,   109,   109,   109,   109,   109,   110,   110,   111,   112,
     113,   113,   114,   114,   114,   114,   115,   116,   116,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   118,
     118,   119,   119,   119,   120,   120,   120,   121,   121,   121,
     122,   123,   124
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     2,
       4,     1,     0,     2,     1,     1,     1,     1,     3,     2,
       2,     3,     1,     2,     2,     2,     2,     3,     1,     1,
       1,     1,     1,     3,     2,     3,     1,     8,     8,     7,
       4,     4,     4,     1,     1,     2,     2,     2,     2,     1,
       1,     4,     6,     4,     1,     6,     1,     1,     1,     3,
       1,     4,     5,     7,     5,     5,     4,     6,     1,     2,
       3,     3,     3,     3,     1,     3,     1,     1,     3,     3,
       2,     3,     1,     1,     3,     3,     4,     3,     5,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     0,     3,     5,
       0,     0,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,    21,    32,     0,
       0,     0,     0,     0,     0,     0,    18,     3,     0,    22,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    86,    87,     0,     0,     0,    19,    15,     0,
       0,    33,    34,    35,    36,     0,    44,    29,    30,     4,
       0,     6,     7,     8,     5,     9,    10,    11,    12,    16,
      17,    13,    14,     0,     0,    38,    39,    40,    41,    42,
       0,     0,    99,    88,    89,     0,    24,    25,    26,    23,
       0,    28,    31,    37,    43,   108,   113,   107,   100,   101,
     102,   110,   109,   103,   104,   105,   106,   112,   111,    90,
       0,    92,    93,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    66,    67,    70,    45,    46,    60,
      53,    68,    64,    54,     0,     0,     0,    59,    27,    20,
      91,    99,    99,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    57,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    58,    94,    95,     0,   117,     0,    97,
       0,     0,     0,    84,     0,     0,    78,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    69,
     114,   115,   116,    96,     0,     0,     0,     0,    71,     0,
       0,     0,     0,     0,     0,    61,    79,     0,    63,     0,
      76,   117,     0,     0,     0,    50,    51,    52,     0,    98,
       0,    72,    85,    80,    81,    82,    83,    74,    75,     0,
       0,     0,     0,     0,     0,   118,     0,    62,    77,    65,
       0,     0,     0,     0,    73,     0,     0,    49,   119,    47,
      48
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     1,    17,    18,    19,    50,    79,    80,   129,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    70,    29,
      30,    46,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   167,   164,    31,    32,    33,    73,   100,
     101,   102,   103,   104,   105,   157,   183,    34,    35,    36
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -137
static const short yypact[] =
{
    -137,    27,  -137,   -32,   -26,   -58,   -35,  -137,  -137,     7,
      29,    39,    42,    43,   -15,    58,  -137,  -137,    60,  -137,
      93,    95,    96,    98,   106,   107,   111,   112,   119,   126,
     137,   138,  -137,  -137,     6,   161,   125,  -137,  -137,   114,
     114,  -137,  -137,  -137,  -137,   132,  -137,  -137,  -137,  -137,
      -7,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,
    -137,  -137,  -137,    79,   121,  -137,  -137,  -137,  -137,  -137,
     117,   122,    56,  -137,  -137,    91,  -137,  -137,  -137,  -137,
     127,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,
    -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,
     135,   145,   146,   128,   130,   133,   140,   141,   142,   144,
     149,   143,   -17,    91,  -137,  -137,  -137,   129,  -137,  -137,
    -137,   136,  -137,  -137,    91,    91,    91,  -137,  -137,  -137,
    -137,    92,    92,   158,   158,   158,   -44,    88,   139,   139,
     139,   -39,   159,  -137,   160,   162,   163,   164,   165,   166,
     -17,  -137,  -137,  -137,  -137,  -137,   -36,  -137,   167,  -137,
     -43,    88,   182,  -137,    35,    41,  -137,     0,   -47,     4,
     168,    74,    88,   150,   151,   152,   153,   154,   155,  -137,
     132,  -137,  -137,   174,   157,    88,   193,   120,  -137,    88,
      88,    88,    88,   194,   195,  -137,  -137,   139,  -137,    88,
    -137,   131,   169,   170,   175,  -137,  -137,  -137,   171,  -137,
      78,  -137,  -137,    20,    20,  -137,  -137,  -137,  -137,    40,
      99,   -38,   176,   177,   172,   173,   196,  -137,  -137,  -137,
     178,   179,   180,   181,  -137,   183,   185,  -137,  -137,  -137,
    -137
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,
    -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,
    -137,    80,  -137,   -53,    94,  -137,    85,   147,  -137,  -137,
    -137,  -137,  -137,  -136,  -135,  -137,  -137,  -137,   204,   -24,
    -137,  -137,  -137,  -137,  -137,   -42,    46,  -137,  -137,  -137
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -123
static const short yytable[] =
{
      76,    77,   165,   168,   169,   197,   171,   185,   108,   109,
     110,   160,    39,   161,   208,    63,   170,    64,   161,    37,
     229,   162,   186,   196,   163,    38,   187,     2,     3,   163,
       4,     5,     6,     7,   180,    40,  -120,   201,  -120,     8,
       9,    10,    11,    12,  -122,  -121,  -121,  -121,  -121,  -121,
     210,   114,   115,    47,   213,   214,   215,   216,   195,    13,
     143,   219,   198,    78,   220,    85,    86,    87,    14,    15,
     196,   151,   152,   153,   196,   191,   192,    41,    16,    88,
      89,    90,    91,    92,    93,    94,    95,    96,   189,   190,
     191,   192,   158,   159,   189,   190,   191,   192,   227,    42,
     193,    85,    86,    87,    97,    98,   194,   154,   155,    43,
     196,    49,    44,    45,    99,    88,    89,    90,    91,    92,
      93,    94,    95,    96,   106,   107,    48,   189,   190,   191,
     192,   189,   190,   191,   192,   108,   109,   110,   111,   200,
      97,    98,    71,   226,    51,   161,    52,    53,   112,    54,
      81,   113,   189,   190,   191,   192,   163,    55,    56,   114,
     115,   116,    57,    58,   228,   144,   145,   146,   147,   148,
      59,    72,   149,   189,   190,   191,   192,    60,   212,    65,
      66,    67,    68,    69,   189,   190,   191,   192,    61,    62,
      83,    75,    82,   130,    84,   128,   131,   132,   133,   138,
     134,   139,   150,   135,   136,   137,   140,   141,   156,   166,
     173,   172,   174,   175,   176,   177,   178,   188,   199,   184,
     202,   203,   204,   205,   206,   207,   208,   209,   211,   217,
     218,   234,   224,   230,   231,   179,   181,   233,   237,   222,
     223,   239,   232,   240,    74,   225,   238,   221,   235,   236,
     182,     0,     0,     0,     0,     0,     0,     0,     0,   142
};

static const short yycheck[] =
{
       7,     8,   137,   139,   140,    52,   141,    50,    44,    45,
      46,    55,    70,    57,    52,     9,    55,    11,    57,    51,
      58,    65,    65,    70,    68,    51,   161,     0,     1,    68,
       3,     4,     5,     6,    70,    70,     9,   172,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
     185,    68,    69,    68,   189,   190,   191,   192,    58,    32,
     113,   197,    58,    70,   199,     9,    10,    11,    41,    42,
      70,   124,   125,   126,    70,    55,    56,    70,    51,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    53,    54,
      55,    56,   134,   135,    53,    54,    55,    56,    58,    70,
      65,     9,    10,    11,    48,    49,    65,   131,   132,    70,
      70,    51,    70,    70,    58,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    33,    34,    68,    53,    54,    55,
      56,    53,    54,    55,    56,    44,    45,    46,    47,    65,
      48,    49,    17,    65,    51,    57,    51,    51,    57,    51,
      71,    60,    53,    54,    55,    56,    68,    51,    51,    68,
      69,    70,    51,    51,    65,    36,    37,    38,    39,    40,
      51,    57,    43,    53,    54,    55,    56,    51,    58,    18,
      19,    20,    21,    22,    53,    54,    55,    56,    51,    51,
      73,    59,    71,    58,    72,    68,    51,    51,    70,    57,
      70,    57,    66,    70,    64,    64,    57,    64,    50,    70,
      50,    52,    50,    50,    50,    50,    50,    35,    50,    52,
      70,    70,    70,    70,    70,    70,    52,    70,    35,    35,
      35,    35,    57,    57,    57,   150,   156,    64,    58,    70,
      70,    58,    70,    58,    40,    74,    65,   201,    70,    70,
     156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    76,     0,     1,     3,     4,     5,     6,    12,    13,
      14,    15,    16,    32,    41,    42,    51,    77,    78,    79,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    94,
      95,   110,   111,   112,   122,   123,   124,    51,    51,    70,
      70,    70,    70,    70,    70,    70,    96,    68,    68,    51,
      80,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      51,    51,    51,     9,    11,    18,    19,    20,    21,    22,
      93,    17,    57,   113,   113,    59,     7,     8,    70,    81,
      82,    71,    71,    73,    72,     9,    10,    11,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    48,    49,    58,
     114,   115,   116,   117,   118,   119,    33,    34,    44,    45,
      46,    47,    57,    60,    68,    69,    70,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,    68,    83,
      58,    51,    51,    70,    70,    70,    64,    64,    57,    57,
      57,    64,   102,    98,    36,    37,    38,    39,    40,    43,
      66,    98,    98,    98,   114,   114,    50,   120,   120,   120,
      55,    57,    65,    68,   109,   109,    70,   108,   108,   108,
      55,   109,    52,    50,    50,    50,    50,    50,    50,   101,
      70,    96,    99,   121,    52,    50,    65,   109,    35,    53,
      54,    55,    56,    65,    65,    58,    70,    52,    58,    50,
      65,   109,    70,    70,    70,    70,    70,    70,    52,    70,
     109,    35,    58,   109,   109,   109,   109,    35,    35,   108,
     109,   121,    70,    70,    57,    74,    65,    58,    65,    58,
      57,    57,    70,    64,    35,    70,    70,    58,    65,    58,
      58
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 14:
#line 239 "parser.y"
    {
    register statement_t *st = stAlloc();

    st->stKind = skRoutine;
    st->stRoutine = yyvsp[-1].routine;
    rtCheckRoutine(yyvsp[-1].routine);
    if (BeVerbose)
	rtPrintRoutine(yyvsp[-1].routine);
;}
    break;

  case 15:
#line 249 "parser.y"
    { rtSkip(); ;}
    break;

  case 19:
#line 254 "parser.y"
    { yyerrok; ;}
    break;

  case 20:
#line 259 "parser.y"
    {
    if (BeVerbose)
    {
	printf("Subsystem %s: base = %u%s%s\n\n",
	       SubsystemName, SubsystemBase,
	       IsKernelUser ? ", KernelUser" : "",
	       IsKernelServer ? ", KernelServer" : "");
    }
;}
    break;

  case 21:
#line 271 "parser.y"
    {
    if (SubsystemName != strNULL)
    {
	warn("previous Subsystem decl (of %s) will be ignored", SubsystemName);
	IsKernelUser = FALSE;
	IsKernelServer = FALSE;
	strfree(SubsystemName);
    }
;}
    break;

  case 24:
#line 287 "parser.y"
    {
    if (IsKernelUser)
	warn("duplicate KernelUser keyword");
    if (!UseMsgRPC) 
    {
	warn("with KernelUser the -R option is meaningless");
	UseMsgRPC = TRUE;
    }
    IsKernelUser = TRUE;
;}
    break;

  case 25:
#line 298 "parser.y"
    {
    if (IsKernelServer)
	warn("duplicate KernelServer keyword");
    IsKernelServer = TRUE;
;}
    break;

  case 26:
#line 305 "parser.y"
    { SubsystemName = yyvsp[0].identifier; ;}
    break;

  case 27:
#line 308 "parser.y"
    { SubsystemBase = yyvsp[0].number; ;}
    break;

  case 28:
#line 312 "parser.y"
    {
    if (streql(yyvsp[0].string, "MACH_MSG_OPTION_NONE"))
    {
	MsgOption = strNULL;
	if (BeVerbose)
	    printf("MsgOption: canceled\n\n");
    }
    else
    {
	MsgOption = yyvsp[0].string;
	if (BeVerbose)
	    printf("MsgOption %s\n\n",yyvsp[0].string);
    }
;}
    break;

  case 29:
#line 329 "parser.y"
    {UserTypeLimit = yyvsp[0].number; ;}
    break;

  case 30:
#line 332 "parser.y"
    {MaxMessSizeOnStack = yyvsp[0].number; ;}
    break;

  case 31:
#line 336 "parser.y"
    {
    WaitTime = yyvsp[0].string;
    if (BeVerbose)
	printf("WaitTime %s\n\n", WaitTime);
;}
    break;

  case 32:
#line 342 "parser.y"
    {
    WaitTime = strNULL;
    if (BeVerbose)
	printf("NoWaitTime\n\n");
;}
    break;

  case 33:
#line 350 "parser.y"
    {
    ErrorProc = yyvsp[0].identifier;
    if (BeVerbose)
	printf("ErrorProc %s\n\n", ErrorProc);
;}
    break;

  case 34:
#line 358 "parser.y"
    {
    ServerPrefix = yyvsp[0].identifier;
    if (BeVerbose)
	printf("ServerPrefix %s\n\n", ServerPrefix);
;}
    break;

  case 35:
#line 366 "parser.y"
    {
    UserPrefix = yyvsp[0].identifier;
    if (BeVerbose)
	printf("UserPrefix %s\n\n", UserPrefix);
;}
    break;

  case 36:
#line 374 "parser.y"
    {
    ServerDemux = yyvsp[0].identifier;
    if (BeVerbose)
	printf("ServerDemux %s\n\n", ServerDemux);
;}
    break;

  case 37:
#line 382 "parser.y"
    {
    register statement_t *st = stAlloc();
    st->stKind = yyvsp[-1].statement_kind;
    st->stFileName = yyvsp[0].string;

    if (BeVerbose)
	printf("%s %s\n\n", import_name(yyvsp[-1].statement_kind), yyvsp[0].string);
;}
    break;

  case 38:
#line 392 "parser.y"
    { yyval.statement_kind = skImport; ;}
    break;

  case 39:
#line 393 "parser.y"
    { yyval.statement_kind = skUImport; ;}
    break;

  case 40:
#line 394 "parser.y"
    { yyval.statement_kind = skSImport; ;}
    break;

  case 41:
#line 395 "parser.y"
    { yyval.statement_kind = skIImport; ;}
    break;

  case 42:
#line 396 "parser.y"
    { yyval.statement_kind = skDImport; ;}
    break;

  case 43:
#line 400 "parser.y"
    {
    if (RCSId != strNULL)
	warn("previous RCS decl will be ignored");
    if (BeVerbose)
	printf("RCSId %s\n\n", yyvsp[0].string);
    RCSId = yyvsp[0].string;
;}
    break;

  case 44:
#line 410 "parser.y"
    {
    register identifier_t name = yyvsp[0].type->itName;

    if (itLookUp(name) != itNULL)
	warn("overriding previous definition of %s", name);
    itInsert(name, yyvsp[0].type);
;}
    break;

  case 45:
#line 420 "parser.y"
    { itTypeDecl(yyvsp[-2].identifier, yyval.type = yyvsp[0].type); ;}
    break;

  case 46:
#line 424 "parser.y"
    { yyval.type = itResetType(yyvsp[0].type); ;}
    break;

  case 47:
#line 427 "parser.y"
    {
    yyval.type = yyvsp[-7].type;

    if ((yyval.type->itTransType != strNULL) && !streql(yyval.type->itTransType, yyvsp[-4].identifier))
	warn("conflicting translation types (%s, %s)",
	     yyval.type->itTransType, yyvsp[-4].identifier);
    yyval.type->itTransType = yyvsp[-4].identifier;

    if ((yyval.type->itInTrans != strNULL) && !streql(yyval.type->itInTrans, yyvsp[-3].identifier))
	warn("conflicting in-translation functions (%s, %s)",
	     yyval.type->itInTrans, yyvsp[-3].identifier);
    yyval.type->itInTrans = yyvsp[-3].identifier;

    if ((yyval.type->itServerType != strNULL) && !streql(yyval.type->itServerType, yyvsp[-1].identifier))
	warn("conflicting server types (%s, %s)",
	     yyval.type->itServerType, yyvsp[-1].identifier);
    yyval.type->itServerType = yyvsp[-1].identifier;
;}
    break;

  case 48:
#line 447 "parser.y"
    {
    yyval.type = yyvsp[-7].type;

    if ((yyval.type->itServerType != strNULL) && !streql(yyval.type->itServerType, yyvsp[-4].identifier))
	warn("conflicting server types (%s, %s)",
	     yyval.type->itServerType, yyvsp[-4].identifier);
    yyval.type->itServerType = yyvsp[-4].identifier;

    if ((yyval.type->itOutTrans != strNULL) && !streql(yyval.type->itOutTrans, yyvsp[-3].identifier))
	warn("conflicting out-translation functions (%s, %s)",
	     yyval.type->itOutTrans, yyvsp[-3].identifier);
    yyval.type->itOutTrans = yyvsp[-3].identifier;

    if ((yyval.type->itTransType != strNULL) && !streql(yyval.type->itTransType, yyvsp[-1].identifier))
	warn("conflicting translation types (%s, %s)",
	     yyval.type->itTransType, yyvsp[-1].identifier);
    yyval.type->itTransType = yyvsp[-1].identifier;
;}
    break;

  case 49:
#line 467 "parser.y"
    {
    yyval.type = yyvsp[-6].type;

    if ((yyval.type->itDestructor != strNULL) && !streql(yyval.type->itDestructor, yyvsp[-3].identifier))
	warn("conflicting destructor functions (%s, %s)",
	     yyval.type->itDestructor, yyvsp[-3].identifier);
    yyval.type->itDestructor = yyvsp[-3].identifier;

    if ((yyval.type->itTransType != strNULL) && !streql(yyval.type->itTransType, yyvsp[-1].identifier))
	warn("conflicting translation types (%s, %s)",
	     yyval.type->itTransType, yyvsp[-1].identifier);
    yyval.type->itTransType = yyvsp[-1].identifier;
;}
    break;

  case 50:
#line 481 "parser.y"
    {
    yyval.type = yyvsp[-3].type;

    if ((yyval.type->itUserType != strNULL) && !streql(yyval.type->itUserType, yyvsp[0].identifier))
	warn("conflicting user types (%s, %s)",
	     yyval.type->itUserType, yyvsp[0].identifier);
    yyval.type->itUserType = yyvsp[0].identifier;

    if ((yyval.type->itServerType != strNULL) && !streql(yyval.type->itServerType, yyvsp[0].identifier))
	warn("conflicting server types (%s, %s)",
	     yyval.type->itServerType, yyvsp[0].identifier);
    yyval.type->itServerType = yyvsp[0].identifier;
;}
    break;

  case 51:
#line 495 "parser.y"
    {
    yyval.type = yyvsp[-3].type;

    if ((yyval.type->itUserType != strNULL) && !streql(yyval.type->itUserType, yyvsp[0].identifier))
	warn("conflicting user types (%s, %s)",
	     yyval.type->itUserType, yyvsp[0].identifier);
    yyval.type->itUserType = yyvsp[0].identifier;
;}
    break;

  case 52:
#line 505 "parser.y"
    {
    yyval.type = yyvsp[-3].type;

    if ((yyval.type->itServerType != strNULL) && !streql(yyval.type->itServerType, yyvsp[0].identifier))
	warn("conflicting server types (%s, %s)",
	     yyval.type->itServerType, yyvsp[0].identifier);
    yyval.type->itServerType = yyvsp[0].identifier;
;}
    break;

  case 53:
#line 516 "parser.y"
    { yyval.type = yyvsp[0].type; ;}
    break;

  case 54:
#line 518 "parser.y"
    { yyval.type = yyvsp[0].type; ;}
    break;

  case 55:
#line 520 "parser.y"
    { yyval.type = itVarArrayDecl(yyvsp[-1].number, yyvsp[0].type); ;}
    break;

  case 56:
#line 522 "parser.y"
    { yyval.type = itArrayDecl(yyvsp[-1].number, yyvsp[0].type); ;}
    break;

  case 57:
#line 524 "parser.y"
    { yyval.type = itPtrDecl(yyvsp[0].type); ;}
    break;

  case 58:
#line 526 "parser.y"
    { yyval.type = itStructDecl(yyvsp[-1].number, yyvsp[0].type); ;}
    break;

  case 59:
#line 528 "parser.y"
    { yyval.type = yyvsp[0].type; ;}
    break;

  case 60:
#line 530 "parser.y"
    { yyval.type = yyvsp[0].type; ;}
    break;

  case 61:
#line 534 "parser.y"
    { yyval.type = itNativeType(yyvsp[-1].identifier, TRUE, 0); ;}
    break;

  case 62:
#line 537 "parser.y"
    { yyval.type = itNativeType(yyvsp[-3].identifier, TRUE, yyvsp[-1].identifier); ;}
    break;

  case 63:
#line 539 "parser.y"
    { yyval.type = itNativeType(yyvsp[-1].identifier, FALSE, 0); ;}
    break;

  case 64:
#line 543 "parser.y"
    {
    yyval.type = itShortDecl(yyvsp[0].symtype.innumber, yyvsp[0].symtype.instr,
		     yyvsp[0].symtype.outnumber, yyvsp[0].symtype.outstr,
		     yyvsp[0].symtype.size);
;}
    break;

  case 65:
#line 550 "parser.y"
    {
    error("Long form type declarations aren't allowed anylonger\n");
;}
    break;

  case 66:
#line 556 "parser.y"
    {
    yyval.symtype.innumber = yyval.symtype.outnumber = yyvsp[0].number;
    yyval.symtype.instr = yyval.symtype.outstr = strNULL;
    yyval.symtype.size = 0;
;}
    break;

  case 67:
#line 562 "parser.y"
    { yyval.symtype = yyvsp[0].symtype; ;}
    break;

  case 68:
#line 566 "parser.y"
    { yyval.symtype = yyvsp[0].symtype; ;}
    break;

  case 69:
#line 568 "parser.y"
    {
    if (yyvsp[-2].symtype.size != yyvsp[0].symtype.size)
    {
	if (yyvsp[-2].symtype.size == 0)
	    yyval.symtype.size = yyvsp[0].symtype.size;
	else if (yyvsp[0].symtype.size == 0)
	    yyval.symtype.size = yyvsp[-2].symtype.size;
	else
	{
	    error("sizes in IPCTypes (%d, %d) aren't equal",
		  yyvsp[-2].symtype.size, yyvsp[0].symtype.size);
	    yyval.symtype.size = 0;
	}
    }
    else
	yyval.symtype.size = yyvsp[-2].symtype.size;
    yyval.symtype.innumber = yyvsp[-2].symtype.innumber;
    yyval.symtype.instr = yyvsp[-2].symtype.instr;
    yyval.symtype.outnumber = yyvsp[0].symtype.outnumber;
    yyval.symtype.outstr = yyvsp[0].symtype.outstr;
;}
    break;

  case 70:
#line 592 "parser.y"
    { yyval.type = itPrevDecl(yyvsp[0].identifier); ;}
    break;

  case 71:
#line 596 "parser.y"
    { yyval.number = 0; ;}
    break;

  case 72:
#line 598 "parser.y"
    { yyval.number = 0; ;}
    break;

  case 73:
#line 601 "parser.y"
    { yyval.number = yyvsp[-2].number; ;}
    break;

  case 74:
#line 605 "parser.y"
    { yyval.number = yyvsp[-2].number; ;}
    break;

  case 75:
#line 609 "parser.y"
    { yyval.number = yyvsp[-2].number; ;}
    break;

  case 76:
#line 613 "parser.y"
    { yyval.type = itCStringDecl(yyvsp[-1].number, FALSE); ;}
    break;

  case 77:
#line 616 "parser.y"
    { yyval.type = itCStringDecl(yyvsp[-1].number, TRUE); ;}
    break;

  case 78:
#line 620 "parser.y"
    { yyval.identifier = yyvsp[0].identifier; ;}
    break;

  case 79:
#line 622 "parser.y"
    { yyval.identifier = strphrase(yyvsp[-1].identifier, yyvsp[0].identifier); strfree(yyvsp[0].identifier); ;}
    break;

  case 80:
#line 626 "parser.y"
    { yyval.number = yyvsp[-2].number + yyvsp[0].number;	;}
    break;

  case 81:
#line 628 "parser.y"
    { yyval.number = yyvsp[-2].number - yyvsp[0].number;	;}
    break;

  case 82:
#line 630 "parser.y"
    { yyval.number = yyvsp[-2].number * yyvsp[0].number;	;}
    break;

  case 83:
#line 632 "parser.y"
    { yyval.number = yyvsp[-2].number / yyvsp[0].number;	;}
    break;

  case 84:
#line 634 "parser.y"
    { yyval.number = yyvsp[0].number;	;}
    break;

  case 85:
#line 636 "parser.y"
    { yyval.number = yyvsp[-1].number;	;}
    break;

  case 86:
#line 640 "parser.y"
    { yyval.routine = yyvsp[0].routine; ;}
    break;

  case 87:
#line 641 "parser.y"
    { yyval.routine = yyvsp[0].routine; ;}
    break;

  case 88:
#line 645 "parser.y"
    { yyval.routine = rtMakeRoutine(yyvsp[-1].identifier, yyvsp[0].argument); ;}
    break;

  case 89:
#line 649 "parser.y"
    { yyval.routine = rtMakeSimpleRoutine(yyvsp[-1].identifier, yyvsp[0].argument); ;}
    break;

  case 90:
#line 653 "parser.y"
    { yyval.argument = argNULL; ;}
    break;

  case 91:
#line 655 "parser.y"
    { yyval.argument = yyvsp[-1].argument; ;}
    break;

  case 92:
#line 660 "parser.y"
    { yyval.argument = yyvsp[0].argument; ;}
    break;

  case 93:
#line 662 "parser.y"
    { yyval.argument = yyvsp[0].argument; ;}
    break;

  case 94:
#line 664 "parser.y"
    {
    yyval.argument = yyvsp[-2].argument;
    yyval.argument->argNext = yyvsp[0].argument;
;}
    break;

  case 95:
#line 669 "parser.y"
    {
    yyval.argument = yyvsp[-2].argument;
    yyval.argument->argNext = yyvsp[0].argument;
;}
    break;

  case 96:
#line 676 "parser.y"
    {
    yyval.argument = argAlloc();
    yyval.argument->argKind = yyvsp[-3].direction;
    yyval.argument->argName = yyvsp[-2].identifier;
    yyval.argument->argType = yyvsp[-1].type;
    yyval.argument->argFlags = yyvsp[0].flag;
    if (yyvsp[-1].type->itNative)
    {
        if (yyvsp[-3].direction != akIn && yyvsp[-3].direction != akOut && yyvsp[-3].direction != akInOut)
	    error("Illegal direction specified");
       
        if (!(yyvsp[-1].type->itNativePointer) && yyvsp[-3].direction != akIn)
	    error("ValueOf only valid for in");

        if ((yyvsp[-1].type->itBadValue) != NULL && yyvsp[-3].direction != akIn)
	    error("PointerToIfNot only valid for in");
    }
;}
    break;

  case 97:
#line 697 "parser.y"
    {
    yyval.argument = argAlloc();
    yyval.argument->argKind = yyvsp[-2].direction;
    yyval.argument->argName = yyvsp[-1].identifier;
    yyval.argument->argType = yyvsp[0].type;
;}
    break;

  case 98:
#line 704 "parser.y"
    {
    yyval.argument = argAlloc();
    yyval.argument->argKind = yyvsp[-4].direction;
    yyval.argument->argName = yyvsp[-3].identifier;
    yyval.argument->argType = yyvsp[-2].type;
    yyval.argument->argMsgField = yyvsp[0].identifier;
;}
    break;

  case 99:
#line 714 "parser.y"
    { yyval.direction = akNone; ;}
    break;

  case 100:
#line 715 "parser.y"
    { yyval.direction = akIn; ;}
    break;

  case 101:
#line 716 "parser.y"
    { yyval.direction = akOut; ;}
    break;

  case 102:
#line 717 "parser.y"
    { yyval.direction = akInOut; ;}
    break;

  case 103:
#line 718 "parser.y"
    { yyval.direction = akRequestPort; ;}
    break;

  case 104:
#line 719 "parser.y"
    { yyval.direction = akReplyPort; ;}
    break;

  case 105:
#line 720 "parser.y"
    { yyval.direction = akSReplyPort; ;}
    break;

  case 106:
#line 721 "parser.y"
    { yyval.direction = akUReplyPort; ;}
    break;

  case 107:
#line 722 "parser.y"
    { yyval.direction = akWaitTime; ;}
    break;

  case 108:
#line 723 "parser.y"
    { yyval.direction = akMsgOption; ;}
    break;

  case 109:
#line 726 "parser.y"
    { yyval.direction = akServerImpl; ;}
    break;

  case 110:
#line 727 "parser.y"
    { yyval.direction = akUserImpl; ;}
    break;

  case 111:
#line 730 "parser.y"
    { yyval.direction = akServerSecToken; ;}
    break;

  case 112:
#line 731 "parser.y"
    { yyval.direction = akUserSecToken; ;}
    break;

  case 113:
#line 732 "parser.y"
    { yyval.direction = akMsgSeqno; ;}
    break;

  case 114:
#line 736 "parser.y"
    {
    yyval.type = itLookUp(yyvsp[0].identifier);
    if (yyval.type == itNULL)
	error("type '%s' not defined", yyvsp[0].identifier);
;}
    break;

  case 115:
#line 742 "parser.y"
    { yyval.type = yyvsp[0].type; ;}
    break;

  case 116:
#line 744 "parser.y"
    { yyval.type = yyvsp[0].type; ;}
    break;

  case 117:
#line 748 "parser.y"
    { yyval.flag = flNone; ;}
    break;

  case 118:
#line 750 "parser.y"
    {
    if (yyvsp[-2].flag & yyvsp[0].flag)
	warn("redundant IPC flag ignored");
    else
	yyval.flag = yyvsp[-2].flag | yyvsp[0].flag;
;}
    break;

  case 119:
#line 757 "parser.y"
    {
    if (yyvsp[-2].flag != flDealloc)
	warn("only Dealloc is variable");
    else
	yyval.flag = yyvsp[-4].flag | flMaybeDealloc;
;}
    break;

  case 120:
#line 765 "parser.y"
    { LookString(); ;}
    break;

  case 121:
#line 769 "parser.y"
    { LookFileName(); ;}
    break;

  case 122:
#line 773 "parser.y"
    { LookQString(); ;}
    break;


    }

/* Line 999 of yacc.c.  */
#line 2149 "parser.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 776 "parser.y"


void
yyerror(s)
    char *s;
{
    error(s);
}

static char *
import_name(sk)
    statement_kind_t sk;
{
    switch (sk)
    {
      case skImport:
	return "Import";
      case skSImport:
	return "SImport";
      case skUImport:
	return "UImport";
      case skIImport:
	return "IImport";
      case skDImport:
	return "DImport";
      default:
	fatal("import_name(%d): not import statement", (int) sk);
	/*NOTREACHED*/
        return strNULL;
    }
}

