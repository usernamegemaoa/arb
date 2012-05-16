/*=============================================================================

    This file is part of ARB.

    ARB is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    ARB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ARB; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

=============================================================================*/
/******************************************************************************

    Copyright (C) 2012 Fredrik Johansson

******************************************************************************/

#include "arb.h"

int _arb_contains_fmpq(const fmpz_t mid, const fmpz_t rad, long exp,
    const fmpz_t num, const fmpz_t den)
{
    fmpz_t a, b;
    int result;

    fmpz_init(a);
    fmpz_init(b);

    /* compare with left endpoint */
    fmpz_sub(a, mid, rad);
    fmpz_mul(a, a, den);
    if (exp >= 0)
    {
        fmpz_mul_2exp(a, a, exp);
        fmpz_set(b, num);
    }
    else
        fmpz_mul_2exp(b, num, -exp);

    result = (fmpz_cmp(a, b) <= 0);

    /* compare with right endpoint */
    if (result != 0)
    {
        fmpz_add(a, mid, rad);
        fmpz_mul(a, a, den);
        if (exp >= 0)
            fmpz_mul_2exp(a, a, exp);
        result = (fmpz_cmp(a, b) >= 0);
    }

    fmpz_clear(a);
    fmpz_clear(b);

    return result;
}

int
arb_contains_fmpq(const arb_t x, const fmpq_t q)
{
    long exp = *arb_expref(x);

    /* fixme */
    if (COEFF_IS_MPZ(exp))
    {
        printf("error: arb_contains_fmpq: too large exponent\n");
        abort();
    }

    return _arb_contains_fmpq(arb_midref(x), arb_radref(x), exp,
        fmpq_numref(q), fmpq_denref(q));
}