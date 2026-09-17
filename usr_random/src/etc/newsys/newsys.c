/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

/* newsys.c -- install new systems, update existing systems */

/* By Zonnie L. Williamson, CMU MacMach 1992 */

#define VERSION "1.0"

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <solicit.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <setjmp.h>
#include <signal.h>

#define IS_DIRECTORY 1
#define IS_DISK      2
#define IS_FTP       3

#define installing(dest) (dest->dest_flag == IS_DISK)
#define updating(dest) (dest->dest_flag == IS_DIRECTORY)

char *stars = "\n**************************************************\n";

/* local commands used */
char *bin_cp = "/bin/cp";
char *bin_echo = "/bin/echo";
char *usr_ucb_ftp = "/usr/ucb/ftp";
char *bin_ftp = "/bin/ftp";
char *etc_ifconfig = "/etc/ifconfig";
char *etc_route = "/etc/route";
char *etc_newfs = "/etc/newfs";
char *etc_mklostfound = "/etc/mklost+found";
#ifdef mac2
char *etc_mac2part = "/etc/mac2part";
#endif

/* special files */
char *dot_fixit = "/etc/newconfig/.fixit";
char *dot_modules = "/etc/newconfig/.modules";
char *dot_name = "/etc/newconfig/.name";
char *dot_hostname = "/etc/newconfig/.hostname";
char *usr_bin_fixit = "/usr/bin/fixit";
char *bin_cpio = "/bin/cpio";
char *lib_cpp = "/lib/cpp";
char *fstab = "/etc/newconfig/fstab";
char *source_mount_point = "/tmp/.source";
char *dest_root_mount_point = "/tmp/.dest";
char *dest_usr_mount_point = "/tmp/.dest/usr";
char *raw_dot_fixit = "/usr/tmp/.fixit.raw";
char *tmp_dot_fixit = "/usr/tmp/.fixit";
char *tmp_cpp = "/usr/tmp/cpp";
char *tmp_cpio = "/usr/tmp/cpio";
char *tmp_fixit = "/usr/tmp/fixit";
char *tmp_dot_modules = "/tmp/.modules";
char *tmp_dot_ftp = "/tmp/.ftp";
char *usr_tmp_dot_ftp = "/usr/tmp/.ftp";
char *ftp_flags = "-n -i -v";
char *tmp_dot_copy = "/usr/tmp/.copy";

/* things to clean up */
char *source_disk_mounted;
char *dest_usr_disk_mounted;
char *dest_root_disk_mounted;

jmp_buf restart_here;

int restart_disable;

int restart()
{
  printf("\n\n");
  if (!restart_disable)
    restart_disable = !solicit_yesno("Would you like to start over?");
  longjmp(restart_here, 0);
}

typedef struct {
  int source_flag;
  union {
    char *directory;
    struct {
      char *device;
      char *directory;
    } disk;
    struct {
      char *server;
      char *directory;
      char *user;
      char *password;
      char *tmp_address;
    } ftp;
  } source;
} *source_t;

typedef struct {
  int dest_flag;
  union {
    char *directory;
    char *disk_device;
  } dest;
} *dest_t;

typedef struct {
  char *hostname;
  char *network_address;
  char *gateway_address;
} *name_t;

typedef struct modules {
  char *module;
  struct modules *next;
} *modules_t;

int disable_command;

/* run a system command, return non-zero if error */
int command(char *buffer, char *name)
{
  union wait status;
  int result;

  printf("# %s\n", buffer);
  if (disable_command) return 0;
  status.w_status = system(buffer);
  if (status.w_T.w_Termsig) result = -1;
  else result = status.w_T.w_Retcode;
  fflush(stdout);
  return result;
}

/* return non-zero if file does not exist */
int file_exists(char *file)
{
  struct stat sbuf;

  return stat(file, &sbuf);
}

/* check if a file exists and is a specified type, return -1 if error */
int check_file(char *file, int type)
{
  struct stat sbuf;
  char *what;

  switch (type) {
    case S_IFDIR:
      what = "directory";
      break;
    case S_IFBLK:
      what = "block device";
      break;
    case S_IFREG:
      what = "file";
      break;
    default:
      what = "???";
      break;
  }

  if (disable_command) return 0;

  if (stat(file, &sbuf)) {
    printf("The %s \"%s\" does not exist.\n", what, file);
    return -1;
  }
  if ((sbuf.st_mode & S_IFMT) != type) {
    printf("\"s\" is not a %s.\n", file, what);
    return -1;
  }
  return 0;

} /* check_file() */

/* return destination root */
char *dest_root(dest_t dest)
{
 switch (dest->dest_flag) {
    case IS_DIRECTORY:
      return dest->dest.directory;
      break;
    case IS_DISK:
      return dest_root_mount_point;
      break;
  }
}

/* use cp to copy a file, return non-zero if error */
int cp_file(char *from_file, char *to_file)
{
  char buffer[1024];

  sprintf(buffer, "%s %s %s", bin_cp, from_file, to_file);
  return command(buffer, "cp");
}

