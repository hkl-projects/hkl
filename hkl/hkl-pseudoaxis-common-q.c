/* This file is part of the hkl library.
 *
 * The hkl library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The hkl library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the hkl library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2003-2019, 2022 Synchrotron SOLEIL
 *                         L'Orme des Merisiers Saint-Aubin
 *                         BP 48 91192 GIF-sur-YVETTE CEDEX
 *
 * Authors: Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
 *          Jens Krüger <Jens.Krueger@frm2.tum.de>
 */
#include <gsl/gsl_errno.h>              // for ::GSL_SUCCESS
#include <gsl/gsl_sf_trig.h>            // for gsl_sf_angle_restrict_symm
#include <gsl/gsl_sys.h>                // for gsl_isnan
#include <gsl/gsl_vector_double.h>      // for gsl_vector
#include <math.h>                       // for sin, atan2, signbit
#include <stdlib.h>                     // for free
#include "hkl-detector-private.h"       // for hkl_detector_compute_kf
#include "hkl-geometry-private.h"       // for _HklGeometry, HklHolder
#include "hkl-macros-private.h"         // for HKL_MALLOC
#include "hkl-parameter-private.h"      // for _HklParameter, etc
#include "hkl-pseudoaxis-auto-private.h"  // for HklFunction, etc
#include "hkl-pseudoaxis-common-q-private.h"  // for HklEngineQ2, etc
#include "hkl-pseudoaxis-common-readonly-private.h"
#include "hkl-pseudoaxis-private.h"     // for _HklEngine, etc
#include "hkl-source-private.h"         // for hkl_source_compute_ki, etc
#include "hkl-vector-private.h"         // for HklVector, hkl_vector_angle, etc
#include "hkl.h"                        // for HklEngine, HklParameter, etc
#include "hkl/ccan/array_size/array_size.h"  // for ARRAY_SIZE
#include "hkl/ccan/container_of/container_of.h"  // for container_of
#include "hkl/ccan/darray/darray.h"     // for darray_item

#define GAMMA "gamma"
#define DELTA "delta"

typedef struct _HklEngineQ HklEngineQ;
typedef struct _HklEngineQ2 HklEngineQ2;
typedef struct _HklEngineQperQpar HklEngineQperQpar;

double qmax(double wavelength)
{
	return 2 * HKL_TAU / wavelength;
}

/*****/
/* q */
/*****/

struct _HklEngineQ
{
	HklEngine engine;
	HklParameter *q;
};

static int _q_func(const gsl_vector *x, void *params, gsl_vector *f)
{
	double q;
	HklEngine *engine = params;
	const HklEngineQ *engine_q = container_of(engine, HklEngineQ, engine);
	double tth;

	CHECK_NAN(x->data, x->size);

	/* update the workspace from x */
	set_geometry_axes(engine, x->data);

	tth = gsl_sf_angle_restrict_symm(x->data[0]);
	q = qmax(hkl_source_get_wavelength(&engine->geometry->source)) * sin(tth/2.);

	f->data[0] = engine_q->q->_value - q;

	return  GSL_SUCCESS;
}

static const HklFunction q_func = {
	.function = _q_func,
	.size = 1,
};

static int get_q_real(HklMode *self,
		      HklEngine *base,
		      HklGeometry *geometry,
		      HklDetector *detector,
		      HklSample *sample,
		      GError **error)
{
	double wavelength;
	double theta;
	HklVector ki, kf;
	HklEngineQ *engine = container_of(base, HklEngineQ, engine);

	wavelength = hkl_source_get_wavelength(&geometry->source);
	ki = hkl_geometry_ki_get(geometry);
	kf = hkl_geometry_kf_get(geometry, detector);
	theta = hkl_vector_angle(&ki, &kf) / 2.;

	/* we decide of the sign of theta depending on the orientation
	 * of kf in the direct-space */
	if(kf.data[1] < 0 || kf.data[2] < 0)
		theta = -theta;

	/* update q */
	engine->q->_value = qmax(wavelength) * sin(theta);

	return TRUE;
}

