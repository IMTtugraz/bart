/* Copyright 2013-2014. The Regents of the University of California.
 * Copyright 2017. University of Oxford.
 * All rights reserved. Use of this source code is governed by 
 * a BSD-style license which can be found in the LICENSE file.
 */

#ifndef __THRESH
#define __THRESH

#include "misc/cppwrap.h"


struct operator_p_s;
extern const struct operator_p_s* prox_thresh_create(int D, const long dim[__VLA(D)], const float lambda, const unsigned long flags);

extern const struct operator_p_s* prox_niht_thresh_create(int D, const long dim[D], int k, const unsigned long flags);

extern void thresh_free(const struct operator_p_s* data);
	
extern void set_thresh_lambda(const struct operator_p_s* o, const float lambda);

extern float get_thresh_lambda(const struct operator_p_s* o);


struct linop_s;

extern const struct operator_p_s* prox_unithresh_create(int D, const struct linop_s* unitary_op, const float lambda, const unsigned long flags);


#include "misc/cppwrap.h"


#endif