/* use ftp to copy a single file, return non-zero if error */
int ftp_file(char *from_file,
             char *to_file,
             char *server,
             char *user,
             char *password)
{
  char buffer[1024];
  struct stat sbuf;
  char *ftp;
  FILE *f;
  int result;

  if (!stat(usr_ucb_ftp, &sbuf)) ftp = usr_ucb_ftp;
  else if (!stat(bin_ftp, &sbuf)) ftp = bin_ftp;
  else {
    printf("Can not find %s or %s.\n", usr_ucb_ftp, bin_ftp);
    return -1;
  }

  if (!(f = fopen(tmp_dot_ftp, "w"))) {
    printf("Can not write \"%s\".\n", tmp_dot_ftp);
    return -1;
  }
  fprintf(f, "user %s %s\n", user, password);
  fprintf(f, "hash\n");
  fprintf(f, "binary\n");
  fprintf(f, "get %s %s\n", from_file, to_file);
  fprintf(f, "bye\n");
  fclose(f);

  sprintf(buffer,
          "%s %s %s <%s",
          ftp,
          ftp_flags,
          server,
          tmp_dot_ftp);
  result = command(buffer, "ftp");

  (void)unlink(tmp_dot_ftp);

  return result;

} /* ftp_file() */

/* copy a file from source to destination, return non-zero if error */
int copy_file(char *from_file, char *to_file, source_t source, dest_t dest)
{
  char from[128], to[128];
  int result;
  char *directory, *file;

  if (!dest) strcpy(to, to_file);
  else sprintf(to, "%s/%s", dest_root(dest), to_file);

  if (!source) {
    strcpy(from, from_file);
    result = cp_file(from, to);
  }
  else switch (source->source_flag) {
    case IS_DIRECTORY:
      file = from_file;
      while (*file && (*file == '/')) file++;
      sprintf(from, "%s/%s", source->source.directory, file);
      result = cp_file(from, to);
      break;
    case IS_DISK:
      directory = source->source.disk.directory;
      while (*directory && (*directory == '/')) directory++;
      file = from_file;
      while (*file && (*file == '/')) file++;
      sprintf(from,
              "%s/%s%s%s",
              source_mount_point,
              directory, *directory ? "/" : "",
              file);
      result = cp_file(from, to);
      break;
    case IS_FTP:
      file = from_file;
      while (*file && (*file == '/')) file++;
      sprintf(from, "%s/%s", source->source.ftp.directory, file);
      result = ftp_file(from,
                        to,
                        source->source.ftp.server,
                        source->source.ftp.user,
                        source->source.ftp.password);
  }
  if (result) printf("Can not copy file \"%s\" to \"%s\".\n", from, to);
  return result;
}

/* use ftp to copy a list of files, return non-zero if error */
int ftp_files(char *list_file,
              char *from_root,
              char *to_root,
              char *server,
              char *user,
              char *password)
{
  char buffer[1024];
  struct stat sbuf;
  char *ftp;
  FILE *f, *list;
  int result;

  if (!stat(usr_ucb_ftp, &sbuf)) ftp = usr_ucb_ftp;
  else if (!stat(bin_ftp, &sbuf)) ftp = bin_ftp;
  else {
    printf("Can not find %s or %s.\n", usr_ucb_ftp, bin_ftp);
    return -1;
  }

  /* files in the list must not begin with '/' */
  if (!(list = fopen(list_file, "r"))) {
    printf("Can not read \"%s\".\n", list_file);
    return -1;
  }
  if (!(f = fopen(usr_tmp_dot_ftp, "w"))) {
    printf("Can not write \"%s\".\n", usr_tmp_dot_ftp);
    return -1;
  }
  fprintf(f, "user %s %s\n", user, password);
  fprintf(f, "hash\n");
  fprintf(f, "binary\n");
  while (fgets(buffer, sizeof(buffer), list)) {
    buffer[strlen(buffer) - 1] = 0;
    fprintf(f, "get %s/%s %s/%s\n", from_root, buffer, to_root, buffer);
  }
  fprintf(f, "bye\n");
  fclose(f);
  fclose(list);

  sprintf(buffer,
          "%s %s %s <%s",
          ftp,
          ftp_flags,
          server,
          usr_tmp_dot_ftp);
  result = command(buffer, "ftp");

  (void)unlink(usr_tmp_dot_ftp);

  return result;

} /* ftp_files() */

/* use cpio to copy a list of files, return non-zero if error */
int cpio_files(char *list_file, char *from_root, char *to_root)
{
  char buffer[1024];

  /* files in the list must not begin with '/' */

  sprintf(buffer,
          "cd %s; %s -pduvm %s <%s",
          from_root,
          tmp_cpio,
          to_root,
          list_file);
  return command(buffer, "cpio");
}

/* copy files from source to destination, return non-zero if error */
int copy_files(char *list_file, source_t source, dest_t dest)
{
  char source_root[128];
  int result;

  switch(source->source_flag) {
    case IS_DIRECTORY:
      strcpy(source_root, source->source.directory);
      result = cpio_files(list_file, source_root, dest_root(dest));
      break;
    case IS_DISK:
      sprintf(source_root,
              "%s/%s",
              source_mount_point,
              source->source.disk.directory);
      result = cpio_files(list_file, source_root, dest_root(dest));
      break;
    case IS_FTP:
      strcpy(source_root, source->source.ftp.directory);
      result = ftp_files(list_file,
                         source_root,
                         dest_root(dest),
                         source->source.ftp.server,
                         source->source.ftp.user,
                         source->source.ftp.password);
  }
  if (result) printf("Can not copy files to \"%s\".\n", dest_root(dest));
  return result;
}

/* run fixit to build list of files to copy
 * returns 0 if no files to copy
 * returns 1 if there are files to copy
 * returns non-zero if error
 */
int fixit(dest_t dest, char *fixit_file, char *list_file)
{
  char buffer[1024];

  sprintf(buffer,
          "%s -fix %s <%s >%s",
          tmp_fixit,
          dest_root(dest),
          fixit_file,
          list_file);
  return command(buffer, "fixit");
}

