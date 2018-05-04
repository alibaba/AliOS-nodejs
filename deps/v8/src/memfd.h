
/*-*- Mode: C; c-basic-offset: 8; indent-tabs-mode: nil -*-*/

#pragma once

/***
  This file is part of systemd.

  Copyright 2010 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

/* Missing glibc definitions to access certain kernel APIs */

#include <sys/resource.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

// #define _PLATFORM_phone 1

#ifndef F_LINUX_SPECIFIC_BASE
#define F_LINUX_SPECIFIC_BASE 1024
#endif

#ifndef F_ADD_SEALS
#define F_ADD_SEALS (F_LINUX_SPECIFIC_BASE + 9)
#define F_GET_SEALS (F_LINUX_SPECIFIC_BASE + 10)

#define F_SEAL_SEAL     0x0001  /* prevent further seals from being set */
#define F_SEAL_SHRINK   0x0002  /* prevent file from shrinking */
#define F_SEAL_GROW     0x0004  /* prevent file from growing */
#define F_SEAL_WRITE    0x0008  /* prevent writes */
#endif


#ifndef MFD_ALLOW_SEALING
#define MFD_ALLOW_SEALING 0x0002U
#endif

#ifndef MFD_CLOEXEC
#define MFD_CLOEXEC 0x0001U
#endif


#ifndef __NR_memfd_create
#  if defined __x86_64__
#    define __NR_memfd_create 319
#  elif defined __arm__
#    if defined _PLATFORM_tv
#      define __NR_memfd_create 380
#    endif
#    if defined _PLATFORM_ivi
#      define __NR_memfd_create 382
#    endif
#    if defined _PLATFORM_phone
#      define __NR_memfd_create 384
#    endif
#  elif defined __aarch64__
#    define __NR_memfd_create 279
#  elif defined _MIPS_SIM
#    if _MIPS_SIM == _MIPS_SIM_ABI32
#      define __NR_memfd_create 4354
#    endif
#    if _MIPS_SIM == _MIPS_SIM_NABI32
#      define __NR_memfd_create 6318
#    endif
#    if _MIPS_SIM == _MIPS_SIM_ABI64
#      define __NR_memfd_create 5314
#    endif
#  elif defined __i386__
#    define __NR_memfd_create 356
#  else
#    warning "__NR_memfd_create unknown for your architecture"
#    define __NR_memfd_create 0xffffffff
#  endif
#endif

#ifndef HAVE_MEMFD_CREATE
static inline int memfd_create(const char *name, unsigned int flags) {
        return (int)syscall(__NR_memfd_create, name, flags);
}
#endif
