/* Copyright 2017. Martin Uecker.
 * All rights reserved. Use of this source code is governed by
 * a BSD-style license which can be found in the LICENSE file.
 *
 * 2017 Martin Uecker <martin.uecker@med.uni-goettingen.de>
 *
 * Simple coil using Biot-Savart law.
 *
 */

#include <assert.h>
#include <complex.h>
#include <math.h>

#include "misc/misc.h"

#include "simu/biot_savart.h"

#include "coil.h"


const struct coil coil_defaults = { .size = 0.2, .dist = 1.5, };


complex float coil(const struct coil* data, float x[3], int N, int i)
{
	assert(i < N);

	float dist = data->dist;

	vec3_t c[50];
	vec3_t nc1 = { dist * sinf(2. * M_PI * i / N), dist * cosf(2. * M_PI * i / N), 0. };
	vec3_t nc2 = { sinf(2. * M_PI * i / N), cosf(2. * M_PI * i / N), 0. };
	vec3_ring(50, c, nc1, nc2, data->size);

	vec3_t b;
	biot_savart(b, x, 50, (const vec3_t*)c);

	return b[0] + 1.i * b[1];
}