/* do a chmod of a file, return non-zero if error */
int chmod_file(char *file, int mode)
{
  printf("# chmod 0%o %s\n", mode, file);
  if (disable_command) return 0;
  if (chmod(file, mode)) {
    printf("Can not chmod \"%s\".\n", file);
    return -1;
  }
  return 0;
}

/* make a directory, return non-zero if error */
int make_directory(char *dir, int mode)
{
  printf("# mkdir %s; chmod 0%o %s\n", dir, mode, dir);
  if (disable_command) return 0;
  if (mkdir(dir, mode)) {
    printf("Can not make directory \"%s\".\n", dir);
    return -1;
  }
  return 0;
}

/* mount a file system, return non-zero if error */
int mount_disk(char *device, char *root, int read_only)
{
  printf("# mount %s%s %s\n", read_only ? "-r " : "", device, root);
  if (disable_command) return 0;
  if (mount(device, root, read_only)) {
    printf("Can not mount \"%s\".\n", device);
    return -1;
  }
  return 0;
}

/* check if the network is up, return non-zero if it is */
int network_up()
{
  struct ifreq ifr;
  int s;

  if (disable_command) return 0;

  bzero(&ifr, sizeof(ifr));
  strncpy(ifr.ifr_name, "en0", sizeof(ifr.ifr_name));

  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
    ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr);
    close(s);
  }

  return ifr.ifr_flags & IFF_UP;

}

/* check that the network is up, return non-zero if error */
int check_network(char *network_address, char *gateway_address)
{
  char buffer[1024];

  if (!network_up()) {
    printf("Starting network access.\n");
    fflush(stdout);
    sprintf(buffer,
            "%s en0 %s up arp -trailers",
            etc_ifconfig,
            network_address);
    if (command(buffer, "ifconfig")) return -1;
  }
  if (gateway_address) {
    sprintf(buffer,
            "%s -f -n add 0 %s 1",
            etc_route,
            gateway_address);
    if (command(buffer, "route")) return -1;
  }
  return 0;
}

/* verify newsys parameters, returns non-zero if error */
int verify(source_t source, dest_t dest, name_t name, modules_t modules)
{
  char question[128];
  modules_t m;

  puts(stars);
  printf("Ready to %s \"%s\".\n",
         installing(dest) ? "install" : "update",
         name->hostname);

  if (!strcmp(name->network_address, "0")) {
    printf("There is no external network access.\n");
  }
  else {
    printf("The network address is \"%s\".\n", name->network_address);
    if (!strcmp(name->gateway_address, "0"))
      printf("There is no gateway.\n");
    else printf("The gateway address is \"%s\";\n", name->gateway_address);
  }

  if (source->source_flag == IS_DIRECTORY)
    printf("The source directory is \"%s\".\n", source->source.directory);
  else if (source->source_flag == IS_DISK)
    printf("The source disk is \"%s\".\n", source->source.disk.device);
  else {
    printf("The source ftp server is \"%s\".\n", source->source.ftp.server);
    printf("The ftp directory is \"%s\".\n", source->source.ftp.directory);
    printf("The ftp user is \"%s\".\n", source->source.ftp.user);
    if (source->source.ftp.tmp_address)
      printf("The temporary address is \"%s\".\n",
             source->source.ftp.tmp_address);
  }

  if (dest->dest_flag == IS_DIRECTORY)
    printf("The destination directory is \"%s\".\n", dest->dest.directory);
  else
    printf("The destination disk is \"%s\".\n", dest->dest.disk_device);

  printf("The modules are:");
  for (m = modules; m; m = m->next) printf(" %s", m->module);
  printf("\n");

  if (installing(dest)) {
    printf("WARNING: Any existing Mach partition will be erased!\n");
    strcpy(question, "Ok to install?");
  }
  else strcpy(question, "Ok to update?");
  return !solicit_yesno(question);

}

/* do installation things */
int install(dest_t dest)
{
  char buffer[1024], usrdisk[128];

#ifdef mac2
  /* for MacMach, run mac2part to setup Mach_UNIX_BSD4.3 partition */
  sprintf(buffer, "%s -mach %s", etc_mac2part, dest->dest.disk_device);
  if (command(buffer, "mac2part")) return -1;
#endif /* mac2 */

  /* XXX set up disklabel here XXX */

  /* run newfs to create the root file system */
  sprintf(buffer, "%s %s", etc_newfs, dest->dest.disk_device);
  if (command(buffer, "newfs")) return -1;

  /* insure that the destination mount point exists */
  if (file_exists(dest_root_mount_point)) {
    if (make_directory(dest_root_mount_point, 0700)) return -1;
  }
  else {
    if (check_file(dest_root_mount_point, S_IFDIR)) return -1;
    if (chmod_file(dest_root_mount_point, 0700)) return -1;
  }

  /* mount the root disk */
  if (mount_disk(dest->dest.disk_device,
                   dest_root_mount_point,
                   0)) return -1;
  dest_root_disk_mounted = dest->dest.disk_device;

  /* run mklost+found */
  sprintf(buffer, "cd %s; %s", dest_root_mount_point, etc_mklostfound);
  if (command(buffer, "mklost+found")) return -1;

  /* make usr directory */
  if (make_directory(dest_usr_mount_point, 0700)) return -1;

  /* compose device name for 'g' partition */
  strcpy(usrdisk, dest->dest.disk_device);
  usrdisk[strlen(usrdisk) - 1] = 'g';

  /* run newfs to create the usr file system on 'g' partition */
  sprintf(buffer, "%s %s", etc_newfs, usrdisk);
  if (command(buffer, "newfs")) return -1;

  /* mount the usr disk */
  if (mount_disk(usrdisk, dest_usr_mount_point, 0)) return -1;
  dest_usr_disk_mounted = usrdisk;

  /* run mklost+found */
  sprintf(buffer, "cd %s; %s", dest_usr_mount_point, etc_mklostfound);
  if (command(buffer, "mklost+found")) return -1;

  /* make /tmp and /usr/tmp */
  sprintf(buffer, "%s/tmp", dest_root_mount_point);
  if (make_directory(buffer, 0777)) return -1;
  sprintf(buffer, "%s/tmp", dest_usr_mount_point);
  if (make_directory(buffer, 0777)) return -1;

  /* all done, no error */
  return 0;

} /* install() */

