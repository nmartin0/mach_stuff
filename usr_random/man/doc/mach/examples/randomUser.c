#include "random.h"
#include <mach/message.h>
#include <mach/mach_types.h>
#include <mach/mig_errors.h>
#include <mach/msg_type.h>
#if	!defined(KERNEL) && !defined(MIG_NO_STRINGS)
#include <strings.h>
#endif
/* LINTLIBRARY */

extern port_t mig_get_reply_port();
extern void mig_dealloc_reply_port();

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

#define msg_request_port	msg_remote_port
#define msg_reply_port		msg_local_port

mig_external void init_random
#if	(defined(__STDC__) || defined(c_plusplus))
	(port_t rep_port)
#else
	(rep_port)
	port_t rep_port;
#endif
{
#ifdef	lint
	rep_port++;
#endif
}

/* Routine get_random */
mig_external kern_return_t get_random
#if	(defined(__STDC__) || defined(c_plusplus))
(
	port_t server_port,
	int *num
)
#else
	(server_port, num)
	port_t server_port;
	int *num;
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

	union {
		Request In;
		Reply Out;
	} Mess;

	register Request *InP = &Mess.In;
	register Reply *OutP = &Mess.Out;

	msg_return_t msg_result;

#if	TypeCheck
	boolean_t msg_simple;
#endif	TypeCheck

	unsigned int msg_size = 24;

#if	UseStaticMsgType
	static msg_type_t RetCodeCheck = {
		/* msg_type_name = */		MSG_TYPE_INTEGER_32,
		/* msg_type_size = */		32,
		/* msg_type_number = */		1,
		/* msg_type_inline = */		TRUE,
		/* msg_type_longform = */	FALSE,
		/* msg_type_deallocate = */	FALSE,
		/* msg_type_unused = */		0
	};
#endif	UseStaticMsgType

#if	UseStaticMsgType
	static msg_type_t numCheck = {
		/* msg_type_name = */		MSG_TYPE_INTEGER_32,
		/* msg_type_size = */		32,
		/* msg_type_number = */		1,
		/* msg_type_inline = */		TRUE,
		/* msg_type_longform = */	FALSE,
		/* msg_type_deallocate = */	FALSE,
		/* msg_type_unused = */		0
	};
#endif	UseStaticMsgType

	InP->Head.msg_simple = TRUE;
	InP->Head.msg_size = msg_size;
	InP->Head.msg_type = MSG_TYPE_NORMAL | MSG_TYPE_RPC;
	InP->Head.msg_request_port = server_port;
	InP->Head.msg_reply_port = mig_get_reply_port();
	InP->Head.msg_id = 500;

	msg_result = msg_rpc(&InP->Head, MSG_OPTION_NONE, sizeof(Reply), 0, 0);
	if (msg_result != RPC_SUCCESS) {
		if (msg_result == RCV_INVALID_PORT)
			mig_dealloc_reply_port();
		return msg_result;
	}

#if	TypeCheck
	msg_size = OutP->Head.msg_size;
	msg_simple = OutP->Head.msg_simple;
#endif	TypeCheck

	if (OutP->Head.msg_id != 600)
		return MIG_REPLY_MISMATCH;

#if	TypeCheck
	if (((msg_size != 40) || (msg_simple != TRUE)) &&
	    ((msg_size != sizeof(death_pill_t)) ||
	     (msg_simple != TRUE) ||
	     (OutP->RetCode == KERN_SUCCESS)))
		return MIG_TYPE_ERROR;
#endif	TypeCheck

#if	TypeCheck
#if	UseStaticMsgType
	if (* (int *) &OutP->RetCodeType != * (int *) &RetCodeCheck)
#else	UseStaticMsgType
	if ((OutP->RetCodeType.msg_type_inline != TRUE) ||
	    (OutP->RetCodeType.msg_type_longform != FALSE) ||
	    (OutP->RetCodeType.msg_type_name != MSG_TYPE_INTEGER_32) ||
	    (OutP->RetCodeType.msg_type_number != 1) ||
	    (OutP->RetCodeType.msg_type_size != 32))
#endif	UseStaticMsgType
		return MIG_TYPE_ERROR;
#endif	TypeCheck

	if (OutP->RetCode != KERN_SUCCESS)
		return OutP->RetCode;

#if	TypeCheck
#if	UseStaticMsgType
	if (* (int *) &OutP->numType != * (int *) &numCheck)
#else	UseStaticMsgType
	if ((OutP->numType.msg_type_inline != TRUE) ||
	    (OutP->numType.msg_type_longform != FALSE) ||
	    (OutP->numType.msg_type_name != MSG_TYPE_INTEGER_32) ||
	    (OutP->numType.msg_type_number != 1) ||
	    (OutP->numType.msg_type_size != 32))
