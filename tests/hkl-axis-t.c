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
 * Copyright (C) 2003-2019 Synchrotron SOLEIL
 *                         L'Orme des Merisiers Saint-Aubin
 *                         BP 48 91192 GIF-sur-YVETTE CEDEX
 *
 * Authors: Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
 */
#include "hkl.h"
#include <tap/basic.h>
#include <tap/float.h>
#include <tap/hkl-tap.h>

#include <hkl-axis-private.h>

static void new(void)
{
	HklParameter *axis;
	static HklVector v = {{1, 0, 0}};
	double min, max;

	axis = hkl_parameter_new_rotation("rotation", &v, &hkl_unit_angle_deg);
	is_string("rotation", hkl_parameter_name_get(axis), __func__);
	hkl_parameter_min_max_get(axis, &min, &max, HKL_UNIT_DEFAULT);
	is_double(-M_PI, min, HKL_EPSILON, __func__);
	is_double(M_PI, max, HKL_EPSILON, __func__);
	is_double(0., hkl_parameter_value_get(axis, HKL_UNIT_DEFAULT), HKL_EPSILON, __func__);
	ok(TRUE == hkl_parameter_fit_get(axis), __func__);
	hkl_parameter_free(axis);

	axis = hkl_parameter_new_translation("translation", &v, &hkl_unit_length_mm);
	is_string("translation", hkl_parameter_name_get(axis), __func__);
	hkl_parameter_min_max_get(axis, &min, &max, HKL_UNIT_DEFAULT);
	is_double(-DBL_MAX, min, HKL_EPSILON, __func__);
	is_double(DBL_MAX, max, HKL_EPSILON, __func__);
	is_double(0., hkl_parameter_value_get(axis, HKL_UNIT_DEFAULT), HKL_EPSILON, __func__);
	ok(TRUE == hkl_parameter_fit_get(axis), __func__);
	hkl_parameter_free(axis);
}

static void get_quaternions(void)
{
	static HklVector v_ref = {{1, 0, 0}};
	static HklQuaternion q1_ref = {{1, 0, 0, 0}};
	static HklQuaternion q2_ref = {{M_SQRT1_2, -M_SQRT1_2, 0, 0}};
	HklParameter *axis;

	axis = hkl_parameter_new_rotation("rotation", &v_ref, &hkl_unit_angle_deg);
	is_quaternion(&q1_ref, hkl_parameter_quaternion_get(axis), __func__);
	ok(TRUE == hkl_parameter_value_set(axis, -M_PI_2, HKL_UNIT_DEFAULT, NULL), __func__);
	is_quaternion(&q2_ref, hkl_parameter_quaternion_get(axis), __func__);
	hkl_parameter_free(axis);

	axis = hkl_parameter_new_translation("translation", &v_ref, &hkl_unit_length_mm);
	ok(NULL == hkl_parameter_quaternion_get(axis), __func__);
	hkl_parameter_free(axis);
}

static void copy(void)
{
	static HklVector v = {{1, 0, 0}};
	static HklQuaternion q_ref = {{M_SQRT1_2, -M_SQRT1_2, 0, 0}};
	HklParameter *axis;
	HklParameter *copy;
	double min, max;


	axis = hkl_parameter_new_rotation("rotation", &v, &hkl_unit_angle_deg);
	ok(TRUE == hkl_parameter_value_set(axis, -M_PI_2, HKL_UNIT_DEFAULT, NULL), __func__);
	copy = hkl_parameter_new_copy(axis);
	is_string("rotation", hkl_parameter_name_get(copy), __func__);
	hkl_parameter_min_max_get(copy, &min, &max, HKL_UNIT_DEFAULT);
	is_double(-M_PI, min, HKL_EPSILON, __func__);
	is_double(M_PI, max, HKL_EPSILON, __func__);
	is_double(-M_PI_2, hkl_parameter_value_get(copy, HKL_UNIT_DEFAULT), HKL_EPSILON, __func__);
	ok(TRUE == hkl_parameter_fit_get(copy), __func__);
	is_quaternion(&q_ref, hkl_parameter_quaternion_get(copy), __func__);
	hkl_parameter_free(axis);
	hkl_parameter_free(copy);

	axis = hkl_parameter_new_translation("translation", &v, &hkl_unit_length_mm);
	ok(TRUE == hkl_parameter_value_set(axis, 100, HKL_UNIT_DEFAULT, NULL), __func__);
	copy = hkl_parameter_new_copy(axis);
	is_string("translation", hkl_parameter_name_get(copy), __func__);
	hkl_parameter_min_max_get(copy, &min, &max, HKL_UNIT_DEFAULT);
	is_double(-DBL_MAX, min, HKL_EPSILON, __func__);
	is_double(DBL_MAX, max, HKL_EPSILON, __func__);
	is_double(100, hkl_parameter_value_get(copy, HKL_UNIT_DEFAULT), HKL_EPSILON, __func__);
	ok(TRUE == hkl_parameter_fit_get(copy), __func__);
	ok(NULL == hkl_parameter_quaternion_get(copy), __func__);
	hkl_parameter_free(axis);
	hkl_parameter_free(copy);
}