/* do update or install, returns non-zero if error */
int newsys(source_t source, dest_t dest, name_t name, modules_t modules)
{
  char buffer[1024],usrdisk[128];
  FILE *f;
  modules_t m;
  int result;

  if (verify(source, dest, name, modules)) restart();

  /* restart not allowed from here on */
  restart_disable = 1;

  puts(stars);

  /* if installing, do installation stuff */
  if (installing(dest) && install(dest)) return -1;

  /* the network was started in setup_source() */

  /* If we are running on a ramdisk, the /usr/tmp is a symbolic
   * link to the /usr/tmp of the destination disk, which is
   * already mounted.  In short, we have space on /usr/tmp.
   */

  /* get raw fixit list, fixit, cpp and maybe cpio */
  if (copy_file(dot_fixit, raw_dot_fixit, source, 0)) return -1;
  if (copy_file(usr_bin_fixit, tmp_fixit, source, 0)) return -1;
  if (copy_file(lib_cpp, tmp_cpp, source, 0)) return -1;
  if (source->source_flag != IS_FTP)
    if (copy_file(bin_cpio, tmp_cpio, source, 0)) return -1;

  /* make fixit, cpp and cpio executable */
  if (chmod_file(tmp_fixit, 0700)) return -1;
  if (chmod_file(tmp_cpp, 0700)) return -1;
  if (!file_exists(tmp_cpio) && chmod_file(tmp_cpio, 0700)) return -1;

  /* use cpp to filter the fixit list for the specified modules */
  sprintf(buffer, "%s -undef", tmp_cpp);
  for (m = modules; m; m = m->next) {
    strcat(buffer, " -D");
    strcat(buffer, m->module);
  }
  strcat(buffer, " <");
  strcat(buffer, raw_dot_fixit);
  strcat(buffer, " >");
  strcat(buffer, tmp_dot_fixit);
  if (command(buffer, "cpp")) return -1;

  puts(stars);

  /* run fixit to create file list, then copy files */
  while ((result = fixit(dest, tmp_dot_fixit, tmp_dot_copy)) == 1)
    if (copy_files(tmp_dot_copy, source, dest)) return -1;
  if (result) return -1;
  printf("All system files are correct and in place.\n");

  puts(stars);

  /* create system name file */
  printf("Creating: /etc/newconfig/.name\n");
  sprintf(buffer, "%s/%s", dest_root(dest), dot_name);
  if (!(f = fopen(buffer, "w"))) {
    printf("Can not write \"%s\".\n", buffer);
    return -1;
  }
  fprintf(f,
          "%s:%s:%s\n",
          name->hostname,
          name->network_address,
          name->gateway_address);
  fclose(f);

  /* copy in the fixit list */
  printf("Creating: /etc/newconfig/.fixit\n");
  if (copy_file(raw_dot_fixit, dot_fixit, 0, dest)) return -1;

  /* create the modules file */
  printf("Creating: /etc/newconfig/.modules\n");
  sprintf(buffer, "%s%s", dest_root(dest), dot_modules);
  if (!(f = fopen(buffer, "w"))) {
    printf("Can not write \"%s\".\n", buffer);
    return -1;
  }
  for (m = modules; m; m = m->next)
    fprintf(f, "%s%s", m->module, m->next ? ":" : "");
  fprintf(f, "\n");
  fclose(f);

  /* if installing, create the fstab file */
  printf("Creating: /etc/fstab\n");
  if (installing(dest)) {
    sprintf(buffer, "%s/%s", dest_root(dest), fstab);
    if (!(f = fopen(buffer, "w"))) {
      printf("Can not write \"%s\".\n", buffer);
      return -1;
    }
    fprintf(f, "%s:/:rw:0:1\n", dest->dest.disk_device);
    strcpy(usrdisk, dest->dest.disk_device);
    usrdisk[strlen(usrdisk) - 1] = 'g';
    fprintf(f, "%s:/usr:rw:0:2\n", usrdisk);
    fclose(f);
  }

  /* create the hostname file */
  printf("Creating: /.hostname\n");
  sprintf(buffer, "%s/%s", dest_root(dest), dot_hostname);
  if (!(f = fopen(buffer, "w"))) {
    printf("Can not write \"%s\".\n", buffer);
    return -1;
  }
  fprintf(f, "HOSTNAME=\"%s\"\n", name->hostname);
  if (!strcmp(name->network_address, "0")) fprintf(f, "ADDRESS=\"\"\n");
  else fprintf(f, "ADDRESS=\"%s\"\n", name->network_address);
  if (!strcmp(name->gateway_address, "0")) fprintf(f, "GATEWAY=\"\"\n");
  else fprintf(f, "GATEWAY=\"%s\"\n", name->gateway_address);
  fclose(f);

  puts(stars);

  /* if install/update the configuration files */
  printf("Installing configuration files...\n");
  sprintf(buffer,
          "\
PATH=\"%s/bin:%s/usr/bin:%s/usr/ucb:%s/etc\"; \
export PATH; \
cd %s/etc/newconfig; \
make INSTALL_TREE=%s install\
          ",
          dest_root(dest),
          dest_root(dest),
          dest_root(dest),
          dest_root(dest),
          dest_root(dest),
          dest_root(dest));
  if (command(buffer, "make")) return -1;

  puts(stars);

  /* all done, no error */
  printf("%s is complete\n", installing(dest) ? "Installation" : "Update");
  return 0;

} /* newsys() */

