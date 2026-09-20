
/*  A Bison parser, made from parser.y
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	sySkip	258
#define	syRoutine	259
#define	sySimpleRoutine	260
#define	sySubsystem	261
#define	syKernelUser	262
#define	syKernelServer	263
#define	syMsgOption	264
#define	syMsgSeqno	265
#define	syWaitTime	266
#define	syNoWaitTime	267
#define	syErrorProc	268
#define	syServerPrefix	269
#define	syUserPrefix	270
#define	syServerDemux	271
#define	syRCSId	272
#define	syImport	273
#define	syUImport	274
#define	sySImport	275
#define	syIImport	276
#define	syDImport	277
#define	syIn	278
#define	syOut	279
#define	syInOut	280
#define	syUserImpl	281
#define	syServerImpl	282
#define	syRequestPort	283
#define	syReplyPort	284
#define	sySReplyPort	285
#define	syUReplyPort	286
#define	syType	287
#define	syArray	288
#define	syStruct	289
#define	syOf	290
#define	syInTran	291
#define	syOutTran	292
#define	syDestructor	293
#define	syCType	294
#define	syCUserType	295
#define	syUserTypeLimit	296
#define	syOnStackLimit	297
#define	syCServerType	298
#define	syPointerTo	299
#define	syPointerToIfNot	300
#define	syValueOf	301
#define	syCString	302
#define	syUserSecToken	303
#define	syServerSecToken	304
#define	syColon	305
#define	sySemi	306
#define	syComma	307
#define	syPlus	308
#define	syMinus	309
#define	syStar	310
#define	syDiv	311
#define	syLParen	312
#define	syRParen	313
#define	syEqual	314
#define	syCaret	315
#define	syTilde	316
#define	syLAngle	317
#define	syRAngle	318
#define	syLBrack	319
#define	syRBrack	320
#define	syBar	321
#define	syError	322
#define	syNumber	323
#define	sySymbolicType	324
#define	syIdentifier	325
#define	syString	326
#define	syQString	327
#define	syFileName	328
#define	syIPCFlag	329

#line 187 "parser.y"


#include "lexxer.h"
#include "strdefs.h"
#include "type.h"
#include "routine.h"
#include "statement.h"
#include "global.h"
#include "error.h"

static char *import_name();


#line 201 "parser.y"
typedef union
{
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
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		241
#define	YYFLAG		-32768
#define	YYNTBASE	75

#define YYTRANSLATE(x) ((unsigned)(x) <= 329 ? yytranslate[x] : 124)

static const char yytranslate[] = {     0,
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
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
    66,    67,    68,    69,    70,    71,    72,    73,    74
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     7,    10,    13,    16,    19,    22,    25,
    28,    31,    34,    37,    40,    43,    46,    48,    51,    56,
    58,    59,    62,    64,    66,    68,    70,    74,    77,    80,
    84,    86,    89,    92,    95,    98,   102,   104,   106,   108,
   110,   112,   116,   119,   123,   125,   134,   143,   151,   156,
   161,   166,   168,   170,   173,   176,   179,   182,   184,   186,
   191,   198,   203,   205,   212,   214,   216,   218,   222,   224,
   229,   235,   243,   249,   255,   260,   267,   269,   272,   276,
   280,   284,   288,   290,   294,   296,   298,   302,   306,   309,
   313,   315,   317,   321,   325,   330,   334,   340,   341,   343,
   345,   347,   349,   351,   353,   355,   357,   359,   361,   363,
   365,   367,   369,   372,   375,   378,   379,   383,   389,   390,
   391
};

static const short yyrhs[] = {    -1,
    75,    76,     0,    77,    51,     0,    86,    51,     0,    83,
    51,     0,    84,    51,     0,    85,    51,     0,    87,    51,
     0,    88,    51,     0,    89,    51,     0,    90,    51,     0,
    94,    51,     0,   109,    51,     0,     3,    51,     0,    91,
    51,     0,    93,    51,     0,    51,     0,     1,    51,     0,
    78,    79,    81,    82,     0,     6,     0,     0,    79,    80,
     0,     7,     0,     8,     0,    70,     0,    68,     0,   121,
     9,    71,     0,    41,    68,     0,    42,    68,     0,   121,
    11,    71,     0,    12,     0,    13,    70,     0,    14,    70,
     0,    15,    70,     0,    16,    70,     0,   122,    92,    73,
     0,    18,     0,    19,     0,    20,     0,    21,     0,    22,
     0,   123,    17,    72,     0,    32,    95,     0,    70,    59,
    96,     0,    97,     0,    96,    36,    50,    70,    70,    57,
    70,    58,     0,    96,    37,    50,    70,    70,    57,    70,
    58,     0,    96,    38,    50,    70,    57,    70,    58,     0,
    96,    39,    50,    70,     0,    96,    40,    50,    70,     0,
    96,    43,    50,    70,     0,    99,     0,   102,     0,   103,
    97,     0,   104,    97,     0,    60,    97,     0,   105,    97,
     0,   106,     0,    98,     0,    44,    57,   107,    58,     0,
    45,    57,   107,    52,   107,    58,     0,    46,    57,   107,
    58,     0,   101,     0,    57,   101,    52,   108,   120,    58,
     0,    68,     0,    69,     0,   100,     0,   100,    66,   100,
     0,    70,     0,    33,    64,    65,    35,     0,    33,    64,
    55,    65,    35,     0,    33,    64,    55,    50,   108,    65,
    35,     0,    33,    64,   108,    65,    35,     0,    34,    64,
   108,    65,    35,     0,    47,    64,   108,    65,     0,    47,
    64,    55,    50,   108,    65,     0,    70,     0,   107,    70,
     0,   108,    53,   108,     0,   108,    54,   108,     0,   108,
    55,   108,     0,   108,    56,   108,     0,    68,     0,    57,
   108,    58,     0,   110,     0,   111,     0,     4,    70,   112,
     0,     5,    70,   112,     0,    57,    58,     0,    57,   113,
    58,     0,   114,     0,   115,     0,   114,    51,   113,     0,
   115,    51,   113,     0,   116,    70,   119,   120,     0,   118,
    70,   119,     0,   117,    70,   119,    52,    70,     0,     0,
    23,     0,    24,     0,    25,     0,    28,     0,    29,     0,
    30,     0,    31,     0,    11,     0,     9,     0,    27,     0,
    26,     0,    49,     0,    48,     0,    10,     0,    50,    70,
     0,    50,    95,     0,    50,    98,     0,     0,   120,    52,
    74,     0,   120,    52,    74,    64,    65,     0,     0,     0,
     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   224,   225,   228,   229,   230,   231,   232,   233,   234,   235,
   236,   237,   238,   248,   250,   251,   252,   253,   257,   270,
   282,   283,   286,   297,   305,   308,   311,   328,   331,   335,
   341,   349,   357,   365,   373,   381,   392,   393,   394,   395,
   396,   399,   409,   419,   423,   425,   445,   465,   480,   494,
   503,   515,   517,   519,   521,   523,   525,   527,   529,   533,
   535,   538,   542,   548,   555,   561,   565,   567,   591,   595,
   597,   599,   604,   608,   612,   614,   619,   621,   625,   627,
   629,   631,   633,   635,   640,   641,   644,   648,   652,   654,
   659,   661,   663,   668,   675,   696,   703,   714,   715,   716,
   717,   718,   719,   720,   721,   722,   723,   726,   727,   730,
   731,   732,   735,   741,   743,   747,   749,   756,   764,   768,
   772
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","sySkip",
"syRoutine","sySimpleRoutine","sySubsystem","syKernelUser","syKernelServer",
"syMsgOption","syMsgSeqno","syWaitTime","syNoWaitTime","syErrorProc","syServerPrefix",
"syUserPrefix","syServerDemux","syRCSId","syImport","syUImport","sySImport",
"syIImport","syDImport","syIn","syOut","syInOut","syUserImpl","syServerImpl",
"syRequestPort","syReplyPort","sySReplyPort","syUReplyPort","syType","syArray",
"syStruct","syOf","syInTran","syOutTran","syDestructor","syCType","syCUserType",
"syUserTypeLimit","syOnStackLimit","syCServerType","syPointerTo","syPointerToIfNot",
"syValueOf","syCString","syUserSecToken","syServerSecToken","syColon","sySemi",
"syComma","syPlus","syMinus","syStar","syDiv","syLParen","syRParen","syEqual",
"syCaret","syTilde","syLAngle","syRAngle","syLBrack","syRBrack","syBar","syError",
"syNumber","sySymbolicType","syIdentifier","syString","syQString","syFileName",
"syIPCFlag","Statements","Statement","Subsystem","SubsystemStart","SubsystemMods",
"SubsystemMod","SubsystemName","SubsystemBase","MsgOption","UserTypeLimit","OnStackLimit",
"WaitTime","Error","ServerPrefix","UserPrefix","ServerDemux","Import","ImportIndicant",
"RCSDecl","TypeDecl","NamedTypeSpec","TransTypeSpec","TypeSpec","NativeTypeSpec",
"BasicTypeSpec","PrimIPCType","IPCType","PrevTypeSpec","VarArrayHead","ArrayHead",
"StructHead","CStringSpec","TypePhrase","IntExp","RoutineDecl","Routine","SimpleRoutine",
"Arguments","ArgumentList","Argument","Trailer","Direction","TrImplKeyword",
"TrExplKeyword","ArgumentType","IPCFlags","LookString","LookFileName","LookQString", NULL
};
#endif

static const short yyr1[] = {     0,
    75,    75,    76,    76,    76,    76,    76,    76,    76,    76,
    76,    76,    76,    76,    76,    76,    76,    76,    77,    78,
    79,    79,    80,    80,    81,    82,    83,    84,    85,    86,
    86,    87,    88,    89,    90,    91,    92,    92,    92,    92,
    92,    93,    94,    95,    96,    96,    96,    96,    96,    96,
    96,    97,    97,    97,    97,    97,    97,    97,    97,    98,
    98,    98,    99,    99,   100,   100,   101,   101,   102,   103,
   103,   103,   104,   105,   106,   106,   107,   107,   108,   108,
   108,   108,   108,   108,   109,   109,   110,   111,   112,   112,
   113,   113,   113,   113,   114,   115,   115,   116,   116,   116,
   116,   116,   116,   116,   116,   116,   116,   117,   117,   118,
   118,   118,   119,   119,   119,   120,   120,   120,   121,   122,
   123
};

static const short yyr2[] = {     0,
     0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     1,     2,     4,     1,
     0,     2,     1,     1,     1,     1,     3,     2,     2,     3,
     1,     2,     2,     2,     2,     3,     1,     1,     1,     1,
     1,     3,     2,     3,     1,     8,     8,     7,     4,     4,
     4,     1,     1,     2,     2,     2,     2,     1,     1,     4,
     6,     4,     1,     6,     1,     1,     1,     3,     1,     4,
     5,     7,     5,     5,     4,     6,     1,     2,     3,     3,
     3,     3,     1,     3,     1,     1,     3,     3,     2,     3,
     1,     1,     3,     3,     4,     3,     5,     0,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     2,     2,     2,     0,     3,     5,     0,     0,
     0
};

static const short yydefact[] = {     1,
     0,     0,     0,     0,     0,    20,    31,     0,     0,     0,
     0,     0,     0,     0,    17,     2,     0,    21,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    85,    86,     0,     0,     0,    18,    14,     0,     0,    32,
    33,    34,    35,     0,    43,    28,    29,     3,     0,     5,
     6,     7,     4,     8,     9,    10,    11,    15,    16,    12,
    13,     0,     0,    37,    38,    39,    40,    41,     0,     0,
    98,    87,    88,     0,    23,    24,    25,    22,     0,    27,
    30,    36,    42,   107,   112,   106,    99,   100,   101,   109,
   108,   102,   103,   104,   105,   111,   110,    89,     0,    91,
    92,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    65,    66,    69,    44,    45,    59,    52,    67,
    63,    53,     0,     0,     0,    58,    26,    19,    90,    98,
    98,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    56,     0,     0,     0,     0,     0,     0,     0,    54,
    55,    57,    93,    94,     0,   116,     0,    96,     0,     0,
     0,    83,     0,     0,    77,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    68,   113,   114,
   115,    95,     0,     0,     0,     0,    70,     0,     0,     0,
     0,     0,     0,    60,    78,     0,    62,     0,    75,   116,
     0,     0,     0,    49,    50,    51,     0,    97,     0,    71,
    84,    79,    80,    81,    82,    73,    74,     0,     0,     0,
     0,     0,     0,   117,     0,    61,    76,    64,     0,     0,
     0,     0,    72,     0,     0,    48,   118,    46,    47,     0,
     0
};

static const short yydefgoto[] = {     1,
    16,    17,    18,    49,    78,    79,   128,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    69,    28,    29,    45,
   116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
   126,   166,   163,    30,    31,    32,    72,    99,   100,   101,
   102,   103,   104,   156,   182,    33,    34,    35
};

static const short yypact[] = {-32768,
    27,   -32,   -26,   -58,   -35,-32768,-32768,     7,    29,    39,
    42,    43,   -15,    58,-32768,-32768,    60,-32768,    93,    95,
    96,    98,   106,   107,   111,   112,   119,   126,   137,   138,
-32768,-32768,     6,   161,   125,-32768,-32768,   114,   114,-32768,
-32768,-32768,-32768,   132,-32768,-32768,-32768,-32768,    -7,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    79,   121,-32768,-32768,-32768,-32768,-32768,   117,   122,
    56,-32768,-32768,    91,-32768,-32768,-32768,-32768,   127,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   135,   145,
   146,   128,   130,   133,   140,   141,   142,   144,   149,   143,
   -17,    91,-32768,-32768,-32768,   129,-32768,-32768,-32768,   136,
-32768,-32768,    91,    91,    91,-32768,-32768,-32768,-32768,    92,
    92,   158,   158,   158,   -44,    88,   139,   139,   139,   -39,
   159,-32768,   160,   162,   163,   164,   165,   166,   -17,-32768,
-32768,-32768,-32768,-32768,   -36,-32768,   167,-32768,   -43,    88,
   182,-32768,    35,    41,-32768,     0,   -47,     4,   168,    74,
    88,   150,   151,   152,   153,   154,   155,-32768,   132,-32768,
-32768,   174,   157,    88,   193,   120,-32768,    88,    88,    88,
    88,   194,   195,-32768,-32768,   139,-32768,    88,-32768,   131,
   169,   170,   175,-32768,-32768,-32768,   171,-32768,    78,-32768,
-32768,    20,    20,-32768,-32768,-32768,-32768,    40,    99,   -38,
   176,   177,   172,   173,   196,-32768,-32768,-32768,   178,   179,
   180,   181,-32768,   183,   185,-32768,-32768,-32768,-32768,   235,
-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    81,
-32768,   -52,    89,-32768,   101,   147,-32768,-32768,-32768,-32768,
-32768,  -135,  -134,-32768,-32768,-32768,   208,   -23,-32768,-32768,
-32768,-32768,-32768,   -41,    51,-32768,-32768,-32768
};


#define	YYLAST		258


static const short yytable[] = {    75,
    76,   164,   167,   168,   196,   170,   184,   107,   108,   109,
   159,    38,   160,   207,    62,   169,    63,   160,    36,   228,
   161,   185,   195,   162,    37,   186,   240,     2,   162,     3,
     4,     5,     6,   179,    39,  -119,   200,  -119,     7,     8,
     9,    10,    11,  -121,  -120,  -120,  -120,  -120,  -120,   209,
   113,   114,    46,   212,   213,   214,   215,   194,    12,   142,
   218,   197,    77,   219,    84,    85,    86,    13,    14,   195,
   150,   151,   152,   195,   190,   191,    40,    15,    87,    88,
    89,    90,    91,    92,    93,    94,    95,   188,   189,   190,
   191,   157,   158,   188,   189,   190,   191,   226,    41,   192,
    84,    85,    86,    96,    97,   193,   153,   154,    42,   195,
    48,    43,    44,    98,    87,    88,    89,    90,    91,    92,
    93,    94,    95,   105,   106,    47,   188,   189,   190,   191,
   188,   189,   190,   191,   107,   108,   109,   110,   199,    96,
    97,    70,   225,    50,   160,    51,    52,   111,    53,    80,
   112,   188,   189,   190,   191,   162,    54,    55,   113,   114,
   115,    56,    57,   227,   143,   144,   145,   146,   147,    58,
    71,   148,   188,   189,   190,   191,    59,   211,    64,    65,
    66,    67,    68,   188,   189,   190,   191,    60,    61,    82,
    74,    81,   129,    83,   127,   130,   131,   132,   137,   133,
   138,   149,   134,   135,   136,   139,   140,   155,   165,   172,
   171,   173,   174,   175,   176,   177,   187,   198,   183,   201,
   202,   203,   204,   205,   206,   207,   208,   210,   216,   217,
   233,   223,   229,   230,   241,   180,   232,   236,   221,   222,
   238,   231,   239,   181,   224,   237,    73,   234,   235,   178,
   220,     0,     0,     0,     0,     0,     0,   141
};

static const short yycheck[] = {     7,
     8,   136,   138,   139,    52,   140,    50,    44,    45,    46,
    55,    70,    57,    52,     9,    55,    11,    57,    51,    58,
    65,    65,    70,    68,    51,   160,     0,     1,    68,     3,
     4,     5,     6,    70,    70,     9,   171,    11,    12,    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,   184,
    68,    69,    68,   188,   189,   190,   191,    58,    32,   112,
   196,    58,    70,   198,     9,    10,    11,    41,    42,    70,
   123,   124,   125,    70,    55,    56,    70,    51,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    53,    54,    55,
    56,   133,   134,    53,    54,    55,    56,    58,    70,    65,
     9,    10,    11,    48,    49,    65,   130,   131,    70,    70,
    51,    70,    70,    58,    23,    24,    25,    26,    27,    28,
    29,    30,    31,    33,    34,    68,    53,    54,    55,    56,
    53,    54,    55,    56,    44,    45,    46,    47,    65,    48,
    49,    17,    65,    51,    57,    51,    51,    57,    51,    71,
    60,    53,    54,    55,    56,    68,    51,    51,    68,    69,
    70,    51,    51,    65,    36,    37,    38,    39,    40,    51,
    57,    43,    53,    54,    55,    56,    51,    58,    18,    19,
    20,    21,    22,    53,    54,    55,    56,    51,    51,    73,
    59,    71,    58,    72,    68,    51,    51,    70,    57,    70,
    57,    66,    70,    64,    64,    57,    64,    50,    70,    50,
    52,    50,    50,    50,    50,    50,    35,    50,    52,    70,
    70,    70,    70,    70,    70,    52,    70,    35,    35,    35,
    35,    57,    57,    57,     0,   155,    64,    58,    70,    70,
    58,    70,    58,   155,    74,    65,    39,    70,    70,   149,
   200,    -1,    -1,    -1,    -1,    -1,    -1,   111
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 196 "/usr/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 13:
#line 239 "parser.y"
{
    register statement_t *st = stAlloc();

    st->stKind = skRoutine;
    st->stRoutine = yyvsp[-1].routine;
    rtCheckRoutine(yyvsp[-1].routine);
    if (BeVerbose)
	rtPrintRoutine(yyvsp[-1].routine);
;
    break;}
case 14:
#line 249 "parser.y"
{ rtSkip(); ;
    break;}
case 18:
#line 254 "parser.y"
{ yyerrok; ;
    break;}
case 19:
#line 259 "parser.y"
{
    if (BeVerbose)
    {
	printf("Subsystem %s: base = %u%s%s\n\n",
	       SubsystemName, SubsystemBase,
	       IsKernelUser ? ", KernelUser" : "",
	       IsKernelServer ? ", KernelServer" : "");
    }
;
    break;}
case 20:
#line 271 "parser.y"
{
    if (SubsystemName != strNULL)
    {
	warn("previous Subsystem decl (of %s) will be ignored", SubsystemName);
	IsKernelUser = FALSE;
	IsKernelServer = FALSE;
	strfree(SubsystemName);
    }
;
    break;}
case 23:
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
;
    break;}
case 24:
#line 298 "parser.y"
{
    if (IsKernelServer)
	warn("duplicate KernelServer keyword");
    IsKernelServer = TRUE;
;
    break;}
case 25:
#line 305 "parser.y"
{ SubsystemName = yyvsp[0].identifier; ;
    break;}
case 26:
#line 308 "parser.y"
{ SubsystemBase = yyvsp[0].number; ;
    break;}
case 27:
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
;
    break;}
case 28:
#line 329 "parser.y"
{UserTypeLimit = yyvsp[0].number; ;
    break;}
case 29:
#line 332 "parser.y"
{MaxMessSizeOnStack = yyvsp[0].number; ;
    break;}
case 30:
#line 336 "parser.y"
{
    WaitTime = yyvsp[0].string;
    if (BeVerbose)
	printf("WaitTime %s\n\n", WaitTime);
;
    break;}
case 31:
#line 342 "parser.y"
{
    WaitTime = strNULL;
    if (BeVerbose)
	printf("NoWaitTime\n\n");
;
    break;}
case 32:
#line 350 "parser.y"
{
    ErrorProc = yyvsp[0].identifier;
    if (BeVerbose)
	printf("ErrorProc %s\n\n", ErrorProc);
;
    break;}
case 33:
#line 358 "parser.y"
{
    ServerPrefix = yyvsp[0].identifier;
    if (BeVerbose)
	printf("ServerPrefix %s\n\n", ServerPrefix);
;
    break;}
case 34:
#line 366 "parser.y"
{
    UserPrefix = yyvsp[0].identifier;
    if (BeVerbose)
	printf("UserPrefix %s\n\n", UserPrefix);
;
    break;}
case 35:
#line 374 "parser.y"
{
    ServerDemux = yyvsp[0].identifier;
    if (BeVerbose)
	printf("ServerDemux %s\n\n", ServerDemux);
;
    break;}
case 36:
#line 382 "parser.y"
{
    register statement_t *st = stAlloc();
    st->stKind = yyvsp[-1].statement_kind;
    st->stFileName = yyvsp[0].string;

    if (BeVerbose)
	printf("%s %s\n\n", import_name(yyvsp[-1].statement_kind), yyvsp[0].string);
;
    break;}
case 37:
#line 392 "parser.y"
{ yyval.statement_kind = skImport; ;
    break;}
case 38:
#line 393 "parser.y"
{ yyval.statement_kind = skUImport; ;
    break;}
case 39:
#line 394 "parser.y"
{ yyval.statement_kind = skSImport; ;
    break;}
case 40:
#line 395 "parser.y"
{ yyval.statement_kind = skIImport; ;
    break;}
case 41:
#line 396 "parser.y"
{ yyval.statement_kind = skDImport; ;
    break;}
case 42:
#line 400 "parser.y"
{
    if (RCSId != strNULL)
	warn("previous RCS decl will be ignored");
    if (BeVerbose)
	printf("RCSId %s\n\n", yyvsp[0].string);
    RCSId = yyvsp[0].string;
;
    break;}
case 43:
#line 410 "parser.y"
{
    register identifier_t name = yyvsp[0].type->itName;

    if (itLookUp(name) != itNULL)
	warn("overriding previous definition of %s", name);
    itInsert(name, yyvsp[0].type);
;
    break;}
case 44:
#line 420 "parser.y"
{ itTypeDecl(yyvsp[-2].identifier, yyval.type = yyvsp[0].type); ;
    break;}
case 45:
#line 424 "parser.y"
{ yyval.type = itResetType(yyvsp[0].type); ;
    break;}
case 46:
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
;
    break;}
case 47:
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
;
    break;}
case 48:
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
;
    break;}
case 49:
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
;
    break;}
case 50:
#line 495 "parser.y"
{
    yyval.type = yyvsp[-3].type;

    if ((yyval.type->itUserType != strNULL) && !streql(yyval.type->itUserType, yyvsp[0].identifier))
	warn("conflicting user types (%s, %s)",
	     yyval.type->itUserType, yyvsp[0].identifier);
    yyval.type->itUserType = yyvsp[0].identifier;
;
    break;}
case 51:
#line 505 "parser.y"
{
    yyval.type = yyvsp[-3].type;

    if ((yyval.type->itServerType != strNULL) && !streql(yyval.type->itServerType, yyvsp[0].identifier))
	warn("conflicting server types (%s, %s)",
	     yyval.type->itServerType, yyvsp[0].identifier);
    yyval.type->itServerType = yyvsp[0].identifier;
;
    break;}
case 52:
#line 516 "parser.y"
{ yyval.type = yyvsp[0].type; ;
    break;}
case 53:
#line 518 "parser.y"
{ yyval.type = yyvsp[0].type; ;
    break;}
case 54:
#line 520 "parser.y"
{ yyval.type = itVarArrayDecl(yyvsp[-1].number, yyvsp[0].type); ;
    break;}
case 55:
#line 522 "parser.y"
{ yyval.type = itArrayDecl(yyvsp[-1].number, yyvsp[0].type); ;
    break;}
case 56:
#line 524 "parser.y"
{ yyval.type = itPtrDecl(yyvsp[0].type); ;
    break;}
case 57:
#line 526 "parser.y"
{ yyval.type = itStructDecl(yyvsp[-1].number, yyvsp[0].type); ;
    break;}
case 58:
#line 528 "parser.y"
{ yyval.type = yyvsp[0].type; ;
    break;}
case 59:
#line 530 "parser.y"
{ yyval.type = yyvsp[0].type; ;
    break;}
case 60:
#line 534 "parser.y"
{ yyval.type = itNativeType(yyvsp[-1].identifier, TRUE, 0); ;
    break;}
case 61:
#line 537 "parser.y"
{ yyval.type = itNativeType(yyvsp[-3].identifier, TRUE, yyvsp[-1].identifier); ;
    break;}
case 62:
#line 539 "parser.y"
{ yyval.type = itNativeType(yyvsp[-1].identifier, FALSE, 0); ;
    break;}
case 63:
#line 543 "parser.y"
{
    yyval.type = itShortDecl(yyvsp[0].symtype.innumber, yyvsp[0].symtype.instr,
		     yyvsp[0].symtype.outnumber, yyvsp[0].symtype.outstr,
		     yyvsp[0].symtype.size);
;
    break;}
case 64:
#line 550 "parser.y"
{
    error("Long form type declarations aren't allowed anylonger\n");
;
    break;}
case 65:
#line 556 "parser.y"
{
    yyval.symtype.innumber = yyval.symtype.outnumber = yyvsp[0].number;
    yyval.symtype.instr = yyval.symtype.outstr = strNULL;
    yyval.symtype.size = 0;
;
    break;}
case 66:
#line 562 "parser.y"
{ yyval.symtype = yyvsp[0].symtype; ;
    break;}
case 67:
#line 566 "parser.y"
{ yyval.symtype = yyvsp[0].symtype; ;
    break;}
case 68:
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
;
    break;}
case 69:
#line 592 "parser.y"
{ yyval.type = itPrevDecl(yyvsp[0].identifier); ;
    break;}
case 70:
#line 596 "parser.y"
{ yyval.number = 0; ;
    break;}
case 71:
#line 598 "parser.y"
{ yyval.number = 0; ;
    break;}
case 72:
#line 601 "parser.y"
{ yyval.number = yyvsp[-2].number; ;
    break;}
case 73:
#line 605 "parser.y"
{ yyval.number = yyvsp[-2].number; ;
    break;}
case 74:
#line 609 "parser.y"
{ yyval.number = yyvsp[-2].number; ;
    break;}
case 75:
#line 613 "parser.y"
{ yyval.type = itCStringDecl(yyvsp[-1].number, FALSE); ;
    break;}
case 76:
#line 616 "parser.y"
{ yyval.type = itCStringDecl(yyvsp[-1].number, TRUE); ;
    break;}
case 77:
#line 620 "parser.y"
{ yyval.identifier = yyvsp[0].identifier; ;
    break;}
case 78:
#line 622 "parser.y"
{ yyval.identifier = strphrase(yyvsp[-1].identifier, yyvsp[0].identifier); strfree(yyvsp[0].identifier); ;
    break;}
case 79:
#line 626 "parser.y"
{ yyval.number = yyvsp[-2].number + yyvsp[0].number;	;
    break;}
case 80:
#line 628 "parser.y"
{ yyval.number = yyvsp[-2].number - yyvsp[0].number;	;
    break;}
case 81:
#line 630 "parser.y"
{ yyval.number = yyvsp[-2].number * yyvsp[0].number;	;
    break;}
case 82:
#line 632 "parser.y"
{ yyval.number = yyvsp[-2].number / yyvsp[0].number;	;
    break;}
case 83:
#line 634 "parser.y"
{ yyval.number = yyvsp[0].number;	;
    break;}
case 84:
#line 636 "parser.y"
{ yyval.number = yyvsp[-1].number;	;
    break;}
case 85:
#line 640 "parser.y"
{ yyval.routine = yyvsp[0].routine; ;
    break;}
case 86:
#line 641 "parser.y"
{ yyval.routine = yyvsp[0].routine; ;
    break;}
case 87:
#line 645 "parser.y"
{ yyval.routine = rtMakeRoutine(yyvsp[-1].identifier, yyvsp[0].argument); ;
    break;}
case 88:
#line 649 "parser.y"
{ yyval.routine = rtMakeSimpleRoutine(yyvsp[-1].identifier, yyvsp[0].argument); ;
    break;}
case 89:
#line 653 "parser.y"
{ yyval.argument = argNULL; ;
    break;}
case 90:
#line 655 "parser.y"
{ yyval.argument = yyvsp[-1].argument; ;
    break;}
case 91:
#line 660 "parser.y"
{ yyval.argument = yyvsp[0].argument; ;
    break;}
case 92:
#line 662 "parser.y"
{ yyval.argument = yyvsp[0].argument; ;
    break;}
case 93:
#line 664 "parser.y"
{
    yyval.argument = yyvsp[-2].argument;
    yyval.argument->argNext = yyvsp[0].argument;
;
    break;}
case 94:
#line 669 "parser.y"
{
    yyval.argument = yyvsp[-2].argument;
    yyval.argument->argNext = yyvsp[0].argument;
;
    break;}
case 95:
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
;
    break;}
case 96:
#line 697 "parser.y"
{
    yyval.argument = argAlloc();
    yyval.argument->argKind = yyvsp[-2].direction;
    yyval.argument->argName = yyvsp[-1].identifier;
    yyval.argument->argType = yyvsp[0].type;
;
    break;}
case 97:
#line 704 "parser.y"
{
    yyval.argument = argAlloc();
    yyval.argument->argKind = yyvsp[-4].direction;
    yyval.argument->argName = yyvsp[-3].identifier;
    yyval.argument->argType = yyvsp[-2].type;
    yyval.argument->argMsgField = yyvsp[0].identifier;
;
    break;}
case 98:
#line 714 "parser.y"
{ yyval.direction = akNone; ;
    break;}
case 99:
#line 715 "parser.y"
{ yyval.direction = akIn; ;
    break;}
case 100:
#line 716 "parser.y"
{ yyval.direction = akOut; ;
    break;}
case 101:
#line 717 "parser.y"
{ yyval.direction = akInOut; ;
    break;}
case 102:
#line 718 "parser.y"
{ yyval.direction = akRequestPort; ;
    break;}
case 103:
#line 719 "parser.y"
{ yyval.direction = akReplyPort; ;
    break;}
case 104:
#line 720 "parser.y"
{ yyval.direction = akSReplyPort; ;
    break;}
case 105:
#line 721 "parser.y"
{ yyval.direction = akUReplyPort; ;
    break;}
case 106:
#line 722 "parser.y"
{ yyval.direction = akWaitTime; ;
    break;}
case 107:
#line 723 "parser.y"
{ yyval.direction = akMsgOption; ;
    break;}
case 108:
#line 726 "parser.y"
{ yyval.direction = akServerImpl; ;
    break;}
case 109:
#line 727 "parser.y"
{ yyval.direction = akUserImpl; ;
    break;}
case 110:
#line 730 "parser.y"
{ yyval.direction = akServerSecToken; ;
    break;}
case 111:
#line 731 "parser.y"
{ yyval.direction = akUserSecToken; ;
    break;}
case 112:
#line 732 "parser.y"
{ yyval.direction = akMsgSeqno; ;
    break;}
case 113:
#line 736 "parser.y"
{
    yyval.type = itLookUp(yyvsp[0].identifier);
    if (yyval.type == itNULL)
	error("type '%s' not defined", yyvsp[0].identifier);
;
    break;}
case 114:
#line 742 "parser.y"
{ yyval.type = yyvsp[0].type; ;
    break;}
case 115:
#line 744 "parser.y"
{ yyval.type = yyvsp[0].type; ;
    break;}
case 116:
#line 748 "parser.y"
{ yyval.flag = flNone; ;
    break;}
case 117:
#line 750 "parser.y"
{
    if (yyvsp[-2].flag & yyvsp[0].flag)
	warn("redundant IPC flag ignored");
    else
	yyval.flag = yyvsp[-2].flag | yyvsp[0].flag;
;
    break;}
case 118:
#line 757 "parser.y"
{
    if (yyvsp[-2].flag != flDealloc)
	warn("only Dealloc is variable");
    else
	yyval.flag = yyvsp[-4].flag | flMaybeDealloc;
;
    break;}
case 119:
#line 765 "parser.y"
{ LookString(); ;
    break;}
case 120:
#line 769 "parser.y"
{ LookFileName(); ;
    break;}
case 121:
#line 773 "parser.y"
{ LookQString(); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 498 "/usr/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
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
