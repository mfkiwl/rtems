/* SPDX-License-Identifier: BSD-2-Clause */

/**
 *  @file
 *
 *  @ingroup
 *
 *  @brief
 */

/*
 * Copyright (C) 2014 Chris Johns <chrisj@rtems.org>.
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
 * Taken from NetBSD and stripped of the relocations not needed on RTEMS.
 */

/*  $NetBSD: ppc_reloc.c,v 1.44 2010/01/13 20:17:22 christos Exp $  */

#include <sys/cdefs.h>

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <rtems/rtl/rtl.h>
#include "rtl-bit-alloc.h"
#include "rtl-elf.h"
#include "rtl-error.h"
#include <rtems/rtl/rtl-trace.h>
#include "rtl-unwind.h"
#include "rtl-unwind-dw2.h"

#define ha(x) ((((u_int32_t)(x) & 0x8000) ? \
                 ((u_int32_t)(x) + 0x10000) : (u_int32_t)(x)) >> 16)
#define l(x) ((u_int32_t)(x) & 0xffff)

/*
 * SDATA allocator.
 */
static rtems_rtl_bit_alloc* sdata;

static Elf_Addr
get_sda_base (void)
{
  uint32_t sda_base;
  __asm__ volatile (" mr %0, 13\n" : "=r" (sda_base));
  return sda_base;
}

/*
 *  Access the variables via asm statements to avoid any fix up issues
 *  generated by the C compiler which thinks they belong in the .sdata
 *  section.
 */