/* create list of valid disks to select from */
char *possible_disks(char partition)
{
  char buffer[1024], disk[128], *tmp, block[512];
  int i;
  FILE *f;

  *buffer = 0;
  for (i = 0; i < 7; i++) {
#ifdef mac2
    sprintf(disk, "/dev/macdisk%d", i);
#endif
    if (f = fopen(disk, "r")) {
      if (fread(block, sizeof(block), 1, f) == 1)
        sprintf(disk, "/dev/disk%d%c", i, partition);
        add_item(buffer, disk);
      fclose(f);
    }
  }
  if (!*buffer) return 0;

  tmp = (char *)malloc(strlen(buffer) + 1);
  strcpy(tmp, buffer);
  return tmp;
}

/* setup the source, returns zero if error */
source_t setup_source(dest_t dest, name_t name, char *source_arg)
{
  source_t tmp = (source_t)malloc(sizeof(*tmp));
  char buffer[1024], *s;

  /* if source argument not specified, solicit source */
  if (!source_arg) {
    s = solicit_item("DIRECTORY:DISK:FTP", "Select source type.",
    "The source is where the system is installed from.");
    if (!s) return 0;
    if (!strcmp(s, "DIRECTORY")) {
      tmp->source_flag = IS_DIRECTORY;
      tmp->source.directory = 0;
    }
    else if (!strcmp(s, "DISK")) {
      tmp->source_flag = IS_DISK;
      tmp->source.disk.device = 0;
      tmp->source.disk.directory = 0;
    }
    else {
      tmp->source_flag = IS_FTP;
      tmp->source.ftp.server = 0;
      tmp->source.ftp.directory = 0;
      tmp->source.ftp.user = 0;
      tmp->source.ftp.password = 0;
      tmp->source.ftp.tmp_address = 0;
    }
  }
  else {

    if (*source_arg != '/') {

      /* source argument specifies ftp server */
      tmp->source_flag = IS_FTP;

      s = source_arg;
      if (source_arg = index(source_arg, ':')) *source_arg++ = 0;
      tmp->source.ftp.server = (char *)malloc(strlen(s) + 1);
      strcpy(tmp->source.ftp.server, s);

      /* pick up :<directory> */
      if (source_arg) {
        s = source_arg;
        if (source_arg = index(source_arg, ':')) *source_arg++ = 0;
        tmp->source.ftp.directory = (char *)malloc(strlen(s) + 1);
        strcpy(tmp->source.ftp.directory, s);
      }
      else tmp->source.ftp.directory = 0;

      /* pick up :<user> */
      if (source_arg) {
        s = source_arg;
        if (source_arg = index(source_arg, ':')) *source_arg++ = 0;
        tmp->source.ftp.user = (char *)malloc(strlen(s) + 1);
        strcpy(tmp->source.ftp.user, s);
      }
      else tmp->source.ftp.user = 0;

      /* pick up :<password> */
      if (source_arg) {
        s = source_arg;
        if (source_arg = index(source_arg, ':')) *source_arg++ = 0;
        tmp->source.ftp.password = (char *)malloc(strlen(s) + 1);
        strcpy(tmp->source.ftp.password, s);
      }
      else tmp->source.ftp.password = 0;

      /* pick up :<tmp address> */
      if (source_arg) {
        s = source_arg;
        if (source_arg = index(source_arg, ':')) *source_arg++ = 0;
        tmp->source.ftp.tmp_address = (char *)malloc(strlen(s) + 1);
        strcpy(tmp->source.ftp.tmp_address, s);
      }
      else tmp->source.ftp.tmp_address = 0;

    }
    else if (!strncmp(source_arg, "/dev/", 5)) {

      /* if source argument begins "/dev/" then it is a disk */
      tmp->source_flag = IS_DISK;
      s = source_arg;
      if (source_arg = index(source_arg, ':')) *source_arg++ = 0;
      tmp->source.disk.device = (char *)malloc(strlen(s) + 1);
      strcpy(tmp->source.disk.device, s);

      /* pick up :<directory> */
      if (source_arg) {
        s = source_arg;
        if (source_arg = index(source_arg, ':')) *source_arg++ = 0;
        tmp->source.disk.directory = (char *)malloc(strlen(s) + 1);
        strcpy(tmp->source.disk.directory, s);
      }

    }
    else {

        /* source argument specifies a directory */
        tmp->source_flag = IS_DIRECTORY;
        tmp->source.disk.device = (char *)malloc(strlen(source_arg) + 1);
        strcpy(tmp->source.disk.device, source_arg);

    }
  }

  /* solicit/verify disk parameters */
  if (tmp->source_flag == IS_DISK) {

    /* solicit disk device if not yet specified */
    if (!tmp->source.disk.device) {
      tmp->source.disk.device = solicit_item(possible_disks('c'),
                                             "Select a source disk.",
      "The source disk is where the new system will be installed from.");
      if (!tmp->source.disk.device) return 0;
    }

    /* verify that the disk device exists and is a block device */
    if (check_file(tmp->source.disk.device, S_IFBLK)) return 0;

    /* the disk device should be a 'c' partition */
    if (tmp->source.disk.device[strlen(tmp->source.disk.device) - 1] != 'c') {
      printf("Source disk \"%s\" is not the 'c' partition.\n",
             tmp->source.disk.device);
      printf("The 'c' partition is usually appropriate here.\n");
      if (!solicit_yesno("Are you sure that you want to use this?")) return 0;
    }

    /* insure that the source mount point exists */
    if (file_exists(source_mount_point)) {
      if (make_directory(source_mount_point, 0700)) return 0;
    }
    else {
      if (check_file(source_mount_point, S_IFDIR)) return 0;
      if (chmod_file(source_mount_point, 0700)) return 0;
    }

    /* mount the disk device */
    if (mount_disk(tmp->source.disk.device, source_mount_point, 1)) {
      printf("Can not mount disk device \"%s\".\n", tmp->source.disk.device);
      return 0;
    }
    source_disk_mounted = tmp->source.disk.device;

    /* solicit the disk directory if not yet specified */
    if (!tmp->source.disk.directory) {
      tmp->source.disk.directory =
         solicit_text("Enter the source directory:", 0,
      "The source directory is where the new system is installed from.");
      if (!tmp->source.disk.directory) return 0;
    }

    /* verify that the disk directory exists and is a directory */
    sprintf(buffer, "%s/%s", source_mount_point, tmp->source.disk.directory);
    if (check_file(buffer, S_IFDIR)) return 0;

    /* copy in the modules file */
    if (copy_file(dot_modules, tmp_dot_modules, tmp, 0)) return 0;

    /* all done, return pointer to source structure */
    return tmp;

  }

  /* solicit/verify directory parameters */
  if (tmp->source_flag == IS_DIRECTORY) {

    /* solicit the source directory if not yet specified */
    if (!tmp->source.directory) {
      tmp->source.directory = solicit_text("Enter the source directory:", 0,
      "The source directory is where the new system is installed from.");
      if (!tmp->source.directory) return 0;
    }

    /* verify that the source directory exists and is a directory */
    if (check_file(tmp->source.directory, S_IFDIR)) return 0;

    /* copy in the modules file */
    if (copy_file(dot_modules, tmp_dot_modules, tmp, 0)) return 0;

    /* all done, return pointer to source structure */
    return tmp;

  }

  /* solicit the ftp server if not yet specified */
  if (!tmp->source.ftp.server) {
    tmp->source.ftp.server = solicit_text("Enter the address of the ftp server:", 0,
    "The ftp server address can be a name or a network address.");
    if (!tmp->source.ftp.server) return 0;
  }
  /* solicit the ftp directory if not yet specified */
  if (!tmp->source.ftp.directory) {
    tmp->source.ftp.directory = solicit_text("Enter the source directory:", 0,
    "The directory is where the system is kept on the ftp server.\n");
  }
  /* solicit the ftp user if not yet specified */
  if (!tmp->source.ftp.user) {
    tmp->source.ftp.user = solicit_text("Enter the ftp user:", 0,
    "The user id is used for the ftp logon.");
    if (!tmp->source.ftp.user) return 0;
  }
  /* solicit the ftp password if not yet specified */
  if (!tmp->source.ftp.password) {
    tmp->source.ftp.password = solicit_password("Enter the ftp password:",
    "The password is used for the ftp logon.");
    if (!tmp->source.ftp.password) return 0;
  }

  /* solicit the temporary address if needed and not yet specified */
  if (!strcmp(name->network_address, "0") && !network_up()) {
    if (!tmp->source.ftp.tmp_address) {
      tmp->source.ftp.tmp_address = solicit_text("Enter a temporary network address:", 0,
      "The temporary address is needed for the ftp operation.");
    }
  }

  /* make sure that the network is up */
  if (check_network(strcmp(name->network_address, "0") ?
                      name->network_address :
                      tmp->source.ftp.tmp_address,
                    strcmp(name->gateway_address, "0") ?
                      name->gateway_address :
                      0)) return 0;

  /* copy in the modules file  */
  /* this verifies that the network/ftp parameters are usable */
  if (copy_file(dot_modules, tmp_dot_modules, tmp, 0)) return 0;

  /* all done, return pointer to source structure */
  return tmp;

} /* setup_source() */

