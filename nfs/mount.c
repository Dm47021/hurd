/* 
   Copyright (C) 1995, 1996 Free Software Foundation, Inc.
   Written by Michael I. Bushnell, p/BSG.

   This file is part of the GNU Hurd.

   The GNU Hurd is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   The GNU Hurd is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA. */

#include "nfs.h"

#include <rpcsvc/mount.h>
#include <rpc/pmap_prot.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>

/* Service name for portmapper */
char *pmap_service_name = "sunrpc";

/* Fallback port number for portmapper */
short pmap_service_number = PMAPPORT; 

/* RPC program for mount server. */
int mount_program = MOUNTPROG;

/* RPC version for mount server. */
int mount_version = MOUNTVERS;

/* Fallback port number for mount server. */
short mount_port = 0;

/* True iff MOUNT_PORT should be used even if portmapper present. */
int mount_port_override = 0;

/* RPC program number for NFS server. */
int nfs_program = NFS_PROGRAM;

/* RPC version number for NFS server. */
int nfs_version = NFS_VERSION;

/* Fallback port number for NFS server. */
short nfs_port = NFS_PORT;

/* True iff NFS_PORT should be used even if portmapper present. */
int nfs_port_override = 0;

int protocol_version = 2;

/* Set up an RPC for procedure PROCNUM for talking to the portmapper.
   Allocate storage with malloc and point *BUF at it; caller must free
   this when done.  Return the address where the args for the
   procedure should be placed. */
int *
pmap_initialize_rpc (int procnum, void **buf)
{
  return initialize_rpc (PMAPPROG, PMAPVERS, procnum, 0, buf, 0, 0, -1);
}

/* Set up an RPC for procedure PROCNUM for talking to the mount
   server.  Allocate storage with malloc and point *BUF at it; caller
   must free this when done.  Return the address where the args for
   the procedure should be placed.  */
int *
mount_initialize_rpc (int procnum, void **buf)
{
  return initialize_rpc (MOUNTPROG, MOUNTVERS, procnum, 0, buf, 0, 0, -1);
}

/* Using the mount protocol, lookup NAME at host HOST.
   Return a node for it or null for an error. */
struct node *
mount_root (char *name, char *host)
{
  struct sockaddr_in addr;
  struct hostent *h;
  struct servent *s;
  int *p;
  void *rpcbuf;
  int port;
  struct node *np;
  short pmapport;

  /* Lookup the portmapper port number */
  if (pmap_service_name)
    {
      s = getservbyname ("sunrpc", pmap_service_name);
      if (s)
	pmapport = s->s_port;
      else
	pmapport = htons (pmap_service_number);
    }
  else
    pmapport = htons (pmap_service_number);

  /* Lookup the host */
  h = gethostbyname (host);
  if (!h)
    {
      herror (host);
      return 0;
    }
  
  addr.sin_family = h->h_addrtype;
  bcopy (h->h_addr_list[0], &addr.sin_addr, h->h_length);
  addr.sin_port = pmapport;
  
  connect (main_udp_socket,
	   (struct sockaddr *)&addr, sizeof (struct sockaddr_in));

  if (!mount_port_override)
    {
      /* Formulate and send a PMAPPROC_GETPORT request
	 to lookup the mount program on the server.  */
      p = pmap_initialize_rpc (PMAPPROC_GETPORT, &rpcbuf);
      *p++ = htonl (MOUNTPROG);
      *p++ = htonl (MOUNTVERS);
      *p++ = htonl (IPPROTO_UDP);
      *p++ = htonl (0);
      errno = conduct_rpc (&rpcbuf, &p);
      if (!errno)
	{
	  port = ntohl (*p++);
	  addr.sin_port = htons (port);
	}
      else if (mount_port)
	addr.sin_port = htons (mount_port);
      else
	{
	  free (rpcbuf);
	  perror ("portmap of mount");
	  return 0;
	}
      free (rpcbuf);
    }
  else
    addr.sin_port = htons (mount_port);
  

  /* Now talking to the mount program, fetch the file handle
     for the root. */
  connect (main_udp_socket, 
	   (struct sockaddr *) &addr, sizeof (struct sockaddr_in));
  p = mount_initialize_rpc (MOUNTPROC_MNT, &rpcbuf);
  p = xdr_encode_string (p, name);
  errno = conduct_rpc (&rpcbuf, &p);
  if (errno)
    {
      free (rpcbuf);
      perror (name);
      return 0;
    }
  /* XXX Protocol spec says this should be a "unix error code"; we'll
     pretend that an NFS error code is what's meant, the numbers match
     anyhow.  */
  errno = nfs_error_trans (htonl (*p++));
  if (errno)
    {
      free (rpcbuf);
      perror (name);
      return 0;
    }
  
  /* Create the node for root */
  np = lookup_fhandle (p);
  p += NFS2_FHSIZE / sizeof (int);
  free (rpcbuf);
  mutex_unlock (&np->lock);

  if (!nfs_port_override)
    {
      /* Now send another PMAPPROC_GETPORT request to lookup the nfs server. */
      addr.sin_port = pmapport;
      connect (main_udp_socket, 
	       (struct sockaddr *) &addr, sizeof (struct sockaddr_in));
      p = pmap_initialize_rpc (PMAPPROC_GETPORT, &rpcbuf);
      *p++ = htonl (NFS_PROGRAM);
      *p++ = htonl (NFS_VERSION);
      *p++ = htonl (IPPROTO_UDP);
      *p++ = htonl (0);
      errno = conduct_rpc (&rpcbuf, &p);
      if (!errno)
	port = ntohl (*p++);
      else if (nfs_port)
	port = nfs_port;
      else
	{
	  free (rpcbuf);
	  perror ("pmap of nfs server");
	  return 0;
	}
      free (rpcbuf);
    }
  else
    port = nfs_port;
  
  addr.sin_port = htons (port);
  connect (main_udp_socket, 
	   (struct sockaddr *) &addr, sizeof (struct sockaddr_in));
  
  return np;
}
