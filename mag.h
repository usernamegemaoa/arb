/*=============================================================================

    This file is part of ARB.

    ARB is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    ARB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ARB; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

=============================================================================*/
/******************************************************************************

    Copyright (C) 2014 Fredrik Johansson

******************************************************************************/

#ifndef MAG_H
#define MAG_H

#include <math.h>
#include "flint.h"
#include "fmpz.h"
#include "fmpz_extras.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LIMB_ONE ((mp_limb_t) 1)
#define LIMB_ONES (-(mp_limb_t) 1)
#define LIMB_TOP (((mp_limb_t) 1) << (FLINT_BITS - 1))
#define MASK_LIMB(n, c) ((n) & (LIMB_ONES << (c)))

#define MAG_MAX_LAGOM_EXP (COEFF_MAX / 4)
#define MAG_MIN_LAGOM_EXP (-MAG_MAX_LAGOM_EXP)

#define ADD2_FAST_MAX (COEFF_MAX / 4)
#define ADD2_FAST_MIN (-ADD2_FAST_MAX)

/* TODO: rename these and move them to fmpz_extras */

static __inline__ void
_fmpz_set_fast(fmpz_t f, const fmpz_t g)
{
    if (!COEFF_IS_MPZ(*f) && !COEFF_IS_MPZ(*g))
        *f = *g;
    else
        fmpz_set(f, g);
}

static __inline__ void
_fmpz_add_fast(fmpz_t z, const fmpz_t x, long c)
{
    fmpz ze, xe;

    ze = *z;
    xe = *x;

    if (!COEFF_IS_MPZ(ze) && (xe > ADD2_FAST_MIN && xe < ADD2_FAST_MAX))
        *z = xe + c;
    else
        fmpz_add_si(z, x, c);
}

static __inline__ void
_fmpz_add2_fast(fmpz_t z, const fmpz_t x, const fmpz_t y, long c)
{
    fmpz ze, xe, ye;

    ze = *z;
    xe = *x;
    ye = *y;

    if (!COEFF_IS_MPZ(ze) && (xe > ADD2_FAST_MIN && xe < ADD2_FAST_MAX) &&
                             (ye > ADD2_FAST_MIN && ye < ADD2_FAST_MAX))
    {
        *z = xe + ye + c;
    }
    else
    {
        fmpz_add(z, x, y);
        fmpz_add_si(z, z, c);
    }
}

static __inline__ void
_fmpz_sub2_fast(fmpz_t z, const fmpz_t x, const fmpz_t y, long c)
{
    fmpz ze, xe, ye;

    ze = *z;
    xe = *x;
    ye = *y;

    if (!COEFF_IS_MPZ(ze) && (xe > ADD2_FAST_MIN && xe < ADD2_FAST_MAX) &&
                             (ye > ADD2_FAST_MIN && ye < ADD2_FAST_MAX))
    {
        *z = xe - ye + c;
    }
    else
    {
        fmpz_sub(z, x, y);
        fmpz_add_si(z, z, c);
    }
}


#define MAG_EXP_POS_INF (COEFF_MIN+1)

/* Finite and with lagom big exponents. */
#define MAG_IS_LAGOM(x) (MAG_EXP(x) >= MAG_MIN_LAGOM_EXP && \
                         MAG_EXP(x) <= MAG_MAX_LAGOM_EXP)

#define MAG_EXPREF(x) (&(x)->exp)
#define MAG_EXP(x) ((x)->exp)
#define MAG_MAN(x) ((x)->man)

#define MAG_BITS 30

#define MAG_ONE_HALF (1UL << (MAG_BITS - 1))

static __inline__ mp_limb_t
__mag_fixmul32(mp_limb_t x, mp_limb_t y)
{
    mp_limb_t u, v;
    umul_ppmm(u, v, x, y);
    return (u << (32 - MAG_BITS)) | (v >> MAG_BITS);
}

#if FLINT_BITS == 64
#define MAG_FIXMUL(x, y) (((x) * (y)) >> MAG_BITS)
#else
#define MAG_FIXMUL(x, y) __mag_fixmul32((x), (y))
#endif


