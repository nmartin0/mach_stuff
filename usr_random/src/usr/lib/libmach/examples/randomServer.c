/* Module random */

#define EXPORT_BOOLEAN
#include <mach/boolean.h>
#include <mach/message.h>
#include <mach/mig_errors.h>

#ifndef	mig_internal
#define	mig_internal	static
#endif

#ifndef	TypeCheck
#define	TypeCheck 1
#endif

#ifndef	UseExternRCSId
#ifdef	__HC__
#define	UseExternRCSId		1
#endif
#endif

#ifndef	UseStaticMsgType
#if	!defined(__HC__) || defined(__STDC__)
#define	UseStaticMsgType	1
#endif
#endif

/* Due to pcc compiler bug, cannot use void */
#if	(defined(__STDC__) || defined(c_plusplus)) || defined(__HC__)
#define novalue void
#else
#define novalue int
#endif

#define msg_request_port	msg_local_port
#define msg_reply_port		msg_remote_port
#include <mach/std_types.h>
#include "random_types.h"

/* Routine get_random */
mig_internal novalue _Xget_random
#if	(defined(__STDC__) || defined(c_plusplus))
	(msg_header_t *InHeadP, msg_header_t *OutHeadP)
#else
	(InHeadP, OutHeadP)
	msg_header_t *InHeadP, *OutHeadP;
#endif
{
	typedef struct {
		msg_header_t Head;
	} Request;

	typedef struct {
		msg_header_t Head;
		msg_type_t RetCodeType;
		kern_return_t RetCode;
		msg_type_t numType;
		int num;
	} Reply;

	register Request *In0P = (Request *) InHeadP;
	register Reply *OutP = (Reply *) OutHeadP;
	extern kern_return_t get_random
#if	(defined(__STDC__) || defined(c_plusplus))
		(port_t server_port, int *num);
#else
		();
#endif

#if	TypeCheck
	boolean_t msg_simple;
#endif	TypeCheck

	unsigned int msg_size;

#if	UseStaticMsgType
	static msg_type_t numType = {
		/* msg_type_name = */		MSG_TYPE_INTEGER_32,
		/* msg_type_size = */		32,
		/* msg_type_number = */		1,
		/* msg_type_inline = */		TRUE,
		/* msg_type_longform = */	FALSE,
		/* msg_type_deallocate = */	FALSE,
		/* msg_type_unused = */		0
	};
#endif	UseStaticMsgType

#if	TypeCheck
	msg_size = In0P->Head.msg_size;
	msg_simple = In0P->Head.msg_simple;
	if ((msg_size != 24) || (msg_simple != TRUE))
		{ OutP->RetCode = MIG_BAD_ARGUMENTS; return; }
#endif	TypeCheck

	OutP->RetCode = get_random(In0P->Head.msg_request_port, &OutP->num);
#ifdef	label_punt0
#undef	label_punt0
punt0:
#endif	label_punt0
	if (OutP->RetCode != KERN_SUCCESS)
		return;

	msg_size = 40;

#if	UseStaticMsgType
	OutP->numType = numType;
#else	UseStaticMsgType
	OutP->numType.msg_type_name = MSG_TYPE_INTEGER_32;
	OutP->numType.msg_type_size = 32;
	OutP->numType.msg_type_number = 1;
	OutP->numType.msg_type_inline = TRUE;
	OutP->numType.msg_type_longform = FALSE;
	OutP->numType.msg_type_deallocate = FALSE;
	OutP->numType.msg_type_unused = 0;
#endif	UseStaticMsgType

	OutP->Head.msg_simple = TRUE;
	OutP->Head.msg_size = msg_size;
}

/* Routine get_secret */
mig_internal novalue _Xget_secret
#if	(defined(__STDC__) || defined(c_plusplus))
	(msg_header_t *InHeadP, msg_header_t *OutHeadP)
#else
	(InHeadP, OutHeadP)
	msg_header_t *InHeadP, *OutHeadP;