static void is_valid(void)
{
	static HklVector v = {{1, 0, 0}};
	HklParameter *axis;
	int res = TRUE;

	axis = hkl_parameter_new_rotation("rotation", &v, &hkl_unit_angle_deg);

	res &= DIAG(TRUE == hkl_parameter_value_set(axis, 45, HKL_UNIT_USER, NULL));
	res &= DIAG(TRUE == hkl_parameter_is_valid(axis));
	res &= DIAG(TRUE == hkl_parameter_min_max_set(axis, -270, 0.0, HKL_UNIT_USER, NULL));
	res &= DIAG(FALSE == hkl_parameter_is_valid(axis));
	res &= DIAG(TRUE == hkl_parameter_value_set(axis, -45, HKL_UNIT_USER, NULL));
	res &= DIAG(TRUE == hkl_parameter_is_valid(axis));
	res &= DIAG(TRUE == hkl_parameter_min_max_set(axis, 350, 450, HKL_UNIT_USER, NULL));
	res &= DIAG(TRUE == hkl_parameter_value_set(axis, 45, HKL_UNIT_USER, NULL));
	res &= DIAG(TRUE == hkl_parameter_is_valid(axis));
	res &= DIAG(TRUE == hkl_parameter_value_set(axis, -45, HKL_UNIT_USER, NULL));
	res &= DIAG(FALSE == hkl_parameter_is_valid(axis));
	res &= DIAG(TRUE == hkl_parameter_min_max_set(axis, -10, 90, HKL_UNIT_USER, NULL));
	res &= DIAG(TRUE == hkl_parameter_value_set(axis, 405, HKL_UNIT_USER, NULL));
	res &= DIAG(TRUE == hkl_parameter_is_valid(axis));
	res &= DIAG(TRUE == hkl_parameter_value_set(axis, -405, HKL_UNIT_USER, NULL));
	res &= DIAG(FALSE == hkl_parameter_is_valid(axis));
	hkl_parameter_free(axis);

	axis = hkl_parameter_new_translation("translation", &v, &hkl_unit_length_mm);
	res &= DIAG(TRUE == hkl_parameter_value_set(axis, 45, HKL_UNIT_USER, NULL));
	res &= DIAG(TRUE == hkl_parameter_is_valid(axis));
	res &= DIAG(TRUE == hkl_parameter_min_max_set(axis, -270, 0, HKL_UNIT_USER, NULL));
	res &= DIAG(FALSE == hkl_parameter_is_valid(axis));
	res &= DIAG(TRUE == hkl_parameter_value_set(axis, -45, HKL_UNIT_USER, NULL));
	res &= DIAG(TRUE == hkl_parameter_is_valid(axis));
	hkl_parameter_free(axis);

	ok(res == TRUE, __func__);
}

static void set_value_smallest_in_range(void)
{
	HklParameter *axis;
	static HklVector v = {{1, 0, 0}};
	int res = TRUE;

	axis = hkl_parameter_new_rotation("rotation", &v, &hkl_unit_angle_deg);

	/* can not set a parameter value with a NaN */
	res &= DIAG(FALSE == hkl_parameter_value_set(axis, NAN, HKL_UNIT_USER, NULL));
	res &= DIAG(FALSE == hkl_parameter_value_set(axis, NAN, HKL_UNIT_DEFAULT, NULL));

	res &= DIAG(TRUE == hkl_parameter_min_max_set(axis, -190, 190, HKL_UNIT_USER, NULL));

	res &= DIAG(TRUE == hkl_parameter_value_set(axis, 185, HKL_UNIT_USER, NULL));
	hkl_parameter_value_set_smallest_in_range(axis);
	is_double(-175., hkl_parameter_value_get(axis, HKL_UNIT_USER), HKL_EPSILON, __func__);

	res &= DIAG(TRUE == hkl_parameter_value_set(axis, 545, HKL_UNIT_USER, NULL));
	hkl_parameter_value_set_smallest_in_range(axis);
	is_double(-175., hkl_parameter_value_get(axis, HKL_UNIT_USER), HKL_EPSILON, __func__);

	res &= DIAG(TRUE == hkl_parameter_value_set(axis, -185, HKL_UNIT_USER, NULL));
	hkl_parameter_value_set_smallest_in_range(axis);
	is_double(-185., hkl_parameter_value_get(axis, HKL_UNIT_USER), HKL_EPSILON, __func__);

	res &= DIAG(TRUE == hkl_parameter_value_set(axis, 175, HKL_UNIT_USER, NULL));
	hkl_parameter_value_set_smallest_in_range(axis);
	is_double(-185., hkl_parameter_value_get(axis, HKL_UNIT_USER), HKL_EPSILON, __func__);

	res &= DIAG(TRUE == hkl_parameter_value_set(axis, 190, HKL_UNIT_USER, NULL));
	hkl_parameter_value_set_smallest_in_range(axis);
	is_double(-170., hkl_parameter_value_get(axis, HKL_UNIT_USER), HKL_EPSILON, __func__);

	res &= DIAG(TRUE == hkl_parameter_value_set(axis, -190, HKL_UNIT_USER, NULL));
	hkl_parameter_value_set_smallest_in_range(axis);
	is_double(-190., hkl_parameter_value_get(axis, HKL_UNIT_USER), HKL_EPSILON, __func__);

	hkl_parameter_free(axis);

	ok(res == TRUE, __func__);
}