#define MAG_ADJUST_ONE_TOO_LARGE(x) \
    do { \
        mp_limb_t __t = MAG_MAN(x) >> MAG_BITS; \
        MAG_MAN(x) = (MAG_MAN(x) >> __t) + __t; \
        if (__t) \
            fmpz_add_ui(MAG_EXPREF(x), MAG_EXPREF(x), __t); \
    } while (0)

#define MAG_ADJUST_ONE_TOO_SMALL(x) \
    do { \
        mp_limb_t __t = !(MAG_MAN(x) >> (MAG_BITS - 1)); \
        MAG_MAN(x) = (MAG_MAN(x) << __t); \
        if (__t) \
            fmpz_sub_ui(MAG_EXPREF(x), MAG_EXPREF(x), __t); \
    } while (0)

#define MAG_FAST_ADJUST_ONE_TOO_LARGE(x) \
    do { \
        mp_limb_t __t = MAG_MAN(x) >> MAG_BITS; \
        MAG_MAN(x) = (MAG_MAN(x) >> __t) + __t; \
        MAG_EXP(x) += __t; \
    } while (0)

#define MAG_FAST_ADJUST_ONE_TOO_SMALL(x) \
    do { \
        mp_limb_t __t = !(MAG_MAN(x) >> (MAG_BITS - 1)); \
        MAG_MAN(x) = (MAG_MAN(x) << __t); \
        MAG_EXP(x) -= __t; \
    } while (0)

#define MAG_CHECK_BITS(rr) \
    if (MAG_MAN(rr) != 0 && FLINT_BIT_COUNT(MAG_MAN(rr)) != MAG_BITS) \
    { \
        printf("FAIL: wrong number of bits in mantissa!\n"); \
        abort(); \
    }

typedef struct
{
    fmpz exp;
    mp_limb_t man;
}
mag_struct;

typedef mag_struct mag_t[1];

static __inline__ void
mag_init(mag_t x)
{
    fmpz_init(MAG_EXPREF(x));
    MAG_MAN(x) = 0;
}

static __inline__ void
mag_init_set(mag_t x, const mag_t y)
{
    fmpz_init_set(MAG_EXPREF(x), MAG_EXPREF(y));
    MAG_MAN(x) = MAG_MAN(y);
}

static __inline__ void
mag_clear(mag_t x)
{
    fmpz_clear(MAG_EXPREF(x));
}

static __inline__ void
mag_swap(mag_t x, mag_t y)
{
    mag_struct t = *x;
    *x = *y;
    *y = t;
}

static __inline__ void
mag_set(mag_t x, const mag_t y)
{
    _fmpz_set_fast(MAG_EXPREF(x), MAG_EXPREF(y));
    x->man = y->man;
}

static __inline__ void
mag_zero(mag_t x)
{
    fmpz_zero(MAG_EXPREF(x));
    MAG_MAN(x) = 0;
}

static __inline__ int
mag_is_special(const mag_t x)
{
    return MAG_MAN(x) == 0;
}

static __inline__ int
mag_is_zero(const mag_t x)
{
    return (MAG_MAN(x) == 0) && (MAG_EXP(x) == 0);
}

static __inline__ void
mag_inf(mag_t x)
{
    fmpz_clear(MAG_EXPREF(x));
    MAG_EXP(x) = MAG_EXP_POS_INF;
}

static __inline__ int
mag_is_inf(const mag_t x)
{
    return (MAG_MAN(x) == 0) && (MAG_EXP(x) != 0);
}

/* general versions */

void mag_mul(mag_t z, const mag_t x, const mag_t y);

void mag_addmul(mag_t z, const mag_t x, const mag_t y);

void mag_add_2exp_fmpz(mag_t z, const mag_t x, const fmpz_t e);

void mag_div(mag_t z, const mag_t x, const mag_t y);

/* Fast versions (no infs/nans, small exponents). Note that this
   applies to outputs too! */

static __inline__ void
mag_fast_init_set(mag_t x, const mag_t y)
{
    MAG_EXP(x) = MAG_EXP(y);
    MAG_MAN(x) = MAG_MAN(y);
}