#endif	UseStaticMsgType
		return MIG_TYPE_ERROR;
#endif	TypeCheck

	*num /* num */ = /* *num */ OutP->num;

	return OutP->RetCode;
}

/* Routine get_secret */
mig_external kern_return_t get_secret
#if	(defined(__STDC__) || defined(c_plusplus))
(
	port_t server_port,
	string25 password
)
#else
	(server_port, password)
	port_t server_port;
	string25 password;
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

	union {
		Request In;
		Reply Out;
	} Mess;

	register Request *InP = &Mess.In;
	register Reply *OutP = &Mess.Out;

	msg_return_t msg_result;

#if	TypeCheck
	boolean_t msg_simple;
#endif	TypeCheck

	unsigned int msg_size = 24;

#if	UseStaticMsgType
	static msg_type_t RetCodeCheck = {
		/* msg_type_name = */		MSG_TYPE_INTEGER_32,
		/* msg_type_size = */		32,
		/* msg_type_number = */		1,
		/* msg_type_inline = */		TRUE,
		/* msg_type_longform = */	FALSE,
		/* msg_type_deallocate = */	FALSE,
		/* msg_type_unused = */		0
	};
#endif	UseStaticMsgType

#if	UseStaticMsgType
	static msg_type_t passwordCheck = {
		/* msg_type_name = */		MSG_TYPE_STRING_C,
		/* msg_type_size = */		200,
		/* msg_type_number = */		1,
		/* msg_type_inline = */		TRUE,
		/* msg_type_longform = */	FALSE,
		/* msg_type_deallocate = */	FALSE,
		/* msg_type_unused = */		0
	};
#endif	UseStaticMsgType

	InP->Head.msg_simple = TRUE;
	InP->Head.msg_size = msg_size;
	InP->Head.msg_type = MSG_TYPE_NORMAL | MSG_TYPE_RPC;
	InP->Head.msg_request_port = server_port;
	InP->Head.msg_reply_port = mig_get_reply_port();
	InP->Head.msg_id = 501;

	msg_result = msg_rpc(&InP->Head, MSG_OPTION_NONE, sizeof(Reply), 0, 0);
	if (msg_result != RPC_SUCCESS) {
		if (msg_result == RCV_INVALID_PORT)
			mig_dealloc_reply_port();
		return msg_result;
	}

#if	TypeCheck
	msg_size = OutP->Head.msg_size;
	msg_simple = OutP->Head.msg_simple;
#endif	TypeCheck

	if (OutP->Head.msg_id != 601)
		return MIG_REPLY_MISMATCH;

#if	TypeCheck
	if (((msg_size != 64) || (msg_simple != TRUE)) &&
	    ((msg_size != sizeof(death_pill_t)) ||
	     (msg_simple != TRUE) ||
	     (OutP->RetCode == KERN_SUCCESS)))
		return MIG_TYPE_ERROR;
#endif	TypeCheck

#if	TypeCheck
#if	UseStaticMsgType
	if (* (int *) &OutP->RetCodeType != * (int *) &RetCodeCheck)
#else	UseStaticMsgType
	if ((OutP->RetCodeType.msg_type_inline != TRUE) ||
	    (OutP->RetCodeType.msg_type_longform != FALSE) ||
	    (OutP->RetCodeType.msg_type_name != MSG_TYPE_INTEGER_32) ||
	    (OutP->RetCodeType.msg_type_number != 1) ||
	    (OutP->RetCodeType.msg_type_size != 32))
#endif	UseStaticMsgType
		return MIG_TYPE_ERROR;
#endif	TypeCheck

	if (OutP->RetCode != KERN_SUCCESS)
		return OutP->RetCode;

#if	TypeCheck
#if	UseStaticMsgType
	if (* (int *) &OutP->passwordType != * (int *) &passwordCheck)
#else	UseStaticMsgType
	if ((OutP->passwordType.msg_type_inline != TRUE) ||
	    (OutP->passwordType.msg_type_longform != FALSE) ||
	    (OutP->passwordType.msg_type_name != MSG_TYPE_STRING_C) ||
	    (OutP->passwordType.msg_type_number != 1) ||
	    (OutP->passwordType.msg_type_size != 200))
#endif	UseStaticMsgType
		return MIG_TYPE_ERROR;
#endif	TypeCheck

	(void) mig_strncpy(password /* password */, /* password */ OutP->password, 25);
	password /* password */[24] = '\0';

	return OutP->RetCode;
}
