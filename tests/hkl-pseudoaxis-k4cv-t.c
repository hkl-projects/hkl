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
 * Copyright (C) 2003-2019, 2021 Synchrotron SOLEIL
 *                         L'Orme des Merisiers Saint-Aubin
 *                         BP 48 91192 GIF-sur-YVETTE CEDEX
 *
 * Authors: Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
 */
#include "hkl.h"
#include <tap/basic.h>
#include <tap/hkl-tap.h>

static void degenerated(void)
{
	int res = TRUE;
	HklEngineList *engines;
	HklEngine *engine;
	const darray_string *modes;
	const char **mode;
	HklGeometry *geometry;
	HklDetector *detector;
	HklSample *sample;
	static double hkl[] = {0, 1, 0};
	Geometry gconf = K4cv(1.54, VALUES(30., 0., 0., 60.));
        struct Sample cu = CU;

	geometry = newGeometry(gconf);
	engines = newEngines(gconf);
	sample = newSample(cu);

	detector = hkl_detector_factory_new(HKL_DETECTOR_TYPE_0D);

	hkl_engine_list_init(engines, geometry, detector, sample);

	engine = hkl_engine_list_engine_get_by_name(engines, "hkl", NULL);
	modes = hkl_engine_modes_names_get(engine);

	darray_foreach(mode, *modes) {
		const darray_string *parameters;
		HklGeometryList *geometries;
		size_t n_params;

		res &= DIAG(hkl_engine_current_mode_set(engine, *mode, NULL));
		parameters = hkl_engine_parameters_names_get(engine);
		n_params = darray_size(*parameters);
		if(n_params){
			double params[n_params];

			hkl_engine_parameters_values_get(engine, params, n_params, HKL_UNIT_DEFAULT);
			params[0] = 1;
			res &= DIAG(hkl_engine_parameters_values_set(engine, params, n_params, HKL_UNIT_DEFAULT, NULL));
		}

		/* studdy this degenerated case */
		geometries = hkl_engine_pseudo_axis_values_set(engine,
							       hkl, ARRAY_SIZE(hkl),
							       HKL_UNIT_DEFAULT, NULL);
		if (geometries){
			const HklGeometryListItem *item;

			HKL_GEOMETRY_LIST_FOREACH(item, geometries){
				hkl_geometry_set(geometry,
						 hkl_geometry_list_item_geometry_get(item));
				res &= DIAG(check_pseudoaxes(engine, hkl, 3));
			}
			hkl_geometry_list_free(geometries);
		}
	}

	ok(res == TRUE, "degenerated");

	hkl_engine_list_free(engines);
	hkl_detector_free(detector);
	hkl_sample_free(sample);
	hkl_geometry_free(geometry);
}

static void eulerians(void)
{
	int res = TRUE;
	HklEngineList *engines;
	HklEngine *engine;
	const darray_string *modes;
	const char **mode;
	HklGeometry *geometry;
	HklDetector *detector;
	HklSample *sample;
	static double eulerians[] = {0., 90 * HKL_DEGTORAD, 0.};
	Geometry gconf = K4cv(1.54, VALUES(0., 0., 0., 0.));
        struct Sample cu = CU;

	geometry = newGeometry(gconf);
	engines = newEngines(gconf);
	sample = newSample(cu);

	detector = hkl_detector_factory_new(HKL_DETECTOR_TYPE_0D);

	hkl_engine_list_init(engines, geometry, detector, sample);

	engine = hkl_engine_list_engine_get_by_name(engines, "eulerians", NULL);
	modes = hkl_engine_modes_names_get(engine);

	darray_foreach(mode, *modes){
		const darray_string *parameters;
		HklGeometryList *geometries;
		size_t n_params;

		res &= DIAG(hkl_engine_current_mode_set(engine, *mode, NULL));
		parameters = hkl_engine_parameters_names_get(engine);
		n_params = darray_size(*parameters);
		if(n_params){
			double params[n_params];

			hkl_engine_parameters_values_get(engine, params, n_params, HKL_UNIT_DEFAULT);
			params[0] = 1;
			res &= DIAG(hkl_engine_parameters_values_set(engine, params, n_params, HKL_UNIT_DEFAULT, NULL));
		}

		/* studdy this degenerated case */
		geometries = hkl_engine_pseudo_axis_values_set(engine,
							       eulerians, ARRAY_SIZE(eulerians),
							       HKL_UNIT_DEFAULT, NULL);
		if (geometries) {
			const HklGeometryListItem *item;

			/* first solution = -180, -90, 180 */
			item = hkl_geometry_list_items_first_get(geometries);
			hkl_geometry_set(geometry,
					 hkl_geometry_list_item_geometry_get(item));
			res &= DIAG(check_pseudoaxes_v(engine, -180. * HKL_DEGTORAD, -90 * HKL_DEGTORAD, 180. * HKL_DEGTORAD));

			/* second solution = 0, 90, 0 */
			item = hkl_geometry_list_items_next_get(geometries,item);
			hkl_geometry_set(geometry,
					 hkl_geometry_list_item_geometry_get(item));
			res &= DIAG(check_pseudoaxes_v(engine, 0., 90 * HKL_DEGTORAD, 0.));

			/* no more solution */
			res &= DIAG(hkl_geometry_list_items_next_get(geometries, item) == NULL);

			hkl_geometry_list_free(geometries);
		}
	}

	ok(res == TRUE, "eulerians");

	hkl_engine_list_free(engines);
	hkl_detector_free(detector);
	hkl_sample_free(sample);
	hkl_geometry_free(geometry);
}

static void q(void)
{
	int res = TRUE;
	HklEngineList *engines;
	HklEngine *engine;
	const darray_string *modes;
	const char **mode;
	HklGeometry *geometry;
	HklDetector *detector;
	HklSample *sample;
	Geometry gconf = K4cv(1.54, VALUES(30., 0., 0., 60.));
        struct Sample cu = CU;

	geometry = newGeometry(gconf);
	engines = newEngines(gconf);
	sample = newSample(cu);

	detector = hkl_detector_factory_new(HKL_DETECTOR_TYPE_0D);

	hkl_engine_list_init(engines, geometry, detector, sample);

	engine = hkl_engine_list_engine_get_by_name(engines, "q", NULL);
	modes = hkl_engine_modes_names_get(engine);

	/* the init part */
	res &= DIAG(hkl_geometry_set_values_v(geometry, HKL_UNIT_USER, NULL, 30., 0., 0., 60.));
	res &= DIAG(hkl_engine_initialized_set(engine, TRUE, NULL));

	darray_foreach(mode, *modes){
		double q;

		res &= DIAG(hkl_engine_current_mode_set(engine, *mode, NULL));
		for(q=-1.; q<1.; q += 0.1){
			HklGeometryList *geometries = NULL;

			geometries = hkl_engine_set_values_v(engine, q, NULL);
			if(NULL != geometries){
				const HklGeometryListItem *item;

				HKL_GEOMETRY_LIST_FOREACH(item, geometries){
					hkl_geometry_set(geometry,
							 hkl_geometry_list_item_geometry_get(item));
					res &= DIAG(check_pseudoaxes_v(engine, q));
				}
				hkl_geometry_list_free(geometries);
			}
		}
	}

	ok(res == TRUE, "q");

	hkl_engine_list_free(engines);
	hkl_detector_free(detector);
	hkl_sample_free(sample);
	hkl_geometry_free(geometry);
}

int main(void)
{
	plan(3);

	degenerated();
	eulerians();
	q();

	return 0;
}