static __inline__ void
mag_fast_zero(mag_t x)
{
    MAG_EXP(x) = 0;
    MAG_MAN(x) = 0;
}

static __inline__ int
mag_fast_is_zero(const mag_t x)
{
    return MAG_MAN(x) == 0;
}

static __inline__ void
mag_fast_mul(mag_t z, const mag_t x, const mag_t y)
{
    if (MAG_MAN(x) == 0 || MAG_MAN(y) == 0)
    {
        mag_fast_zero(z);
    }
    else
    {
        MAG_MAN(z) = MAG_FIXMUL(MAG_MAN(x), MAG_MAN(y)) + LIMB_ONE;
        MAG_EXP(z) = MAG_EXP(x) + MAG_EXP(y);
        MAG_FAST_ADJUST_ONE_TOO_SMALL(z);
    }
}

static __inline__ void
mag_fast_addmul(mag_t z, const mag_t x, const mag_t y)
{
    if (MAG_MAN(z) == 0)
    {
        mag_fast_mul(z, x, y);
    }
    else if (MAG_MAN(x) == 0 || MAG_MAN(y) == 0)
    {
        return;
    }
    else
    {
        long shift, e;

        /* x*y < 2^e */
        e = MAG_EXP(x) + MAG_EXP(y);
        shift = MAG_EXP(z) - e;

        if (shift >= 0)
        {
            if (shift >= MAG_BITS)
                MAG_MAN(z)++;
            else
                MAG_MAN(z) = MAG_MAN(z) + (MAG_FIXMUL(MAG_MAN(x), MAG_MAN(y)) >> shift) + 1;
        }
        else
        {
            shift = -shift;
            MAG_EXP(z) = e;

            if (shift >= MAG_BITS)
                MAG_MAN(z) = MAG_FIXMUL(MAG_MAN(x), MAG_MAN(y)) + 2;
            else
                MAG_MAN(z) = MAG_FIXMUL(MAG_MAN(x), MAG_MAN(y)) + (MAG_MAN(z) >> shift) + 2;

            MAG_FAST_ADJUST_ONE_TOO_SMALL(z);
        }

        MAG_FAST_ADJUST_ONE_TOO_LARGE(z);
    }
}

static __inline__ void
mag_fast_add_2exp_si(mag_t z, const mag_t x, long e)
{
    /* Must be zero */
    if (mag_is_special(x))
    {
        MAG_MAN(z) = MAG_ONE_HALF;
        MAG_EXP(z) = e + 1;
    }
    else
    {
        long shift;
        shift = MAG_EXP(x) - e;

        if (shift > 0)
        {
            MAG_EXP(z) = MAG_EXP(x);

            if (shift >= MAG_BITS)
                MAG_MAN(z) = MAG_MAN(x) + LIMB_ONE;
            else
                MAG_MAN(z) = MAG_MAN(x) + (LIMB_ONE << (MAG_BITS - shift));
        }
        else
        {
            shift = -shift;

            MAG_EXP(z) = e + 1;

            if (shift >= MAG_BITS)
                MAG_MAN(z) = MAG_ONE_HALF + LIMB_ONE;
            else
                MAG_MAN(z) = MAG_ONE_HALF + (MAG_MAN(x) >> (shift + 1)) + LIMB_ONE;
        }

        MAG_FAST_ADJUST_ONE_TOO_LARGE(z);
    }
}

#include "fmpr.h"

void mag_set_fmpr(mag_t x, const fmpr_t y);

static __inline__ void
mag_get_fmpr(fmpr_t x, const mag_t r)
{
    if (mag_is_zero(r))
    {
        fmpr_zero(x);
    }
    else if (mag_is_inf(r))
    {
        fmpr_pos_inf(x);
    }
    else
    {
        fmpr_set_ui_2exp_si(x, MAG_MAN(r), -MAG_BITS);
        _fmpz_add2_fast(fmpr_expref(x), fmpr_expref(x), MAG_EXPREF(r), 0);
    }
}

#ifdef __cplusplus
}
#endif

#endif

