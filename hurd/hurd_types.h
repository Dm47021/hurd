/* C declarations for Hurd server interfaces
   Copyright (C) 1993, 1994, 1995 Free Software Foundation, Inc.

This file is part of the GNU Hurd.

The GNU Hurd is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU Hurd is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU Hurd; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef _HURD_TYPES_H
#define _HURD_TYPES_H

#include <mach/std_types.h>	/* For mach_port_t et al. */
#include <sys/types.h>		/* For pid_t and uid_t.  */

/* A string identifying this release of the GNU Hurd.  Our
   interpretation of the term "release" is that it refers to a set of
   server interface definitions.  A "version" in Posix terminology is
   a distribution of the Hurd; there may be more than one distribution
   without changing the release number.  */
#define HURD_RELEASE "0.0 pre-alpha"


/*   Simple type declarations   */

/* These types identify certain kinds of ports used by the Hurd. */
typedef mach_port_t file_t;
typedef mach_port_t fsys_t;
typedef mach_port_t io_t;
typedef mach_port_t process_t;
typedef mach_port_t auth_t;
typedef mach_port_t socket_t;
typedef mach_port_t pf_t;	/* Protocol family */
typedef mach_port_t addr_port_t;
typedef mach_port_t startup_t;
typedef mach_port_t proccoll_t;

#include <errno.h>		/* Defines `error_t'.  */

/* These names exist only because of MiG deficiencies.
   You should not use them in C source; use the normal C types instead.  */
typedef char *data_t;
typedef char string_t [1024];
typedef int *intarray_t;
typedef int *fd_mask_t;
typedef mach_port_t *portarray_t;
typedef pid_t *pidarray_t;
typedef uid_t *idarray_t;
typedef off_t *off_array_t;
typedef struct rusage rusage_t;
typedef struct flock flock_t;
typedef struct utsname utsname_t;
typedef struct stat io_statbuf_t;


/*   Parameters and flags in RPC calls   */

/* Many such parameters and flags are also defined in various libc
   headers. */

/* Bits for flags in fs.defs:file_exec and exec.defs:exec_* calls: */
#define EXEC_NEWTASK	0x00000001 /* Create new task; kill old one.  */
#define EXEC_SECURE	0x00000002 /* Use secure values of portarray, etc. */
#define EXEC_DEFAULTS	0x00000004 /* Use defaults for unspecified ports.  */
/* These two are passed through by the exec server but not examined by it.  */
#define	EXEC_TRACED	0x00000008 /* Process is traced.  */
#define	EXEC_STACK_ARGS	0x00000010 /* Use arguments from stack, not RPC.  */
#define	EXEC_INHERITED	(EXEC_TRACED) /* Flags inherited by later execs.  */

/* Bits for flags in fs.defs:file_set_translator call: */
#define FS_TRANS_FORCE     0x00000001 /* Must use translator(no sht circuit) */
#define FS_TRANS_EXCL      0x00000002 /* Don't do it if already translated.  */
#define FS_TRANS_SET	   0x00000004 /* Set or clear translator */

/* Values for retry field in fs.defs:dir_pathtrans call: */
enum retry_type
{
  FS_RETRY_NORMAL = 1,		/* Retry normally.  */
  FS_RETRY_REAUTH = 2,		/* Retry after reauthenticating retry port. */
  FS_RETRY_MAGICAL = 3,		/* Retry string is magical.  */
  /* "tty" means controlling tty;

     "fd/%u" means file descriptor N;

     "machtype/..." means replace `machtype' with the numbers in decimal
		    returned by the user's kernel as the cpu_type (N) and 
		    cpu_subtype (M) (producing N/M/...) and then retry
		    as for FS_RETRY_NORMAL.

     "/..." means retry "...", but starting from the users root directory.
     */
};
typedef enum retry_type retry_type;

/* Types for msg.defs:dir_changed call: */
enum dir_changed_type
{
  DIR_CHANGED_NULL,		/* Always sent first for sync.  */
  DIR_CHANGED_NEW,		/* Specified name has been added.  */
  DIR_CHANGED_UNLINK,		/* Specified name has been removed.  */
  DIR_CHANGED_RENUMBER,		/* Name has been the target of rename.  */
};
typedef enum dir_changed_type dir_changed_type_t;