#define GET_ADDR(_l, _v) \
  __asm__ volatile (" lis %0, " #_l "@h\n" \
                    " ori %0, %0, " #_l "@l\n" : "=r" (_v))

static void*
get_sdata_start (void)
{
#if _ARCH_PPC64
  return NULL;
#else
  Elf_Addr addr;
  GET_ADDR(__SDATA_START__, addr);
  return (void*) addr;
#endif
}

#if !_ARCH_PPC64
static size_t
get_sdata_sbss_size (void)
{
  Elf_Addr sdata_begin;
  Elf_Addr sbss_end;
  GET_ADDR(bsp_section_sdata_begin, sdata_begin);
  GET_ADDR(bsp_section_sbss_end, sbss_end);
  return sbss_end - sdata_begin;
}

static size_t
get_sdata_libdl_size (void)
{
  Elf_Addr begin;
  Elf_Addr end;
  GET_ADDR(bsp_section_sdata_libdl_begin, begin);
  GET_ADDR(bsp_section_sdata_libdl_end, end);
  return end - begin;
}
#endif

uint32_t
rtems_rtl_elf_section_flags (const rtems_rtl_obj* obj,
                             const Elf_Shdr*      shdr)
{
  return 0;
}

uint32_t
rtems_rtl_elf_arch_parse_section (const rtems_rtl_obj* obj,
                                  int                  section,
                                  const char*          name,
                                  const Elf_Shdr*      shdr,
                                  const uint32_t       flags)
{
#if !_ARCH_PPC64
  struct {
    const char* label;
    size_t      len;
  } prefix[] = {
    #define SEPARATED_PREFIX(_p) { _p, sizeof (_p) - 1 }
    SEPARATED_PREFIX (".sdata"),
    SEPARATED_PREFIX (".sbss"),
  };
  const size_t prefixes = sizeof (prefix) / sizeof (prefix[0]);
  size_t       p;
  for (p = 0; p < prefixes; ++p)
  {
    if (strncmp (name, prefix[p].label, prefix[p].len) == 0)
      return flags | RTEMS_RTL_OBJ_SECT_ARCH_ALLOC;
  }
#endif
  return flags;
}

bool
rtems_rtl_elf_arch_section_alloc (const rtems_rtl_obj* obj,
                                  rtems_rtl_obj_sect*  sect)
{
#if _ARCH_PPC64
    rtems_rtl_set_error (ENOMEM, ".sdata no supported by ABI");
    return false;
#else
  if (rtems_rtl_trace (RTEMS_RTL_TRACE_DETAIL))
    printf ("rtl: section: arch: alloc: name=%s size=%zu flags=%08" PRIx32 \
            " order=%i link=%d info=%d\n",
            sect->name, sect->size, sect->flags, sect->load_order,
            sect->link, sect->info);

  if (sdata == NULL)
  {
    sdata = rtems_rtl_bit_alloc_open (get_sdata_start (),
                                      get_sdata_libdl_size (),
                                      sizeof (uint32_t),
                                      get_sdata_sbss_size ());
    if (sdata == NULL)
    {
      rtems_rtl_set_error (ENOMEM, "no memory for sdata allocator");
      return false;
    }
  }

  sect->base = rtems_rtl_bit_alloc_balloc (sdata, sect->size);
  if (sect->base == NULL)
  {
    rtems_rtl_set_error (ENOMEM, "no .sdata memory: %s", sect->name);
    return false;
  }

  return true;
#endif
}

bool
rtems_rtl_elf_arch_section_free (const rtems_rtl_obj* obj,
                                  rtems_rtl_obj_sect*  sect)
{
#if !_ARCH_PPC64
  if (rtems_rtl_trace (RTEMS_RTL_TRACE_DETAIL))
    printf ("rtl: section: arch: free: name=%s size=%zu\n", sect->name, sect->size);
  if (sdata != NULL)
    rtems_rtl_bit_alloc_bfree (sdata, sect->base, sect->size);
#endif
  return true;
}

bool
rtems_rtl_elf_rel_resolve_sym (Elf_Word type)
{
  return true;
}

uint32_t rtems_rtl_obj_tramp_alignment (const rtems_rtl_obj* obj)
{
  (void) obj;
  return sizeof(uint32_t);
}

size_t
rtems_rtl_elf_relocate_tramp_max_size (void)
{
  /*
   * We have 4 instructions and each instruction is 32bits.
   */
  return 4 * 4;
}

static void*
set_veneer (void* trampoline, Elf_Addr target)
{
  /*
   * http://shell-storm.org/online/Online-Assembler-and-Disassembler/
   *
   *  lis   12,0x1234
   *  ori   12,12,0x5678
   *  mtctr 12
   *  bctr
   */
#if COMPILE_ASM
  asm volatile (" lis   12,0x1234\n" \
                " ori   12,12,0x5678\n" \
                " mtctr 12\n" \
                " bctr\n");
#endif
  uint32_t* tramp = (uint32_t*) trampoline;
  *tramp++ = 0x3d800000 | (target >> 16);
  *tramp++ = 0x618c0000 | (target & 0xffff);
  *tramp++ = 0x7d8903a6;
  *tramp++ = 0x4e800420;
  return tramp;
}

static size_t
get_veneer_size (int type)
{
  (void) type;
  return rtems_rtl_elf_relocate_tramp_max_size ();
}

/**
 * The offsets in the reloc words.
 */
#define REL_R_OFFSET (0)
#define REL_R_INFO   (1)
#define REL_R_ADDEND (2)

static rtems_rtl_elf_rel_status
rtems_rtl_elf_reloc_rela (rtems_rtl_obj*            obj,
                          const Elf_Rela*           rela,
                          const rtems_rtl_obj_sect* sect,
                          const char*               symname,
                          const Elf_Byte            syminfo,
                          const Elf_Word            symvalue,
                          const bool                parsing)
{
  Elf_Addr* where;
  Elf_Word tmp;
  uint32_t mask = 0;
  uint32_t bits = 0;
  bool     needs_tramp = false;

  where = (Elf_Addr *)(sect->base + rela->r_offset);
  switch (ELF_R_TYPE(rela->r_info)) {
    case R_TYPE(NONE):
      break;

    case R_TYPE(32):
      /*
       * value:1; Field: word32; Expression: S + A
       */
      if (!parsing) {
        *where = symvalue + rela->r_addend;
        if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
          printf ("rtl: ADDR32 %p @ %p in %s\n",
                  (void *)*(where), where, rtems_rtl_obj_oname (obj));
      }
      break;

    case R_TYPE(14):
      /*
       * value:7; Field: low14*; Expression: (S + A) >> 2
       */
    case R_TYPE(24):
      /*
       * value:2; Field: low24*; Expression: (S + A) >> 2
       */
      if (ELF_R_TYPE(rela->r_info) == R_TYPE(14)) {
        bits = 14;
        mask = 0xfffc;
      } else {
        bits = 24;
        mask = 0x3fffffc;
      }

      if (parsing && sect->base == 0) {
        if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
          printf ("rtl: ADDR14/ADDR24 tramp cache\n");
        return rtems_rtl_elf_rel_tramp_cache;
      }

      tmp = (symvalue + rela->r_addend) >> 2;
      if (tmp > ((1<<bits) - 1 )) {
        Elf_Word tramp_addr;
        size_t   tramp_size = get_veneer_size(ELF_R_TYPE(rela->r_info));
        if (parsing) {
          if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
            printf ("rtl: ADDR14/ADDR24 tramp add\n");
          return rtems_rtl_elf_rel_tramp_add;
        }
        if (!rtems_rtl_obj_has_tramp_space (obj, tramp_size)) {
          if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
            printf ("rtl: ADDR14/ADDR24 no tramp slot: %s\n", rtems_rtl_obj_oname (obj));
          rtems_rtl_set_error (ENOMEM, "%s: tramp: no slot: ADDR14/ADDR24", sect->name);
          return rtems_rtl_elf_rel_failure;
        }
        needs_tramp = true;
        tramp_addr = (Elf_Addr) obj->tramp_brk;
        obj->tramp_brk = set_veneer(obj->tramp_brk,
                                    symvalue + rela->r_addend);
        tmp = *where;
        tmp &= ~mask;
        tmp |= (tramp_addr + rela->r_addend) & mask;
      }
      else {
        tmp = *where;
        tmp &= ~mask;
        tmp |= (symvalue + rela->r_addend) & mask;
      }

      if (!parsing) {
        *where = tmp;
        if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
          printf ("rtl: ADDR14/ADDR24%s %p @ %p in %s\n",
                  needs_tramp ? "(tramp)" : "",
                  (void *)*where, where, rtems_rtl_obj_oname (obj));
      }
      break;

    case R_TYPE(16_HA):
    case R_TYPE(TPREL16_HA):
      /*
       * value:6; Field:half16; Expression: #ha(S+A)
       * value:72; Field:half16; Expression: #ha(S+A)
       */
      if (!parsing) {
        tmp = symvalue + rela->r_addend;
        *(uint16_t *)where = (((tmp >> 16) + ((tmp & 0x8000) ? 1: 0)) & 0xffff);
        if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
          printf ("rtl: %s16_HA %p @ %p in %s\n",
                  ELF_R_TYPE(rela->r_info) == R_TYPE(TPREL16_HA) ? "TPREL" : "",
                  (void *)*(where), where, rtems_rtl_obj_oname (obj));
      }
      break;

    case R_TYPE(16_HI):
    case R_TYPE(TPREL16_HI):
      /*
       * value:5; Field:half16; Expression: #hi(S+A)
       * value:71; Field:half16; Expression: #hi(S+A)
       */
      if (!parsing) {
        *(uint16_t *)where = ((symvalue + rela->r_addend) >> 16) & 0xffff;
        if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
          printf ("rtl: %s16_HI %p @ %p in %s\n",
                  ELF_R_TYPE(rela->r_info) == R_TYPE(TPREL16_HI) ? "TPREL" : "",
                  (void *)*where, where, rtems_rtl_obj_oname (obj));
      }
      break;

    case R_TYPE(16_LO):
    case R_TYPE(TPREL16_LO):
      /*
       * value:4; Field:half16; Expression: #lo(S+A)
       * value:71; Field:half16; Expression: #lo(S+A)
       */
      if (!parsing) {
        *(uint16_t *)where = (symvalue + (rela->r_addend)) & 0xffff;
        if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
          printf ("rtl: %s16_LO %p @ %p in %s\n",
                  ELF_R_TYPE(rela->r_info) == R_TYPE(TPREL16_LO) ? "TPREL" : "",
                  (void *)*where, where, rtems_rtl_obj_oname (obj));
      }
      break;

    case R_TYPE(REL14):
      /*
       * value:11; Field:low14*; Expression:(S+A-P)>>2
       */
    case R_TYPE(REL24):
      /*
       * value:10; Field:low24*; Expression:(S+A-P)>>2
       */
      if (ELF_R_TYPE(rela->r_info) == R_TYPE(REL24)) {
        mask = 0x3fffffc;
        bits = 24;
      }
      else if (ELF_R_TYPE(rela->r_info) == R_TYPE(REL14)) {
        mask = 0xfffc;
        bits = 14;
      }

      if (parsing && sect->base == 0) {
        if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
          printf ("rtl: REL24/REL14 tramp cache\n");
        return rtems_rtl_elf_rel_tramp_cache;
      }

      tmp =((int) (symvalue + rela->r_addend - (Elf_Addr)where)) >> 2;
      if (((Elf_Sword)tmp > ((1<<(bits-1)) - 1)) ||
          ((Elf_Sword)tmp < -(1<<(bits-1)))) {
        Elf_Word tramp_addr;
        size_t   tramp_size = get_veneer_size(ELF_R_TYPE(rela->r_info));
        if (parsing) {
          if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
            printf ("rtl: REL24/REL14 tramp add\n");
          return rtems_rtl_elf_rel_tramp_add;
        }
        if (!rtems_rtl_obj_has_tramp_space (obj, tramp_size)) {
          if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
            printf ("rtl: REL24/REL14 no tramp slot: %s\n", rtems_rtl_obj_oname (obj));
          rtems_rtl_set_error (ENOMEM, "%s: tramp: no slot: REL24/REL14", sect->name);
          return rtems_rtl_elf_rel_failure;
        }
        needs_tramp = true;
        tramp_addr = (Elf_Addr) obj->tramp_brk;
        obj->tramp_brk = set_veneer(obj->tramp_brk,
                                    symvalue + rela->r_addend);
        tmp = *where;
        tmp &= ~mask;
        tmp |= (tramp_addr + rela->r_addend - (Elf_Addr)where) & mask;
      }
      else
      {
        tmp = *where;
        tmp &= ~mask;
        tmp |= (symvalue + rela->r_addend - (Elf_Addr)where) & mask;
      }

      if (!parsing) {
        *where = tmp;
        if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
          printf ("rtl: REL24/REL14%s %p @ %p in %s\n",
                  needs_tramp ? "(tramp)" : "",
                  (void *)*where, where, rtems_rtl_obj_oname (obj));
      }
      break;

    case R_TYPE(REL32):
      /*
       * value:26; Field:word32*; Expression:S+A-P
       */
      if (!parsing) {
        *where = symvalue + rela->r_addend - (Elf_Addr)where;
        if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
          printf ("rtl: REL32 %p @ %p in %s\n",
                  (void *)*where, where, rtems_rtl_obj_oname (obj));
      }
      break;

    case R_TYPE(SDAREL16):
      /*
       * A sign-extended 16 bit value relative to _SDA_BASE_, for use with
       * small data items.
       */
      if (!parsing) {
        Elf_Addr sda_base = get_sda_base ();
        mask = 0xffff;
        tmp = *((Elf32_Half*) where);
        tmp &= ~mask;
        tmp |= (symvalue + rela->r_addend - sda_base) & mask;
        *((Elf32_Half*) where) = tmp;
        if (rtems_rtl_trace (RTEMS_RTL_TRACE_RELOC))
          printf ("rtl: SDAREL16 %p @ %p in %s\n",
                  (void *) (uintptr_t) *((Elf32_Half*) where),
                  where, rtems_rtl_obj_oname (obj));
      }
      break;

    default:
      printf ("rtl: reloc unknown: sym = %" PRIu32 ", type = %" PRIu32 ", offset = %p, "
              "contents = %p\n",
              ELF_R_SYM(rela->r_info), (uint32_t) ELF_R_TYPE(rela->r_info),
              (void *)rela->r_offset, (void *)*where);
      rtems_rtl_set_error (EINVAL,
                           "%s: Unsupported relocation type %" PRId32
                           " in non-PLT relocations",
                           sect->name, (uint32_t) ELF_R_TYPE(rela->r_info));
      return rtems_rtl_elf_rel_failure;
  }
  return rtems_rtl_elf_rel_no_error;
}

rtems_rtl_elf_rel_status
rtems_rtl_elf_relocate_rela_tramp (rtems_rtl_obj*            obj,
                                   const Elf_Rela*           rela,
                                   const rtems_rtl_obj_sect* sect,
                                   const char*               symname,
                                   const Elf_Byte            syminfo,
                                   const Elf_Word            symvalue)
{
  return rtems_rtl_elf_reloc_rela (obj,
                                   rela,
                                   sect,
                                   symname,
                                   syminfo,
                                   symvalue,
                                   true);
}

rtems_rtl_elf_rel_status
rtems_rtl_elf_relocate_rela (rtems_rtl_obj*            obj,
                             const Elf_Rela*           rela,
                             const rtems_rtl_obj_sect* sect,
                             const char*               symname,
                             const Elf_Byte            syminfo,
                             const Elf_Word            symvalue)
{
  return rtems_rtl_elf_reloc_rela (obj,
                                   rela,
                                   sect,
                                   symname,
                                   syminfo,
                                   symvalue,
                                   false);
}

rtems_rtl_elf_rel_status
rtems_rtl_elf_relocate_rel_tramp (rtems_rtl_obj*            obj,
                                  const Elf_Rel*            rel,
                                  const rtems_rtl_obj_sect* sect,
                                  const char*               symname,
                                  const Elf_Byte            syminfo,
                                  const Elf_Word            symvalue)
{
  (void) obj;
  (void) rel;
  (void) sect;
  (void) symname;
  (void) syminfo;
  (void) symvalue;
  rtems_rtl_set_error (EINVAL, "rel type record not supported");
  return rtems_rtl_elf_rel_failure;
}

rtems_rtl_elf_rel_status
rtems_rtl_elf_relocate_rel (rtems_rtl_obj*            obj,
                            const Elf_Rel*            rel,
                            const rtems_rtl_obj_sect* sect,
                            const char*               symname,
                            const Elf_Byte            syminfo,
                            const Elf_Word            symvalue)
{
  (void) obj;
  (void) rel;
  (void) sect;
  (void) symname;
  (void) syminfo;
  (void) symvalue;
  rtems_rtl_set_error (EINVAL, "rel type record not supported");
  return rtems_rtl_elf_rel_failure;
}

bool
rtems_rtl_elf_unwind_parse (const rtems_rtl_obj* obj,
                            const char*          name,
                            uint32_t             flags)
{
  return rtems_rtl_elf_unwind_dw2_parse (obj, name, flags);
}

bool
rtems_rtl_elf_unwind_register (rtems_rtl_obj* obj)
{
  return rtems_rtl_elf_unwind_dw2_register (obj);
}

bool
rtems_rtl_elf_unwind_deregister (rtems_rtl_obj* obj)
{
  return rtems_rtl_elf_unwind_dw2_deregister (obj);
}
