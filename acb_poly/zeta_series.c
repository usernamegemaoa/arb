/*
    Copyright (C) 2014 Fredrik Johansson

    This file is part of Arb.

    Arb is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "acb_poly.h"

void
_acb_poly_zeta_cpx_series(acb_ptr z, const acb_t s, const acb_t a, int deflate, slong d, slong prec)
{
    ulong M, N;
    slong i, bound_prec;
    mag_t bound;
    arb_ptr vb;
    int is_real, const_is_real;

    if (d < 1)
        return;

    if (!acb_is_finite(s) || !acb_is_finite(a))
    {
        _acb_vec_indeterminate(z, d);
        return;
    }

    if (acb_is_one(s) && deflate && d == 1)
    {
        acb_digamma(z, a, prec);
        acb_neg(z, z);
        if (!acb_is_finite(z))  /* todo: in digamma */
            acb_indeterminate(z);
        return;
    }

    is_real = const_is_real = 0;

    if (acb_is_real(s) && acb_is_real(a))
    {
        if (arb_is_positive(acb_realref(a)))
        {
            is_real = const_is_real = 1;
        }
        else if (arb_is_int(acb_realref(a)) &&
             arb_is_int(acb_realref(s)) &&
             arb_is_nonpositive(acb_realref(s)))
        {
            const_is_real = 1;
        }
    }

    mag_init(bound);
    vb = _arb_vec_init(d);

    bound_prec = 40 + prec / 20;

    _acb_poly_zeta_em_choose_param(bound, &N, &M, s, a, FLINT_MIN(d, 2), prec, bound_prec);
    _acb_poly_zeta_em_bound(vb, s, a, N, M, d, bound_prec);

    _acb_poly_zeta_em_sum(z, s, a, deflate, N, M, d, prec);

    for (i = 0; i < d; i++)
    {
        arb_get_mag(bound, vb + i);
        arb_add_error_mag(acb_realref(z + i), bound);

        if (!is_real && !(i == 0 && const_is_real))
            arb_add_error_mag(acb_imagref(z + i), bound);
    }

    mag_clear(bound);
    _arb_vec_clear(vb, d);
}

void
_acb_poly_zeta_cpx_reflect(acb_ptr t, const acb_t h, const acb_t a, int deflate, slong len, slong prec)
{
    /* use reflection formula */
    if (arf_sgn(arb_midref(acb_realref(h))) < 0 && acb_is_one(a))
    {
        /* zeta(s) = (2*pi)**s * sin(pi*s/2) / pi * gamma(1-s) * zeta(1-s) */
        acb_t pi, hcopy;
        acb_ptr f, s1, s2, s3, s4, u;
        slong i;

        acb_init(pi);
        acb_init(hcopy);
        f = _acb_vec_init(2);
        s1 = _acb_vec_init(len);
        s2 = _acb_vec_init(len);
        s3 = _acb_vec_init(len);
        s4 = _acb_vec_init(len);
        u = _acb_vec_init(len);
        acb_set(hcopy, h);

        acb_const_pi(pi, prec);

        /* s1 = (2*pi)**s */
        acb_mul_2exp_si(pi, pi, 1);
        _acb_poly_pow_cpx(s1, pi, h, len, prec);
        acb_mul_2exp_si(pi, pi, -1);

        /* s2 = sin(pi*s/2) / pi */
        acb_set(f, h);
        acb_one(f + 1);
        acb_mul_2exp_si(f, f, -1);
        acb_mul_2exp_si(f + 1, f + 1, -1);
        _acb_poly_sin_pi_series(s2, f, 2, len, prec);
        _acb_vec_scalar_div(s2, s2, len, pi, prec);

        /* s3 = gamma(1-s) */
        acb_sub_ui(f, hcopy, 1, prec);
        acb_neg(f, f);
        acb_set_si(f + 1, -1);
        _acb_poly_gamma_series(s3, f, 2, len, prec);

        /* s4 = zeta(1-s) */
        acb_sub_ui(f, hcopy, 1, prec);
        acb_neg(f, f);
        _acb_poly_zeta_cpx_series(s4, f, a, 0, len, prec);
        for (i = 1; i < len; i += 2)
            acb_neg(s4 + i, s4 + i);

        _acb_poly_mullow(u, s1, len, s2, len, len, prec);
        _acb_poly_mullow(s1, s3, len, s4, len, len, prec);
        _acb_poly_mullow(t, u, len, s1, len, len, prec);

        /* add 1/(1-(s+t)) = 1/(1-s) + t/(1-s)^2 + ... */
        if (deflate)
        {
            acb_sub_ui(u, hcopy, 1, prec);
            acb_neg(u, u);
            acb_inv(u, u, prec);
            for (i = 1; i < len; i++)
                acb_mul(u + i, u + i - 1, u, prec);
            _acb_vec_add(t, t, u, len, prec);
        }

        acb_clear(pi);
        acb_clear(hcopy);
        _acb_vec_clear(f, 2);
        _acb_vec_clear(s1, len);
        _acb_vec_clear(s2, len);
        _acb_vec_clear(s3, len);
        _acb_vec_clear(s4, len);
        _acb_vec_clear(u, len);
    }
    else
    {
        _acb_poly_zeta_cpx_series(t, h, a, deflate, len, prec);
    }
}

void
_acb_poly_zeta_series(acb_ptr res, acb_srcptr h, slong hlen, const acb_t a, int deflate, slong len, slong prec)
{
    acb_ptr t, u;
    hlen = FLINT_MIN(hlen, len);

    t = _acb_vec_init(len);
    u = _acb_vec_init(len);

    _acb_poly_zeta_cpx_reflect(t, h, a, deflate, len, prec);

    /* compose with nonconstant part */
    acb_zero(u);
    _acb_vec_set(u + 1, h + 1, hlen - 1);
    _acb_poly_compose_series(res, t, len, u, hlen, len, prec);

    _acb_vec_clear(t, len);
    _acb_vec_clear(u, len);
}

void
acb_poly_zeta_series(acb_poly_t res, const acb_poly_t f, const acb_t a, int deflate, slong n, slong prec)
{
    if (n == 0)
    {
        acb_poly_zero(res);
        return;
    }

    acb_poly_fit_length(res, n);

    if (f->length == 0)
    {
        acb_t t;
        acb_init(t);
        _acb_poly_zeta_series(res->coeffs, t, 1, a, deflate, n, prec);
        acb_clear(t);
    }
    else
    {
        _acb_poly_zeta_series(res->coeffs, f->coeffs, f->length, a, deflate, n, prec);
    }

    _acb_poly_set_length(res, n);
    _acb_poly_normalise(res);
}