/* Types for msg.defs:file_changed call: */
enum file_changed_type
{
  FILE_CHANGED_NULL,		/* Always sent first for sync.  */
  FILE_CHANGED_WRITE,		/* File data has been written.  */
  FILE_CHANGED_EXTEND,		/* File has grown.  */
  FILE_CHANGED_TRUNCATE,	/* File has been truncated.  */
  FILE_CHANGED_META,		/* Stat information has changed, and none
				   of the previous three apply.  Not sent
				   for changes in node times.  */
};
typedef enum file_changed_type file_changed_type_t;

/* Select types for io.defs:io_select call: */
#define SELECT_READ  0x00000001
#define SELECT_WRITE 0x00000002
#define SELECT_URG   0x00000004

/* Flags for fsys.defs:fsys_goaway.  Also, these flags are sent as the
   oldtrans_flags in fs.defs:file_set_translator to describe how to
   terminate the old translator. */
#define FSYS_GOAWAY_NOWAIT    0x00000001 /* Return immediately.  */
#define FSYS_GOAWAY_NOSYNC    0x00000002 /* Don't update physical media.  */
#define FSYS_GOAWAY_FORCE     0x00000004 /* Go away despite current users.  */
#define FSYS_GOAWAY_UNLINK    0x00000008 /* Go away only if non-directory.  */
#define FSYS_GOAWAY_RECURSE   0x00000010 /* Shutdown children too.  */

/* Types of ports the terminal driver can run on top of;
   used in term.defs:term_get_bottom_type.  */
enum term_bottom_type
{
 TERM_ON_MACHDEV,
 TERM_ON_HURDIO,
 TERM_ON_MASTERPTY,
};

/* Flags for file_get_storage_info. */
#define STORAGE_MUTATED   0x00000001 /* data as stored is munged from file */

/* Classes for file_get_storage_info. */
enum file_storage_class
{
  STORAGE_OTHER,
  STORAGE_DEVICE,
  STORAGE_HURD_FILE,
  STORAGE_NETWORK,
  STORAGE_MEMORY,
  STORAGE_TASK,
};

/* STORAGE_DEVICE implies that:
    
   STORAGE_PORT, if non-null, is a device_t holding the data.
   
   STORAGE_NAME, if non-null, is the name (as for device_open) of the
   device holding the data.  (Caveat: use storage_port in preference
   to storage_name for actual I/O)
   
   The even members of ADDRESSES specify the physical addresses of the
   data of the file, in order (in units appropriate as a RECNUM to
   device_read/write) .  The odd members specify the lengths (in
   bytes) of the storage at the preceding address.  An address of -1
   identifies a hole, a length of zero should be ignored along with
   the preceding address.

   STORAGE_MISC may contain additional type specific information.
   */

/* STORAGE_HURD_FILE implies that:

   STORAGE_PORT, if non-null, is a file_t holding the data.

   STORAGE_NAME, if non-null, is the filename of the file referenced
   by STORAGE_PORT.  (Caveat: use storage_port in preference to
   storage_name if both are provided.)

   ADDRESSES are pairs of address pairs; the even numbers are off_t
   offsets from the start of the file and the odd numbers are the
   length of the segment.  -1 addresses are holes; zero lengths
   should be ignored.

   STORAGE_MISC may contain additional type-specific information. 
*/

/* STORAGE_TASK is like STORAGE_HURD_FILE, except that the data is found
   in the virtual address space of the task identified by STORAGE_PORT. 

/* STORAGE_MEMORY is similar, except the data is found in the memory
   object identified by STORAGE_PORT. */

/* STORAGE_NETWORK means that the file is stored elsewhere on the
   network; all the remaining fields contan type-specific information. */

/* STORAGE_OTHER means none of these apply; and should be used when no
   meaningful answer can be given. */



/*   Data types   */

/* This structure is known to be 19 ints long in hurd_types.defs.  */
struct fsys_statfsbuf
{
  long fsys_stb_type;
  long fsys_stb_bsize;		/* fundamental allocation unit */
  long fsys_stb_iosize;		/* optimal I/O transfer size */
  long fsys_stb_blocks;		/* number of blocks (fsys_stb_bsize) */
  long fsys_stb_bfree;		/* likewise */
  long fsys_stb_bavail;		/* likewise */
  long fsys_stb_files;
  long fsys_stb_ffree;
  fsid_t fsys_stb_fsid;
  long fsys_stb_spare[9];
};
typedef struct fsys_statfsbuf fsys_statfsbuf_t;

