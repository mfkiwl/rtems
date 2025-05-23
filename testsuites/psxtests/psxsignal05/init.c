/* SPDX-License-Identifier: BSD-2-Clause */

/*
 *  COPYRIGHT (c) 1989-2012.
 *  On-Line Applications Research Corporation (OAR).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define TEST_NAME                "PSXSIGNAL 5"
#define TEST_STRING              "User Signals"
#define SIGNAL_ONE               SIGUSR1
#define SIGNAL_TWO               SIGUSR2
#define SIGNAL_TEST              31

#include <pmacros.h>
#include <signal.h>
#include <errno.h>
#include <errno.h>
#include <rtems/posix/psignalimpl.h>

const char rtems_test_name[] = TEST_NAME;

/* forward declarations to avoid warnings */
void *POSIX_Init(void *argument);
void Signal_handler(int signo, siginfo_t *info, void *arg);
const char *signal_name(int signo);

void Signal_handler(
  int        signo,
  siginfo_t *info,
  void      *arg
)
{
  puts( "Signal handler hit" );
  rtems_test_exit(0);
}

const char *signal_name(int signo)
{
  if (signo == SIGUSR1)
    return "SIGUSR1";
  if (signo == SIGUSR2)
    return "SIGUSR2";
  if (signo == SIGRTMIN)
    return "SIGRTMIN";
  if (signo == SIGRTMAX)
    return "SIGRTMAX";
  return "unknown-signal";
}

void *POSIX_Init(
  void *argument
)
{
  struct sigaction    act;
  siginfo_t           info;
  bool                bc;

  TEST_BEGIN();

  act.sa_handler = NULL;
  act.sa_sigaction = Signal_handler;
  act.sa_flags   = SA_SIGINFO;
  sigaction( SIGNAL_ONE, &act, NULL );
  sigaction( SIGNAL_TWO, &act, NULL );
  sigaction( SIGNAL_TEST, &act, NULL );

  printf(
   "Init - _POSIX_signals_Clear_signals when signals pending but\n"
   "Init -    not queued on SA_SIGINFO configured signal.\n"
  );

  /* cheat and put signal directly in */
  _POSIX_signals_Pending |= signo_to_mask( SIGUSR1 );

  rtems_test_assert( _POSIX_signals_Pending == signo_to_mask( SIGUSR1 ) );

  bc = _POSIX_signals_Clear_signals(
    _Thread_Get_executing()->API_Extensions[ THREAD_API_POSIX ],
    SIGNAL_ONE,
    &info,
    true,              /* is_global */
    false,             /* check_blocked */
    true               /* do_signals_acquire_release */
  );
  rtems_test_assert( bc );
  rtems_test_assert( _POSIX_signals_Pending == 0 );

  /* cheat and put signal directly in */
  _POSIX_signals_Pending |= signo_to_mask( SIGNAL_TEST );

  rtems_test_assert( _POSIX_signals_Pending == signo_to_mask( SIGNAL_TEST ) );
  /* clear invalid signo */
  bc = _POSIX_signals_Clear_signals(
    _Thread_Get_executing()->API_Extensions[ THREAD_API_POSIX ],
    -1,
    &info,
    true,              /* is_global */
    false,             /* check_blocked */
    true               /* do_signals_acquire_release */
  );
  rtems_test_assert( !bc );
  rtems_test_assert( _POSIX_signals_Pending == signo_to_mask( SIGNAL_TEST ) );

  bc = _POSIX_signals_Clear_signals(
    _Thread_Get_executing()->API_Extensions[ THREAD_API_POSIX ],
    SIGNAL_TEST,
    &info,
    true,              /* is_global */
    false,             /* check_blocked */
    true               /* do_signals_acquire_release */
  );
  rtems_test_assert( bc );
  rtems_test_assert( _POSIX_signals_Pending == 0 );

  TEST_END();
  rtems_test_exit(0);

  return NULL; /* just so the compiler thinks we returned something */
}

/* configuration information */

#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_MAXIMUM_POSIX_THREADS        1

#define CONFIGURE_POSIX_INIT_THREAD_TABLE

#define CONFIGURE_INIT
#include <rtems/confdefs.h>
