/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 *
 * @ingroup RTEMSTestCaseRtemsBarrierReqDelete
 */

/*
 * Copyright (C) 2021 embedded brains GmbH (http://www.embedded-brains.de)
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

/*
 * This file is part of the RTEMS quality process and was automatically
 * generated.  If you find something that needs to be fixed or
 * worded better please post a report or patch to an RTEMS mailing list
 * or raise a bug report:
 *
 * https://www.rtems.org/bugs.html
 *
 * For information on updating and regenerating please refer to the How-To
 * section in the Software Requirements Engineering chapter of the
 * RTEMS Software Engineering manual.  The manual is provided as a part of
 * a release.  For development sources please refer to the online
 * documentation at:
 *
 * https://docs.rtems.org
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems.h>
#include <string.h>

#include <rtems/test.h>

/**
 * @defgroup RTEMSTestCaseRtemsBarrierReqDelete spec:/rtems/barrier/req/delete
 *
 * @ingroup RTEMSTestSuiteTestsuitesValidation0
 *
 * @{
 */

typedef enum {
  RtemsBarrierReqDelete_Pre_Id_Valid,
  RtemsBarrierReqDelete_Pre_Id_Invalid,
  RtemsBarrierReqDelete_Pre_Id_NA
} RtemsBarrierReqDelete_Pre_Id;

typedef enum {
  RtemsBarrierReqDelete_Post_Status_Ok,
  RtemsBarrierReqDelete_Post_Status_InvId,
  RtemsBarrierReqDelete_Post_Status_NA
} RtemsBarrierReqDelete_Post_Status;

typedef enum {
  RtemsBarrierReqDelete_Post_Id_Valid,
  RtemsBarrierReqDelete_Post_Id_Invalid,
  RtemsBarrierReqDelete_Post_Id_NA
} RtemsBarrierReqDelete_Post_Id;

typedef enum {
  RtemsBarrierReqDelete_Post_Flush_Yes,
  RtemsBarrierReqDelete_Post_Flush_No,
  RtemsBarrierReqDelete_Post_Flush_NA
} RtemsBarrierReqDelete_Post_Flush;

/**
 * @brief Test context for spec:/rtems/barrier/req/delete test case.
 */
typedef struct {
  rtems_id worker_id;

  rtems_id barrier_id;

  uint32_t wait_done;

  uint32_t wait_expected;

  rtems_id id;

  rtems_status_code status;

  /**
   * @brief This member defines the pre-condition states for the next action.
   */
  size_t pcs[ 1 ];

  /**
   * @brief This member indicates if the test action loop is currently
   *   executed.
   */
  bool in_action_loop;
} RtemsBarrierReqDelete_Context;

static RtemsBarrierReqDelete_Context
  RtemsBarrierReqDelete_Instance;

static const char * const RtemsBarrierReqDelete_PreDesc_Id[] = {
  "Valid",
  "Invalid",
  "NA"
};

static const char * const * const RtemsBarrierReqDelete_PreDesc[] = {
  RtemsBarrierReqDelete_PreDesc_Id,
  NULL
};

#define NAME rtems_build_name( 'T', 'E', 'S', 'T' )

typedef RtemsBarrierReqDelete_Context Context;

typedef enum {
  PRIO_HIGH = 1,
  PRIO_NORMAL,
  PRIO_LOW
} Priorities;

static void Worker( rtems_task_argument arg )
{
  Context *ctx;

  ctx = (Context *) arg;

  while ( true ) {
    rtems_status_code   sc;
    rtems_task_priority prio;

    T_eq_u32( ctx->barrier_id, 0 );

    sc = rtems_barrier_create(
      NAME,
      RTEMS_DEFAULT_ATTRIBUTES,
      0,
      &ctx->barrier_id
    );
    T_rsc_success( sc );

    sc = rtems_barrier_wait(
      ctx->barrier_id,
      RTEMS_NO_TIMEOUT
    );
    T_rsc( sc, RTEMS_OBJECT_WAS_DELETED );

    ++ctx->wait_done;

    prio = 0;
    sc = rtems_task_set_priority( RTEMS_SELF, PRIO_LOW, &prio );
    T_rsc_success( sc );
    T_eq_u32( prio, PRIO_HIGH );
  }
}