#include <mach/task_info.h>
#include <mach/thread_info.h>

struct procinfo
{
  int state;
  uid_t owner;
  pid_t ppid;
  pid_t pgrp;
  pid_t session;
  pid_t logincollection;

  int nthreads;			/* size of pi_threadinfos */
  
  struct task_basic_info taskinfo;
  struct
    {
      struct thread_basic_info pis_bi;
      struct thread_sched_info pis_si;
    } threadinfos[0];
};
typedef int *procinfo_t;

/* Bits in struct procinfo  state: */
#define PI_STOPPED 0x00000001	/* Proc server thinks is stopped.  */
#define PI_EXECED  0x00000002	/* Has called proc_exec.  */
#define PI_WAITING 0x00000004	/* Process is waiting for child to exit */
#define PI_ORPHAN  0x00000008	/* Process group is orphaned.  */
#define PI_NOMSG   0x00000010	/* Process has no message port.  */
#define PI_SESSLD  0x00000020	/* Session leader.  */
#define PI_NOTOWNED 0x0000040	/* Process has no owner.  */
#define PI_NOPARENT 0x0000080	/* Hasn't identified a parent.  */
#define PI_ZOMBIE  0x00000100	/* Has no associated task.  */
#define PI_TRACED  0x00000200	/* Process is being traced */
#define PI_GETMSG  0x00000400	/* Process is blocked in proc_getmsgport. */
#define PI_LOGINLD 0x00000800	/* Process is leader of login collection */


/*   Conventions   */


/* st_fstype in struct stat and fsys_stb_type in fsys_statfsbuf is one of: */
#define FSTYPE_UFS     0x00000000 /* 4.x BSD Fast File System */
#define FSTYPE_NFS     0x00000001 /* Network File System ala Sun */
#define FSTYPE_GFS     0x00000002 /* GNU file system */
#define FSTYPE_LFS     0x00000003 /* Logging File System ala Sprite */
#define FSTYPE_SYSV    0x00000004 /* Old U*x filesystem ala System V */
#define FSTYPE_FTP     0x00000005 /* Transparent FTP */
#define FSTYPE_TAR     0x00000006 /* Transparent TAR */
#define FSTYPE_AR      0x00000007 /* Transparent AR */
#define FSTYPE_CPIO    0x00000008 /* Transparent CPIO */
#define FSTYPE_MSLOSS  0x00000009 /* MS-DOS */
#define FSTYPE_CPM     0x0000000a /* CP/M */
#define FSTYPE_HFS     0x0000000b /* Don't ask */
#define FSTYPE_DTFS    0x0000000c /* used by desktop to provide more info */
#define FSTYPE_GRFS    0x0000000d /* GNU Remote File System */
#define FSTYPE_TERM    0x0000000e /* GNU Terminal driver */
#define FSTYPE_DEV     0x0000000f /* GNU Special file server */
#define FSTYPE_PROC    0x00000010 /* /proc filesystem ala Version 9 */
#define FSTYPE_IFSOCK  0x00000011 /* PF_LOCAL socket naming point */
#define FSTYPE_AFS     0x00000012 /* Andrew File System 3.xx */
#define FSTYPE_DFS     0x00000013 /* Distributed File Sys (OSF) == AFS 4.xx */
#define FSTYPE_PROC9   0x00000014 /* /proc filesystem ala Plan 9 */
#define FSTYPE_SOCKET  0x00000015 /* Naming point for socket server */
#define FSTYPE_MISC    0x00000016 /* generic trivfs server */
#define FSTYPE_EXT2FS  0x00000017 /* Linux filesystem by Remy Card */

/* Standard port assignments for file_exec and exec_* */
enum
  {
    INIT_PORT_CWDIR,
    INIT_PORT_CRDIR,
    INIT_PORT_AUTH,
    INIT_PORT_PROC,
    INIT_PORT_CTTYID,
    /* If MACH_PORT_NULL is given for the bootstrap port,
       the bootstrap port of the old task is used.  */
    INIT_PORT_BOOTSTRAP,
    INIT_PORT_MAX
  };

/* Standard ints for file_exec and exec_* */
enum
  {
    INIT_UMASK,
    INIT_SIGMASK,
    INIT_SIGIGN,
    INIT_SIGPENDING,
    INIT_INT_MAX,
  };

#endif