/* setup the dest, returns zero if error */
dest_t setup_dest(char *dest_arg)
{
  dest_t tmp = (dest_t)malloc(sizeof(*tmp));
  char buffer[128], *d;

  /* if destination argument not specified, solicit destination */
  if (!dest_arg) {
    d = solicit_item("INSTALL:UPDATE", "Select operation.",
    "INSTALL a new system or UPDATE an existing system.");
    if (!d) return 0;
    if (!strcmp(d, "INSTALL")) {
      tmp->dest_flag = IS_DISK;
      tmp->dest.disk_device = 0;
    }
    else {
      tmp->dest_flag = IS_DIRECTORY;
      tmp->dest.directory = 0;
    }
  }
  else if (!strcmp(dest_arg, "-install")) {
      tmp->dest_flag = IS_DISK;
      tmp->dest.disk_device = 0;
  }
  else if (!strcmp(dest_arg, "-update")) {
      tmp->dest_flag = IS_DIRECTORY;
      tmp->dest.directory = 0;
  }
  else {

    if (*dest_arg != '/') {
      printf("Invalid destination \"%s\".\n", dest_arg);
      return 0;
    }

    if (!strncmp(dest_arg, "/dev/", 5)) {

      /* if destination argument begins "/dev/" then it is a disk */
      tmp->dest_flag = IS_DISK;

      tmp->dest.disk_device = (char *)malloc(strlen(dest_arg) + 1);
      strcpy(tmp->dest.disk_device, dest_arg);

    }
    else {

       /* destination argument specifies a directory */
      tmp->dest.directory = (char *)malloc(strlen(dest_arg) + 1);
      strcpy(tmp->dest.directory, dest_arg);

    }
  }

  /* solicit/verify disk parameters */
  if (tmp->dest_flag == IS_DISK) {

    /* solicit disk device if not yet specified */
    if (!tmp->dest.disk_device) {
      tmp->dest.disk_device = solicit_item(possible_disks('a'),
                                           "Select a destination disk.",
      "The destination disk is where the new system will be installed.");
      if (!tmp->dest.disk_device) return 0;
    }

    /* verify that the disk device exists and is a block device */
    if (check_file(tmp->dest.disk_device, S_IFBLK)) return 0;

    /* the disk device must be the 'a' partition */
    if (tmp->dest.disk_device[strlen(tmp->dest.disk_device) - 1] != 'a') {
      printf("Destination disk \"%s\" is not the 'a' partition.\n",
             tmp->dest.disk_device);
      return 0;
    }

    /* all done, return pointer to dest structure */
    return tmp;

  }

  /* solicit/verify directory parameter */

  /* solicit directory if not yet specified */
  if (!tmp->dest.directory) {
    tmp->dest.directory = solicit_text("Enter destination directory:", 0,
    "The destination directory is where the system to be updated is.");
    if (!tmp->dest.directory) return 0;
  }

  /* verify that the destination directory exists and is a directory */
  if (check_file(tmp->dest.directory, S_IFDIR)) return 0;

  /* all done, return pointer to dest structure */
  return tmp;

} /* setup_dest() */