static void RtemsBarrierReqDelete_Pre_Id_Prepare(
  RtemsBarrierReqDelete_Context *ctx,
  RtemsBarrierReqDelete_Pre_Id   state
)
{
  switch ( state ) {
    case RtemsBarrierReqDelete_Pre_Id_Valid: {
      ctx->id = ctx->barrier_id;
      break;
    }

    case RtemsBarrierReqDelete_Pre_Id_Invalid: {
      ctx->id = 0;
      break;
    }

    case RtemsBarrierReqDelete_Pre_Id_NA:
      break;
  }
}

static void RtemsBarrierReqDelete_Post_Status_Check(
  RtemsBarrierReqDelete_Context    *ctx,
  RtemsBarrierReqDelete_Post_Status state
)
{
  switch ( state ) {
    case RtemsBarrierReqDelete_Post_Status_Ok: {
      ctx->barrier_id = 0;
      T_rsc_success( ctx->status );
      break;
    }

    case RtemsBarrierReqDelete_Post_Status_InvId: {
      T_rsc( ctx->status, RTEMS_INVALID_ID );
      break;
    }

    case RtemsBarrierReqDelete_Post_Status_NA:
      break;
  }
}

static void RtemsBarrierReqDelete_Post_Id_Check(
  RtemsBarrierReqDelete_Context *ctx,
  RtemsBarrierReqDelete_Post_Id  state
)
{
  rtems_status_code sc;
  rtems_id          id;

  switch ( state ) {
    case RtemsBarrierReqDelete_Post_Id_Valid: {
      id = 0;
      sc = rtems_barrier_ident( NAME, &id );
      T_rsc_success( sc );
      T_eq_u32( id, ctx->barrier_id );
      break;
    }

    case RtemsBarrierReqDelete_Post_Id_Invalid: {
      sc = rtems_barrier_ident( NAME, &id );
      T_rsc( sc, RTEMS_INVALID_NAME );
      break;
    }

    case RtemsBarrierReqDelete_Post_Id_NA:
      break;
  }
}

static void RtemsBarrierReqDelete_Post_Flush_Check(
  RtemsBarrierReqDelete_Context   *ctx,
  RtemsBarrierReqDelete_Post_Flush state
)
{
  switch ( state ) {
    case RtemsBarrierReqDelete_Post_Flush_Yes: {
      ++ctx->wait_expected;
      T_eq_u32( ctx->wait_done, ctx->wait_expected );
      break;
    }

    case RtemsBarrierReqDelete_Post_Flush_No: {
      T_eq_u32( ctx->wait_done, ctx->wait_expected );
      break;
    }

    case RtemsBarrierReqDelete_Post_Flush_NA:
      break;
  }
}

static void RtemsBarrierReqDelete_Setup( RtemsBarrierReqDelete_Context *ctx )
{
  rtems_status_code   sc;
  rtems_task_priority prio;

  memset( ctx, 0, sizeof( *ctx ) );

  prio = 0;
  sc = rtems_task_set_priority( RTEMS_SELF, PRIO_NORMAL, &prio );
  T_rsc_success( sc );
  T_eq_u32( prio, PRIO_HIGH );

  sc = rtems_task_create(
    rtems_build_name( 'W', 'O', 'R', 'K' ),
    PRIO_LOW,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &ctx->worker_id
  );
  T_assert_rsc_success( sc );

  sc = rtems_task_start( ctx->worker_id, Worker, (rtems_task_argument) ctx );
  T_assert_rsc_success( sc );
}

static void RtemsBarrierReqDelete_Setup_Wrap( void *arg )
{
  RtemsBarrierReqDelete_Context *ctx;

  ctx = arg;
  ctx->in_action_loop = false;
  RtemsBarrierReqDelete_Setup( ctx );
}

static void RtemsBarrierReqDelete_Teardown(
  RtemsBarrierReqDelete_Context *ctx
)
{
  rtems_status_code   sc;
  rtems_task_priority prio;

  prio = 0;
  sc = rtems_task_set_priority( RTEMS_SELF, PRIO_HIGH, &prio );
  T_rsc_success( sc );
  T_eq_u32( prio, PRIO_NORMAL );

  if ( ctx->worker_id != 0 ) {
    sc = rtems_task_delete( ctx->worker_id );
    T_rsc_success( sc );
  }
}

static void RtemsBarrierReqDelete_Teardown_Wrap( void *arg )
{
  RtemsBarrierReqDelete_Context *ctx;

  ctx = arg;
  ctx->in_action_loop = false;
  RtemsBarrierReqDelete_Teardown( ctx );
}