static void get_value_closest(void)
{
	HklParameter *axis1, *axis2;
	static HklVector v = {{1, 0, 0}};

	axis1 = hkl_parameter_new_rotation("rotation", &v, &hkl_unit_angle_deg);
	axis2 = hkl_parameter_new_rotation("rotation", &v, &hkl_unit_angle_deg);

	ok(TRUE == hkl_parameter_value_set(axis1, 0, HKL_UNIT_USER, NULL), __func__);
	ok(TRUE == hkl_parameter_value_set(axis2, 0, HKL_UNIT_USER, NULL), __func__);
	is_double(0., hkl_parameter_value_get_closest(axis1,
						      axis2),
		  HKL_EPSILON, __func__);

	/* change the range of axis1 */
	ok(TRUE == hkl_parameter_min_max_set(axis1, -270, 180, HKL_UNIT_USER, NULL), __func__);
	ok(TRUE == hkl_parameter_value_set(axis1, 100, HKL_UNIT_USER, NULL), __func__);

	ok(TRUE == hkl_parameter_value_set(axis2, -75, HKL_UNIT_USER, NULL), __func__);
	is_double(100 * HKL_DEGTORAD, hkl_parameter_value_get_closest(axis1,
								      axis2),
		  HKL_EPSILON, __func__);
	ok(TRUE == hkl_parameter_value_set(axis2, -85, HKL_UNIT_USER, NULL), __func__);
	is_double(-260 * HKL_DEGTORAD, hkl_parameter_value_get_closest(axis1,
								       axis2),
		  HKL_EPSILON, __func__);

	hkl_parameter_free(axis2);
	hkl_parameter_free(axis1);
}

static void transformation_cmp(void)
{
	int res = TRUE;
	HklParameter *axis1, *axis2, *translation1, *translation2;
	static HklVector v1 = {{1, 0, 0}};
	static HklVector v2 = {{0, 1, 0}};

	axis1 = hkl_parameter_new_rotation("rotation", &v1, &hkl_unit_angle_deg);
	axis2 = hkl_parameter_new_rotation("rotation", &v2, &hkl_unit_angle_deg);
	translation1 = hkl_parameter_new_translation("translation", &v1, &hkl_unit_length_mm);
	translation2 = hkl_parameter_new_translation("translation", &v2, &hkl_unit_length_mm);

	res &= DIAG(0 == hkl_parameter_transformation_cmp(axis1, axis1));
	res &= DIAG(0 != hkl_parameter_transformation_cmp(axis1, axis2));
	res &= DIAG(0 != hkl_parameter_transformation_cmp(axis1, translation1));
	res &= DIAG(0 != hkl_parameter_transformation_cmp(axis1, translation2));
	res &= DIAG(0 != hkl_parameter_transformation_cmp(axis2, translation1));
	res &= DIAG(0 != hkl_parameter_transformation_cmp(axis2, translation2));
	res &= DIAG(0 == hkl_parameter_transformation_cmp(translation1, translation1));
	res &= DIAG(0 != hkl_parameter_transformation_cmp(translation1, translation2));
	res &= DIAG(0 != hkl_parameter_transformation_cmp(translation1, axis1));
	res &= DIAG(0 != hkl_parameter_transformation_cmp(translation1, axis2));
	res &= DIAG(0 != hkl_parameter_transformation_cmp(translation2, axis1));
	res &= DIAG(0 != hkl_parameter_transformation_cmp(translation2, axis2));

	ok(res == TRUE, __func__);

	hkl_parameter_free(translation2);
	hkl_parameter_free(translation1);
	hkl_parameter_free(axis2);
	hkl_parameter_free(axis1);
}

int main(void)
{
	plan(46);

	new();
	get_quaternions();
	copy();
	is_valid();
	set_value_smallest_in_range();
	get_value_closest();
	transformation_cmp();

	return 0;
}