#endif
{
	typedef struct {
		msg_header_t Head;
	} Request;

	typedef struct {
		msg_header_t Head;
		msg_type_t RetCodeType;
		kern_return_t RetCode;
		msg_type_t passwordType;
		string25 password;
		char passwordPad[3];
	} Reply;

	register Request *In0P = (Request *) InHeadP;
	register Reply *OutP = (Reply *) OutHeadP;
	extern kern_return_t get_secret
#if	(defined(__STDC__) || defined(c_plusplus))
		(port_t server_port, string25 password);
#else
		();
#endif

#if	TypeCheck
	boolean_t msg_simple;
#endif	TypeCheck

	unsigned int msg_size;

#if	UseStaticMsgType
	static msg_type_t passwordType = {
		/* msg_type_name = */		MSG_TYPE_STRING_C,
		/* msg_type_size = */		200,
		/* msg_type_number = */		1,
		/* msg_type_inline = */		TRUE,
		/* msg_type_longform = */	FALSE,
		/* msg_type_deallocate = */	FALSE,
		/* msg_type_unused = */		0
	};
#endif	UseStaticMsgType

#if	TypeCheck
	msg_size = In0P->Head.msg_size;
	msg_simple = In0P->Head.msg_simple;
	if ((msg_size != 24) || (msg_simple != TRUE))
		{ OutP->RetCode = MIG_BAD_ARGUMENTS; return; }
#endif	TypeCheck

	OutP->RetCode = get_secret(In0P->Head.msg_request_port, OutP->password);
#ifdef	label_punt0
#undef	label_punt0
punt0:
#endif	label_punt0
	if (OutP->RetCode != KERN_SUCCESS)
		return;

	msg_size = 64;

#if	UseStaticMsgType
	OutP->passwordType = passwordType;
#else	UseStaticMsgType
	OutP->passwordType.msg_type_name = MSG_TYPE_STRING_C;
	OutP->passwordType.msg_type_size = 200;
	OutP->passwordType.msg_type_number = 1;
	OutP->passwordType.msg_type_inline = TRUE;
	OutP->passwordType.msg_type_longform = FALSE;
	OutP->passwordType.msg_type_deallocate = FALSE;
	OutP->passwordType.msg_type_unused = 0;
#endif	UseStaticMsgType

	OutP->Head.msg_simple = TRUE;
	OutP->Head.msg_size = msg_size;
}

boolean_t random_server
#if	(defined(__STDC__) || defined(c_plusplus))
	(msg_header_t *InHeadP, msg_header_t *OutHeadP)
#else
	(InHeadP, OutHeadP)
	msg_header_t *InHeadP, *OutHeadP;
#endif
{
	register msg_header_t *InP =  InHeadP;
	register death_pill_t *OutP = (death_pill_t *) OutHeadP;

#if	UseStaticMsgType
	static msg_type_t RetCodeType = {
		/* msg_type_name = */		MSG_TYPE_INTEGER_32,
		/* msg_type_size = */		32,
		/* msg_type_number = */		1,
		/* msg_type_inline = */		TRUE,
		/* msg_type_longform = */	FALSE,
		/* msg_type_deallocate = */	FALSE,
		/* msg_type_unused = */		0
	};
#endif	UseStaticMsgType

	OutP->Head.msg_simple = TRUE;
	OutP->Head.msg_size = sizeof *OutP;
	OutP->Head.msg_type = InP->msg_type;
	OutP->Head.msg_local_port = PORT_NULL;
	OutP->Head.msg_remote_port = InP->msg_reply_port;
	OutP->Head.msg_id = InP->msg_id + 100;

#if	UseStaticMsgType
	OutP->RetCodeType = RetCodeType;
#else	UseStaticMsgType
	OutP->RetCodeType.msg_type_name = MSG_TYPE_INTEGER_32;
	OutP->RetCodeType.msg_type_size = 32;
	OutP->RetCodeType.msg_type_number = 1;
	OutP->RetCodeType.msg_type_inline = TRUE;
	OutP->RetCodeType.msg_type_longform = FALSE;
	OutP->RetCodeType.msg_type_deallocate = FALSE;
	OutP->RetCodeType.msg_type_unused = 0;
#endif	UseStaticMsgType
	OutP->RetCode = MIG_BAD_ID;

	if ((InP->msg_id > 501) || (InP->msg_id < 500))
		return FALSE;
	else {
		typedef novalue (*SERVER_STUB_PROC)
#if	(defined(__STDC__) || defined(c_plusplus))
			(msg_header_t *, msg_header_t *);
#else
			();
#endif
		static SERVER_STUB_PROC routines[] = {
			_Xget_random,
			_Xget_secret,
		};

		if (routines[InP->msg_id - 500])
			(routines[InP->msg_id - 500]) (InP, &OutP->Head);
		 else
			return FALSE;
	}
	return TRUE;
}