static size_t RtemsBarrierReqDelete_Scope( void *arg, char *buf, size_t n )
{
  RtemsBarrierReqDelete_Context *ctx;

  ctx = arg;

  if ( ctx->in_action_loop ) {
    return T_get_scope( RtemsBarrierReqDelete_PreDesc, buf, n, ctx->pcs );
  }

  return 0;
}

static T_fixture RtemsBarrierReqDelete_Fixture = {
  .setup = RtemsBarrierReqDelete_Setup_Wrap,
  .stop = NULL,
  .teardown = RtemsBarrierReqDelete_Teardown_Wrap,
  .scope = RtemsBarrierReqDelete_Scope,
  .initial_context = &RtemsBarrierReqDelete_Instance
};

static const uint8_t RtemsBarrierReqDelete_TransitionMap[][ 3 ] = {
  {
    RtemsBarrierReqDelete_Post_Status_Ok,
    RtemsBarrierReqDelete_Post_Id_Invalid,
    RtemsBarrierReqDelete_Post_Flush_Yes
  }, {
    RtemsBarrierReqDelete_Post_Status_InvId,
    RtemsBarrierReqDelete_Post_Id_Valid,
    RtemsBarrierReqDelete_Post_Flush_No
  }
};

static const struct {
  uint8_t Skip : 1;
  uint8_t Pre_Id_NA : 1;
} RtemsBarrierReqDelete_TransitionInfo[] = {
  {
    0, 0
  }, {
    0, 0
  }
};

static void RtemsBarrierReqDelete_Prepare( RtemsBarrierReqDelete_Context *ctx )
{
  rtems_status_code   sc;
  rtems_task_priority prio;

  prio = 0;
  sc = rtems_task_set_priority( ctx->worker_id, PRIO_HIGH, &prio );
  T_rsc_success( sc );
  T_true( prio == PRIO_LOW || prio == PRIO_HIGH );
}

static void RtemsBarrierReqDelete_Action( RtemsBarrierReqDelete_Context *ctx )
{
  ctx->status = rtems_barrier_delete( ctx->id );
}

static void RtemsBarrierReqDelete_Cleanup( RtemsBarrierReqDelete_Context *ctx )
{
  if ( ctx->barrier_id != 0 ) {
    rtems_status_code sc;

    sc = rtems_barrier_delete( ctx->barrier_id );
    T_rsc_success( sc );
  }
}

/**
 * @fn void T_case_body_RtemsBarrierReqDelete( void )
 */
T_TEST_CASE_FIXTURE( RtemsBarrierReqDelete, &RtemsBarrierReqDelete_Fixture )
{
  RtemsBarrierReqDelete_Context *ctx;
  size_t index;

  ctx = T_fixture_context();
  ctx->in_action_loop = true;
  index = 0;

  for (
    ctx->pcs[ 0 ] = RtemsBarrierReqDelete_Pre_Id_Valid;
    ctx->pcs[ 0 ] < RtemsBarrierReqDelete_Pre_Id_NA;
    ++ctx->pcs[ 0 ]
  ) {
    if ( RtemsBarrierReqDelete_TransitionInfo[ index ].Pre_Id_NA ) {
      ctx->pcs[ 0 ] = RtemsBarrierReqDelete_Pre_Id_NA;
      index += ( RtemsBarrierReqDelete_Pre_Id_NA - 1 );
    }

    if ( RtemsBarrierReqDelete_TransitionInfo[ index ].Skip ) {
      ++index;
      continue;
    }

    RtemsBarrierReqDelete_Prepare( ctx );
    RtemsBarrierReqDelete_Pre_Id_Prepare( ctx, ctx->pcs[ 0 ] );
    RtemsBarrierReqDelete_Action( ctx );
    RtemsBarrierReqDelete_Post_Status_Check(
      ctx,
      RtemsBarrierReqDelete_TransitionMap[ index ][ 0 ]
    );
    RtemsBarrierReqDelete_Post_Id_Check(
      ctx,
      RtemsBarrierReqDelete_TransitionMap[ index ][ 1 ]
    );
    RtemsBarrierReqDelete_Post_Flush_Check(
      ctx,
      RtemsBarrierReqDelete_TransitionMap[ index ][ 2 ]
    );
    RtemsBarrierReqDelete_Cleanup( ctx );
    ++index;
  }
}

/** @} */