/* not declared in the constructor as it is used also in the q2 pseudo
 * axis engine */
static const HklParameter q = {
	HKL_PARAMETER_DEFAULTS, .name="q",
	.description = "the norm of $\\vec{q}$",
	.range = { .max=1 },
};

static HklMode *mode_q(void)
{
	static const char *axes[] = {"tth"};
	static const HklFunction *functions[] = {&q_func};
	static HklModeAutoInfo info = {
		HKL_MODE_AUTO_INFO("q", axes, axes, functions),
	};
	static const HklModeOperations operations = {
		HKL_MODE_OPERATIONS_AUTO_DEFAULTS,
		.get = get_q_real,
	};

	return hkl_mode_auto_new(&info, &operations, TRUE);
}

static void hkl_engine_q_free_real(HklEngine *base)
{
	HklEngineQ *self=container_of(base, HklEngineQ, engine);
	hkl_engine_release(&self->engine);
	free(self);
}

HklEngine *hkl_engine_q_new(HklEngineList *engines)
{
	HklEngineQ *self = g_new(HklEngineQ, 1);
	HklMode *mode;
	static const HklParameter *pseudo_axes[] = {&q};
	static const HklEngineInfo info = {
		HKL_ENGINE_INFO("q",
				pseudo_axes,
				HKL_ENGINE_DEPENDENCIES_AXES | HKL_ENGINE_DEPENDENCIES_ENERGY),
	};
	static const HklEngineOperations operations = {
		HKL_ENGINE_OPERATIONS_DEFAULTS,
		.free=hkl_engine_q_free_real,
	};

	hkl_engine_init(&self->engine, &info, &operations, engines);
	self->q = register_pseudo_axis(&self->engine, engines, &q);

	/* q [default] */
	mode = mode_q();
	hkl_engine_add_mode(&self->engine, mode);
	hkl_engine_mode_set(&self->engine, mode);

	return &self->engine;
}

/******/
/* q2 */
/******/

struct _HklEngineQ2
{
	HklEngine engine;
	HklParameter *q;
	HklParameter *alpha;
};

static void _q2(HklGeometry *geometry, HklDetector *detector,
		double *q, double *alpha)
{
	double wavelength, theta;
	HklVector kf, ki;
	static HklVector x = {
		.data = {1, 0, 0},
	};

	wavelength = hkl_source_get_wavelength(&geometry->source);
	ki = hkl_geometry_ki_get(geometry);
	kf = hkl_geometry_kf_get(geometry, detector);
	theta = hkl_vector_angle(&ki, &kf) / 2.;

	*q = qmax(wavelength) * sin(theta);

	/* project kf on the x plan to compute alpha */
	hkl_vector_project_on_plan(&kf, &x);

	*alpha = atan2(kf.data[2], kf.data[1]);
}

static int _q2_func(const gsl_vector *x, void *params, gsl_vector *f)
{
	HklEngine *engine = params;
	const HklEngineQ2 *engine_q2 = container_of(engine, HklEngineQ2, engine);
	double q;
	double alpha;

	CHECK_NAN(x->data, x->size);

	/* update the workspace from x */
	set_geometry_axes(engine, x->data);

	_q2(engine->geometry, engine->detector, &q, &alpha);

	f->data[0] = engine_q2->q->_value - q;
	f->data[1] = engine_q2->alpha->_value - alpha;

	return  GSL_SUCCESS;
}

static const HklFunction q2_func = {
	.function = _q2_func,
	.size = 2,
};

static int get_q2_real(HklMode *self,
		       HklEngine *engine,
		       HklGeometry *geometry,
		       HklDetector *detector,
		       HklSample *sample,
		       GError **error)
{
	HklEngineQ2 *engine_q2 = container_of(engine, HklEngineQ2, engine);

	_q2(geometry, detector, &engine_q2->q->_value, &engine_q2->alpha->_value);

	return TRUE;
}