/* setup the name, returns zero if error */
name_t setup_name(dest_t dest, char *name_arg)
{
  char buffer[128], *n;
  FILE *f;
  name_t tmp;
  int access;

  tmp = (name_t)malloc(sizeof(*tmp));

  /* if name not specified and updating, try to use previous name */
  if (!name_arg && updating(dest)) {
    sprintf(buffer, "%s/etc/newconfig/.name", dest->dest.directory);
    if (f = fopen(buffer, "r")) {
      if (fgets(buffer, sizeof(buffer), f)) buffer[strlen(buffer) - 1] = 0;
      else buffer[0] = 0;
      close(f);
      name_arg = buffer;
    }
  }

  /* try to extract the host name */
  if (name_arg) {
    n = name_arg;
    if (name_arg = index(name_arg, ':')) *name_arg++ = 0;
    tmp->hostname = (char *)malloc(strlen(n) + 1);
    strcpy(tmp->hostname, n);
  }
  else tmp->hostname = 0;

  /* try to extract the network address */
  if (name_arg) {
    n = name_arg;
    if (name_arg = index(name_arg, ':')) *name_arg++ = 0;
    tmp->network_address = (char *)malloc(strlen(n) + 1);
    strcpy(tmp->network_address, n);
  }
  else tmp->network_address = 0;

  /* try to extract the gateway address */
  if (name_arg) {
    n = name_arg;
    if (name_arg = index(name_arg, ':')) *name_arg++ = 0;
    tmp->gateway_address = (char *)malloc(strlen(n) + 1);
    strcpy(tmp->gateway_address, n);
  }
  else tmp->gateway_address = 0;

  /* decide if the system will have network access */
  if (!tmp->hostname || !tmp->network_address || !tmp->gateway_address)
    access = solicit_yesno("Will this system have network access?");
  else access = strcmp(tmp->network_address, "0");

  /* if it has not been specified, solicit the host name */
  if (!tmp->hostname) {
    if (access) tmp->hostname = solicit_text("Enter the host name:", 0,
      "The host name is allocated by the local network administrator.");
    else tmp->hostname = solicit_text("Enter the system name:", 0,
      "The system name is the system's name.");
  }
  if (!tmp->hostname) return 0;

  /* if it has not been specified and is needed, solicit the network address */
  if (!tmp->network_address && !access) tmp->network_address = "0";
  else if (!tmp->network_address) {
    tmp->network_address = solicit_text("Enter the network address:", 0,
    "The network address is allocated by the local network administrator.");
  }
  if (!tmp->network_address) return 0;

  /* if it has not been specified and is needed, solicit the gateway address */
  if (!tmp->gateway_address) {
    if (!access) tmp->gateway_address = "0";
    else if (!solicit_yesno("Is there a gateway?")) tmp->gateway_address = "0";
    else if (!tmp->gateway_address) {
      tmp->gateway_address = solicit_text("Enter the gateway address: ", 0,
      "The gateway address is allocated by the local network administrator.");
    }
  }
  if (!tmp->gateway_address) return 0;

  /* all done, return pointer to name structure */
  return tmp;

} /* setup_name() */

