#include <tap/basic.h>
#include <tap/hkl-tap.h>

/*
Initial test to check that new geometries are recognized and functional.
E4CGV and E4CGV2 are simplified forms of E4CV with an added gamma axis. For gamma=0, results should be the same as E4CV
*/


static void register_and_setup(void)
{
    int res = TRUE;
    Geometry gconf = E4CGV2(1.54, VALUES(30., 0., 90., 60., 0.));
    HklGeometry *geometry = newGeometry(gconf);
    HklDetector *detector = hkl_detector_factory_new(HKL_DETECTOR_TYPE_0D);
    HklSample *sample = newSample(CU);
    HklEngineList *engines = newEngines(gconf);

    hkl_engine_list_init(engines, geometry, detector, sample);

    HklEngine *engine = hkl_engine_list_engine_get_by_name(engines, "hkl", NULL);
    res &= DIAG(hkl_geometry_set_values_v(geometry, HKL_UNIT_USER, NULL, 30., 0., 90., 60., 0.));
    res &= DIAG(check_pseudoaxes_v(engine, 1., 0., 0.));

    ok(res == TRUE, "E4CVG: bissector mode computes correct pseudoaxes");

    hkl_engine_list_free(engines);
    hkl_sample_free(sample);
    hkl_detector_free(detector);
    hkl_geometry_free(geometry);
}

int main(void)
{
    plan(1);
    register_and_setup();
    return 0;
}

