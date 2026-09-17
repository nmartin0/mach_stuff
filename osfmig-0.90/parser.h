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


extern YYSTYPE yylval;