static HklMode *mode_q2(void)
{
	static const char* axes[] = {GAMMA, DELTA};
	static const HklFunction *functions[] = {&q2_func};
	static const HklModeAutoInfo info = {
		HKL_MODE_AUTO_INFO("q2", axes, axes, functions),
	};
	static const HklModeOperations operations = {
		HKL_MODE_OPERATIONS_AUTO_DEFAULTS,
		.get = get_q2_real,
	};

	return hkl_mode_auto_new(&info, &operations, TRUE);
}

static const HklParameter alpha = {
	HKL_PARAMETER_DEFAULTS_ANGLE, .name = "alpha",
	.description = "angle of the projection of $\\vec{q}$ on the $yOz$ plan and $\\vec{y}$",
};

static void hkl_engine_q2_free_real(HklEngine *base)
{
	HklEngineQ2 *self = container_of(base, HklEngineQ2, engine);
	hkl_engine_release(&self->engine);
	free(self);
}

HklEngine *hkl_engine_q2_new(HklEngineList *engines)
{
	HklEngineQ2 *self = g_new(HklEngineQ2, 1);
	HklMode *mode;
	static const HklParameter *pseudo_axes[] = {&q, &alpha};
	static const HklEngineInfo info = {
		HKL_ENGINE_INFO("q2",
				pseudo_axes,
				HKL_ENGINE_DEPENDENCIES_AXES | HKL_ENGINE_DEPENDENCIES_ENERGY),
	};
	static const HklEngineOperations operations = {
		HKL_ENGINE_OPERATIONS_DEFAULTS,
		.free=hkl_engine_q2_free_real,
	};

	hkl_engine_init(&self->engine, &info, &operations, engines);
	self->q = register_pseudo_axis(&self->engine, engines, &q);
	self->alpha = register_pseudo_axis(&self->engine, engines, &alpha);

	/* q2 [default] */
	mode = mode_q2();
	hkl_engine_add_mode(&self->engine, mode);
	hkl_engine_mode_set(&self->engine, mode);

	return &self->engine;
}

/************/
/* QperQpar */
/************/

typedef struct _HklModeIncidence HklModeQperQpar;

struct _HklEngineQperQpar
{
	HklEngine engine;
	HklParameter *qper;
	HklParameter *qpar;
};

static void _qper_qpar(HklEngine *engine,
		       const HklGeometry *geometry,
		       const HklDetector *detector,
		       const HklSample *sample,
		       double *qper, double *qpar)
{
	HklModeQperQpar *mode = container_of(engine->mode, HklModeQperQpar, parent);
	HklHolder *sample_holder = hkl_geometry_sample_holder_get(geometry, sample);
	HklVector ki;
	HklVector q;
	HklVector n = {
		.data = {
			mode->n_x->_value,
			mode->n_y->_value,
			mode->n_z->_value,
		},
	};
	HklVector npar;
	HklVector qper_v;
	HklVector qpar_v;
	double norm;

	/* compute q = kf - ki */
	ki = hkl_geometry_ki_get(geometry);
	q = hkl_geometry_kf_get(geometry, detector);
	hkl_vector_minus_vector(&q, &ki);

	/* compute the real orientation of the surface n */
	hkl_vector_rotated_quaternion(&n, &sample_holder->q);
	hkl_vector_normalize(&n);

	/* compute the npar used to define the sign of qpar */
	npar = ki;
	hkl_vector_vectorial_product(&npar, &n);

	/* qper */
	qper_v = n;
	norm = hkl_vector_scalar_product(&q, &n);
	hkl_vector_times_double(&qper_v, norm);
	*qper = hkl_vector_norm2(&qper_v);
	if (signbit(norm))
		*qper *= -1;

	/* qpar */
	qpar_v = q;
	norm = hkl_vector_scalar_product(&q, &npar);
	hkl_vector_minus_vector(&qpar_v, &qper_v);
	*qpar = hkl_vector_norm2(&qpar_v);
	if (signbit(norm))
		*qpar *= -1;
}