/* create list of valid modules to select from */
char *possible_modules(source_t source)
{
  static char buffer[1024];
  FILE *f;

  /* open modules file */
  if (!(f = fopen(tmp_dot_modules, "r"))) {
    printf("Can not read modules file \"%s\".\n", tmp_dot_modules);
    return 0;
  }

  /* read modules list from modules file */
  if (fgets(buffer, sizeof(buffer), f)) buffer[strlen(buffer) - 1] = 0;
  else buffer[0] = 0;

  /* close modules file */
  fclose(f);

  /* all done, return modules list */
  return *buffer ? buffer : 0;

} /* possible_modules() */

/* setup the modules, returns zero if error */
modules_t setup_modules(dest_t dest, source_t source, char **modules_arg)
{
  modules_t list, tmp;
  char buffer[1024], module[128], *modules;
  FILE *f;
  int i;

  list = 0;

  /* if updating and modules argument not specified,
   * try to get previous list of modules
   */
  if (updating(dest) && !modules_arg) {
    sprintf(buffer, "%s/etc/newconfig/.modules", dest->dest.directory);
    if (f = fopen(buffer, "r")) {
      if (fgets(buffer, sizeof(buffer), f)) {
        buffer[strlen(buffer) - 1] = 0;
        for (i = 0; !select_item(buffer, module, i); i++) {
          tmp = (modules_t)malloc(sizeof(*tmp));
          tmp->module = (char *)malloc(strlen(module) + 1);
          strcpy(tmp->module, module);
          tmp->next = list;
          list = tmp;
        }
        if (list) {
          printf("Updating modules: %s\n", buffer);
          return list;
        }
      }
      fclose(f);
    }
  }

  /* if modules argument specified, create list */
  if (modules_arg) {
    while (*modules_arg) {
      tmp = (modules_t)malloc(sizeof(*tmp));
      tmp->module = (char *)malloc(strlen(*modules_arg) + 1);
      strcpy(tmp->module, *modules_arg);
      tmp->next = list;
      list = tmp;
      modules_arg++;
    }
    return list;
  }

  /* solicit modules if none have been specified */
  modules = solicit_items(possible_modules(source),
                          "Select the modules to install.",
  "These are the modules that will make up the installed system.");
  if (!modules) return 0;
  for (i = 0; !select_item(modules, module, i); i++) {
    tmp = (modules_t)malloc(sizeof(*tmp));
    tmp->module = (char *)malloc(strlen(module) + 1);
    strcpy(tmp->module, module);
    tmp->next = list;
    list = tmp;
  }
  return list;

} /* setup_modules() */

main(int argc, char **argv)
{
  char *source_arg, *dest_arg, *name_arg, **modules_arg;
  source_t source;
  dest_t dest;
  name_t name;
  modules_t modules;
  int result;
  int restart_argc;
  char **restart_argv;

  restart_argc = argc;
  restart_argv = argv;

  /* setup restart if interrupted */
  (void)setjmp(restart_here);
  if (!restart_disable) {

    signal(SIGINT, restart);

    argc = restart_argc;
    argv = restart_argv;

    /* skip the program name */
    argc--;
    argv++;

    puts(stars);

    /* announce "newsys" */
    printf("newsys, system installer/updater, version %s\n", VERSION);

    puts(stars);

    puts("[ Enter \"?\" at any prompt to get additional help. ]\n");

    /* optional -n to keep commands from actually being executed */
    if (argc && (!strcmp(*argv, "-n"))) {
      disable_command++;
      argc--;
      argv++;
    }

    /* the first argument is the destination */
    if (argc) {
      dest_arg = *argv++;
      argc--;
    }
    else dest_arg = 0;

    /* the second argument is the name */
    if (argc) {
      name_arg = *argv++;
      argc--;
    }
    else name_arg = 0;

    /* the third argument is the source */
    if (argc) {
      source_arg = *argv++;
      argc--;
    }
    else source_arg = 0;

    /* the remaining arguments are the modules */
    if (argc) modules_arg = argv;
    else modules_arg = 0;

    /* setup the destination argument */
    if (!(dest = setup_dest(dest_arg))) {
      printf("Could not setup destination.\n");
      exit(1);
    }

    /* setup the name argument */
    if (!(name = setup_name(dest, name_arg))) {
      printf("Could not setup name.\n");
      exit(1);
    }

    /* setup the source argument */
    if (!(source = setup_source(dest, name, source_arg))) {
      printf("Could not setup source.\n");
      exit(1);
    }

    /* setup the modules argument */
    if (!(modules = setup_modules(dest, source, modules_arg))) {
      printf("Could not setup modules.\n");
      exit(1);
    }

    /* now that the arguments have been collected, run newsys() */
    if (result = newsys(source, dest, name, modules))
      printf("Error detected.\n");

  }

  /* clean up */
  sync();
  if (source_disk_mounted) (void)umount(source_disk_mounted);
  if (dest_usr_disk_mounted) (void)umount(dest_usr_disk_mounted);
  if (dest_root_disk_mounted) (void)umount(dest_root_disk_mounted);
  (void)unlink(source_mount_point);
  (void)unlink(dest_root_mount_point);
  (void)unlink(tmp_fixit);
  (void)unlink(tmp_cpp);
  (void)unlink(tmp_cpio);
  (void)unlink(tmp_dot_fixit);
  (void)unlink(raw_dot_fixit);
  (void)unlink(tmp_dot_copy);
  (void)unlink(tmp_dot_ftp);
  (void)unlink(usr_tmp_dot_ftp);
  (void)unlink(tmp_dot_modules);
  (void)unlink(source_mount_point);
  (void)unlink(dest_root_mount_point);
  sync();
  exit(result ? 1 : 0);

} /* main() */
