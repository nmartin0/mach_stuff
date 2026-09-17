/*	pwd.h	4.1	83/05/03	*/

#ifndef _PWD_H_
#define _PWD_H_

struct	passwd { /* see getpwent(3) */
	char	*pw_name;
	char	*pw_passwd;
	int	pw_uid;
	int	pw_gid;
	int	pw_quota;
	char	*pw_comment;
	char	*pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
};

struct passwd *getpwent(), *getpwuid(), *getpwnam();

#endif /* _PWD_H_ */