static int _qper_qpar_func(const gsl_vector *x, void *params, gsl_vector *f)
{
	HklEngine *engine = params;
	const HklEngineQperQpar *engine_qper_qpar = container_of(engine, HklEngineQperQpar, engine);
	double qper;
	double qpar;

	CHECK_NAN(x->data, x->size);

	/* update the workspace from x */
	set_geometry_axes(engine, x->data);

	_qper_qpar(engine, engine->geometry, engine->detector, engine->sample,
		   &qper, &qpar);

	f->data[0] = engine_qper_qpar->qper->_value - qper;
	f->data[1] = engine_qper_qpar->qpar->_value - qpar;

	return  GSL_SUCCESS;
}

static const HklFunction qper_qpar_func = {
	.function = _qper_qpar_func,
	.size = 2,
};

static int get_qper_qpar_real(HklMode *self,
			      HklEngine *engine,
			      HklGeometry *geometry,
			      HklDetector *detector,
			      HklSample *sample,
			      GError **error)
{
	HklEngineQperQpar *engine_qper_qpar = container_of(engine, HklEngineQperQpar, engine);

	_qper_qpar(engine, geometry, detector, sample,
		   &engine_qper_qpar->qper->_value,
		   &engine_qper_qpar->qpar->_value);

	return TRUE;
}

static HklMode *mode_qper_qpar(void)
{
	static const char* axes[] = {GAMMA, DELTA};
	static const HklFunction *functions[] = {&qper_qpar_func};
	static const HklParameter parameters[] = {
		SURFACE_PARAMETERS(0, 1, 0),
	};
	static const HklModeAutoInfo info = {
		HKL_MODE_AUTO_INFO_WITH_PARAMS("qper_qpar", axes, axes, functions, parameters),
	};
	static const HklModeOperations operations = {
		HKL_MODE_OPERATIONS_AUTO_DEFAULTS,
		.get = get_qper_qpar_real,
	};

	HklModeQperQpar *self = g_new(HklModeQperQpar, 1);

	/* the base constructor; */
	hkl_mode_auto_init(&self->parent,
			   &info,
			   &operations, TRUE);

	self->n_x = register_mode_parameter(&self->parent, 0);
	self->n_y = register_mode_parameter(&self->parent, 1);
	self->n_z = register_mode_parameter(&self->parent, 2);

	return &self->parent;
}

static void hkl_engine_qper_qpar_free_real(HklEngine *base)
{
	HklEngineQperQpar *self = container_of(base, HklEngineQperQpar, engine);
	hkl_engine_release(&self->engine);
	free(self);
}

HklEngine *hkl_engine_qper_qpar_new(HklEngineList *engines)
{
	HklEngineQperQpar *self = g_new(HklEngineQperQpar, 1);
	HklMode *mode;
	static const HklParameter qper = {
		HKL_PARAMETER_DEFAULTS, .name = "qper",
		.description = "perpendicular component of $\\vec{q}$ along the normal of the sample surface",
		.range = { .min=-1, .max=1 },
	};
	static const HklParameter qpar = {
		HKL_PARAMETER_DEFAULTS, .name = "qpar",
		.description = "parallel component of $\\vec{q}$",
		.range = { .min=-1, .max=1 },
	};
	static const HklParameter *pseudo_axes[] = {&qper, &qpar};
	static const HklEngineInfo info = {
		HKL_ENGINE_INFO("qper_qpar",
				pseudo_axes,
				HKL_ENGINE_DEPENDENCIES_AXES | HKL_ENGINE_DEPENDENCIES_ENERGY),
	};
	static const HklEngineOperations operations = {
		HKL_ENGINE_OPERATIONS_DEFAULTS,
		.free = hkl_engine_qper_qpar_free_real,
	};

	hkl_engine_init(&self->engine, &info, &operations, engines);
	self->qper = register_pseudo_axis(&self->engine, engines, &qper);
	self->qpar = register_pseudo_axis(&self->engine, engines, &qpar);

	/* qper_qpar [default] */
	mode = mode_qper_qpar();
	hkl_engine_add_mode(&self->engine, mode);
	hkl_engine_mode_set(&self->engine, mode);

	return &self->engine;
}
