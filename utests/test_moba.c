/* Copyright 2019. Uecker Lab, University Medical Center Goettingen.
 * All rights reserved. Use of this source code is governed by
 * a BSD-style license which can be found in the LICENSE file.
 *
 * Authors: Xiaoqing Wang, Martin Uecker
 */

#include <complex.h>

#include "num/multind.h"
#include "num/flpmath.h"
#include "num/rand.h"
#include "num/iovec.h"
#include "num/ops_p.h"

#include "misc/misc.h"
#include "misc/debug.h"
#include "misc/mmio.h"
#include "misc/utils.h"

#include "linops/linop.h"
#include "linops/lintest.h"
#include "linops/someops.h"

#include "nlops/nlop.h"
#include "nlops/nltest.h"

#include "moba/moba.h"
#include "moba/blochfun.h"
#include "moba/T1fun.h"
#include "moba/T1phyfun.h"
#include "moba/ir_meco.h"
#include "moba/optreg.h"

#include "utest.h"






static bool test_nlop_T1fun(void) 
{
	enum { N = 16 };
	long map_dims[N] = { 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long out_dims[N] = { 16, 16, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long in_dims[N] = { 16, 16, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long TI_dims[N] = { 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	complex float* dst = md_alloc(N, out_dims, CFL_SIZE);
	complex float* src = md_alloc(N, in_dims, CFL_SIZE);

	complex float TI[4] = { 0., 1., 2., 3. };

	md_zfill(N, in_dims, src, 1.0);

	struct nlop_s* T1 = nlop_T1_create(N, map_dims, out_dims, in_dims, TI_dims, TI, 2., 1.);

	nlop_apply(T1, N, out_dims, dst, N, in_dims, src);
	
	float err = linop_test_adjoint_real(nlop_get_derivative(T1, 0, 0));

	nlop_free(T1);

	md_free(src);
	md_free(dst);

	UT_RETURN_ASSERT(err < UT_TOL);
}

UT_REGISTER_TEST(test_nlop_T1fun);

static bool test_nlop_T1fun_der(void) 
{
	enum { N = 16 };
	long map_dims[N] = { 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long out_dims[N] = { 16, 16, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long in_dims[N] = { 16, 16, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long TI_dims[N] = { 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	complex float TI[4] = { 0., 1., 2., 3. };


	struct nlop_s* T1 = nlop_T1_create(N, map_dims, out_dims, in_dims, TI_dims, TI, 2., 1.);
	float err = nlop_test_derivative(T1);

	nlop_free(T1);


	const float tol = 1.e-2;
	debug_printf(DP_DEBUG1, "T1fun_der error: %.8f, tol %.2f\n", err, tol);
	UT_RETURN_ASSERT(err < tol);
}

UT_REGISTER_TEST(test_nlop_T1fun_der);

static bool test_op_p_stack_moba_nonneg(void)
{
	enum { N = 5 };
	long dims[N] = { 2, 4, 7, 5, 6};

	long strs[N];
	md_calc_strides(N, strs, dims, CFL_SIZE);

	long s_dim = 2;

	long p_pos = 3;
	unsigned int s_flag = MD_BIT(p_pos);

	const struct operator_p_s* p = moba_nonneg_prox_create(N, dims, s_dim, s_flag, 0.);

	complex float* in  = md_alloc(N, dims, CFL_SIZE);
	complex float* out = md_alloc(N, dims, CFL_SIZE);

	md_zfill(N, dims, in, -1.);
	md_zfill(N, dims, out, 100.);

	operator_p_apply(p, 0., N, dims, out, N, dims, in);
	operator_p_free(p);

	long dims1[N];
	md_select_dims(N, ~MD_BIT(s_dim), dims1, dims);

	complex float* in1 = md_alloc(N, dims1, CFL_SIZE);

	long pos[N];
	for (int i = 0; i < N; i++)
		pos[i] = 0;

	pos[s_dim] = p_pos;

	md_copy_block(N, pos, dims1, in1, dims, in, CFL_SIZE);
	md_clear(N, dims1, in1, CFL_SIZE);
	md_copy_block(N, pos, dims, in, dims1, in1, CFL_SIZE);

	float err = md_znrmse(N, dims, out, in);

	md_free(in);
	md_free(in1);
	md_free(out);

	UT_RETURN_ASSERT(err < UT_TOL);
}

UT_REGISTER_TEST(test_op_p_stack_moba_nonneg);


static bool test_nlop_blochfun(void)
{
	enum { N = 16 };
	long map_dims[N] = { 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long out_dims[N] = { 3, 3, 1, 1, 1, 500, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long in_dims[N] = { 3, 3, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long all_dims[N] = { 3, 3, 1, 1, 1, 500, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	complex float* dst = md_alloc(N, out_dims, CFL_SIZE);
	complex float* src = md_alloc(N, in_dims, CFL_SIZE);

	struct moba_conf_s data;

        data.sim.seq = simdata_seq_defaults;
        data.sim.voxel = simdata_voxel_defaults;
        data.sim.pulse = simdata_pulse_defaults;
        data.sim.pulse.hs = pulse_hypsec_defaults;
        data.sim.pulse.sinc = pulse_sinc_defaults;
        data.sim.grad = simdata_grad_defaults;
	data.sim.other = simdata_other_defaults;

        data.other = moba_other_defaults;

        data.sim.seq.perfect_inversion = true; //FIXME: HypSec inversion too slow
        data.sim.seq.type = SIM_ODE;

	md_zfill(N, in_dims, src, 1.0);

	struct nlop_s* op_bloch = nlop_bloch_create(N, all_dims, map_dims, out_dims, in_dims, NULL, NULL, &data);

	nlop_apply(op_bloch, N, out_dims, dst, N, in_dims, src);

	float err = linop_test_adjoint_real(nlop_get_derivative(op_bloch, 0, 0));

	nlop_free(op_bloch);

	md_free(src);
	md_free(dst);

	UT_RETURN_ASSERT(err < 1.E-3);
}

UT_REGISTER_TEST(test_nlop_blochfun);


static bool test_nlop_T1phyfun(void)
{
	enum { N = 16 };
	long map_dims[N] = { 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long out_dims[N] = { 16, 16, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long in_dims[N] = { 16, 16, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long TI_dims[N] = { 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	complex float* dst = md_alloc(N, out_dims, CFL_SIZE);
	complex float* src = md_alloc(N, in_dims, CFL_SIZE);

	complex float TI[4] = { 0., 1., 2., 3. };

	md_zfill(N, in_dims, src, 1.0);

	struct moba_conf_s data;

	data.sim.seq = simdata_seq_defaults;
        data.sim.voxel = simdata_voxel_defaults;
        data.sim.pulse = simdata_pulse_defaults;
        data.sim.pulse.hs = pulse_hypsec_defaults;
        data.sim.grad = simdata_grad_defaults;
	data.sim.other = simdata_other_defaults;
        data.other = moba_other_defaults;

	struct nlop_s* T1_phy = nlop_T1_phy_create(N, map_dims, out_dims, in_dims, TI_dims, TI, &data);

	nlop_apply(T1_phy, N, out_dims, dst, N, in_dims, src);

	float err = linop_test_adjoint(nlop_get_derivative(T1_phy, 0, 0));

	nlop_free(T1_phy);

	md_free(src);
	md_free(dst);

	UT_RETURN_ASSERT(err < 1.E-3);
}

UT_REGISTER_TEST(test_nlop_T1phyfun);


static bool test_nlop_ir_meco(void)
{
	enum { N = 16 };
	long map_dims[N] = { 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long out_dims[N] = { 16, 16, 1, 1, 1, 4, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1 };
	long in_dims[N] = { 16, 16, 1, 1, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long TI_dims[N] = { 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long TE_dims[N] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1 };

	complex float* dst = md_alloc(N, out_dims, CFL_SIZE);
	complex float* src = md_alloc(N, in_dims, CFL_SIZE);

	complex float TI[4] = { 0., 1., 2., 3. };	// [s]

	complex float TE[5] = { 0.1, .5, 1., 1.5, 2.0 }; // [ms]

	md_zfill(N, in_dims, src, 1.0);

	float scale_fB0[2] = { 22., 6. };

	float scale_others[8] = { 1, 1, 1, 1, 1, 1, 0.1, 0.1 };

	struct nlop_s* ir_meco = nlop_ir_meco_create(N, map_dims, out_dims, in_dims, TI_dims, TI, TE_dims, TE, scale_fB0, scale_others);

	nlop_apply(ir_meco, N, out_dims, dst, N, in_dims, src);

	float err = linop_test_adjoint_real(nlop_get_derivative(ir_meco, 0, 0));

	nlop_free(ir_meco);

	md_free(src);
	md_free(dst);

	UT_RETURN_ASSERT(err < 1.e-5);
}

UT_REGISTER_TEST(test_nlop_ir_meco);


static bool test_nlop_ir_meco_der(void)
{
	enum { N = 16 };
	long map_dims[N] = { 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long out_dims[N] = { 16, 16, 1, 1, 1, 4, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1 };
	long in_dims[N] = { 16, 16, 1, 1, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long TI_dims[N] = { 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long TE_dims[N] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1 };

	complex float TI[4] = { 0., 1., 2., 3. };	// [s]

	complex float TE[5] = { 0.1, .5, 1., 1.5, 2.0 }; // [ms]

	float scale_fB0[2] = { 22., 6. };

	float scale_others[8] = { 1, 1, 1, 1, 1, 1, 0.1, 0.1 };

	complex float* in= md_alloc(N, in_dims, CFL_SIZE);
	md_zfill(N, in_dims, in, 1.0);

	// create reasonable input, masked to the center and with larger last two coeffs:

	long coeff_dims[N] =  { 1, 1, 1, 1, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	long coeff_strs[N];
	md_calc_strides(N, coeff_strs, coeff_dims, CFL_SIZE);

	long in_strs[N];
	md_calc_strides(N, in_strs, in_dims, CFL_SIZE);

	complex float coeffs[8] = {1., 1., 1., 1., 1., 1., 10., 10.};
	md_zmul2(N, in_dims, in_strs, in, in_strs, in, coeff_strs, coeffs);


	float restrict_dims[N] = { [0 ... N - 1] = 1. };
	restrict_dims[0] = 0.5;
	restrict_dims[1] = 0.5;
	complex float* mask = compute_mask(DIMS, map_dims, restrict_dims);
	long map_strs[N];
	md_calc_strides(N, map_strs, map_dims, CFL_SIZE);

	md_zmul2(N, in_dims, in_strs, in, in_strs, in, map_strs, mask);

	md_free(mask);



	struct nlop_s* ir_meco = nlop_ir_meco_create(N, map_dims, out_dims, in_dims, TI_dims, TI, TE_dims, TE, scale_fB0, scale_others);

	float err = nlop_test_derivative_at(ir_meco, in);

	nlop_free(ir_meco);
	md_free(in);

	const float tol = 6.e-3;
	debug_printf(DP_DEBUG1, "ir_meco_der error: %.8f, tol %.1e\n", err, tol);
	UT_RETURN_ASSERT(err < tol);
}

UT_REGISTER_TEST(test_nlop_ir_meco_der);
