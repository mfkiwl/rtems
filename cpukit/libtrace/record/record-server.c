/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2018, 2024 embedded brains GmbH & Co. KG
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

#include <rtems/recordserver.h>
#include <rtems/record.h>
#include <rtems/score/threadimpl.h>

#include <sys/socket.h>
#include <sys/uio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>

#define WAKEUP_EVENT RTEMS_EVENT_0

static void wakeup( rtems_id task )
{
  (void) rtems_event_send( task, WAKEUP_EVENT );
}

static void wait( rtems_option options )
{
  rtems_event_set events;

  (void) rtems_event_receive(
    WAKEUP_EVENT,
    RTEMS_EVENT_ANY | options,
    RTEMS_NO_TIMEOUT,
    &events
  );
}

static void wakeup_timer( rtems_id timer, void *arg )
{
  rtems_id *server;

  server = arg;
  wakeup( *server );
  (void) rtems_timer_reset( timer );
}

static void send_header( int fd )
{
  Record_Stream_header header;
  size_t               size;

  size = _Record_Stream_header_initialize( &header );
  (void) write( fd, &header, size );
}

typedef struct {
  int fd;
  size_t index;
  rtems_record_item items[ 128 ];
} thread_names_context;

static void thread_names_produce(
  thread_names_context *ctx,
  rtems_record_event    event,
  rtems_record_data     data
)
{
  size_t i;

  i = ctx->index;
  ctx->items[ i ].event = RTEMS_RECORD_TIME_EVENT( 0, event );
  ctx->items[ i ].data = data;

  if (i == RTEMS_ARRAY_SIZE(ctx->items) - 1) {
    ctx->index = 0;
    (void) write( ctx->fd, ctx->items, sizeof( ctx->items ) );
  } else {
    ctx->index = i + 1;
  }
}

static bool thread_names_visitor( rtems_tcb *tcb, void *arg )
{
  thread_names_context *ctx;
  char                  name[ 2 * THREAD_DEFAULT_MAXIMUM_NAME_SIZE ];
  size_t                n;
  size_t                i;
  rtems_record_data     data;

  ctx = arg;
  thread_names_produce( ctx, RTEMS_RECORD_THREAD_ID, tcb->Object.id );
  n = _Thread_Get_name( tcb, name, sizeof( name ) );
  i = 0;

  while ( i < n ) {
    size_t j;

    data = 0;

    for ( j = 0; i < n && j < sizeof( data ); ++j ) {
      rtems_record_data c;

      c = (unsigned char) name[ i ];
      data |= c << ( j * 8 );
      ++i;
    }

    thread_names_produce( ctx, RTEMS_RECORD_THREAD_NAME, data );
  }

  return false;
}

static void send_thread_names( int fd )
{
  thread_names_context ctx;

  ctx.fd = fd;
  ctx.index = 0;
  rtems_task_iterate( thread_names_visitor, &ctx );

  if ( ctx.index > 0 ) {
    (void) write( ctx.fd, ctx.items, ctx.index * sizeof( ctx.items[ 0 ] ) );
  }
}

static void fetch_and_write(
  int fd,
  rtems_record_item *items,
  size_t count
)
{
  rtems_record_fetch_control control;

  rtems_record_fetch_initialize( &control, items, count );

  while ( true ) {
    rtems_record_fetch_status status;

    do {
      ssize_t n;

      status = rtems_record_fetch( &control );
      n = write(
        fd,
        control.fetched_items,
        control.fetched_count * sizeof( *control.fetched_items )
      );

      if ( n <= 0 ) {
        return;
      }
    } while ( status == RTEMS_RECORD_FETCH_CONTINUE );

    wait( RTEMS_WAIT );
  }
}

void rtems_record_server( uint16_t port, rtems_interval period )
{
  rtems_status_code sc;
  rtems_id self;
  rtems_id timer;
  struct sockaddr_in addr;
  int sd;
  int rv;
  size_t count;
  rtems_record_item *items;

  self = rtems_task_self();

  count = rtems_record_get_item_count_for_fetch();
  items = calloc( count, sizeof( *items ) );
  if ( items == NULL ) {
    return;
  }

  sc = rtems_timer_create( rtems_build_name( 'R', 'C', 'R', 'D' ), &timer );
  if ( sc != RTEMS_SUCCESSFUL ) {
    goto timer_error;
  }

  sd = socket( PF_INET, SOCK_STREAM, 0 );
  if (sd < 0) {
    goto socket_error;
  }

  memset( &addr, 0, sizeof( addr ) );
  addr.sin_family = AF_INET;
  addr.sin_port = htons( port );
  addr.sin_addr.s_addr = htonl( INADDR_ANY );

  rv = bind( sd, (const struct sockaddr *) &addr, sizeof( addr ) );
  if (rv != 0) {
    goto error;
  }

  rv = listen( sd, 0 );
  if (rv != 0) {
    goto error;
  }

  while ( true ) {
    int cd;

    cd = accept( sd, NULL, NULL );

    if ( cd < 0 ) {
      break;
    }

    wait( RTEMS_NO_WAIT );
    (void) rtems_timer_fire_after( timer, period, wakeup_timer, &self );
    send_header( cd );
    send_thread_names( cd );
    fetch_and_write( cd, items, count );
    (void) rtems_timer_cancel( timer );
    (void) close( cd );
  }

error:

  (void) close( sd );

socket_error:

  (void) rtems_timer_delete( timer );

timer_error:

  free( items );
}

typedef struct {
  rtems_id       task;
  uint16_t       port;
  rtems_interval period;
} server_arg;

static void server( rtems_task_argument arg )
{
  server_arg     *sarg;
  uint16_t        port;
  rtems_interval  period;

  sarg = (server_arg *) arg;
  port = sarg->port;
  period = sarg->period;
  wakeup(sarg->task);
  rtems_record_server( port, period );
  rtems_task_exit();
}

rtems_status_code rtems_record_start_server(
  rtems_task_priority priority,
  uint16_t            port,
  rtems_interval      period
)
{
  rtems_status_code sc;
  rtems_id          id;
  server_arg        sarg;

  sarg.port = port;
  sarg.period = period;
  sarg.task = rtems_task_self();

  sc = rtems_task_create(
    rtems_build_name( 'R', 'C', 'R', 'D' ),
    priority,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &id
  );
  if ( sc != RTEMS_SUCCESSFUL ) {
    return sc;
  }

  (void) rtems_task_start( id, server, (rtems_task_argument) &sarg );
  wait( RTEMS_WAIT );

  return RTEMS_SUCCESSFUL;
}
