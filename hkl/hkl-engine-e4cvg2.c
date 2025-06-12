#include "hkl-factory-private.h"        // for autodata_factories_, etc
#include "hkl-pseudoaxis-common-hkl-private.h"  // for hkl_engine_hkl_new, etc
#include "hkl-pseudoaxis-common-q-private.h"  // for hkl_engine_q2_new, etc
#include "hkl-pseudoaxis-common-tth-private.h"  // for hkl_engine_tth2_new, etc
#include "hkl-pseudoaxis-common-readonly-private.h"

#define OMEGA "omega"
#define PHI "phi"
#define CHI "chi"
#define TTH "tth"
#define GAMMA "gamma"

/************/
/* mode hkl */
/************/

static int _bissector_func(const gsl_vector *x, void *params, gsl_vector *f)
{
    const double omega = x->data[0];
    const double tth = x->data[3];

    CHECK_NAN(x->data, x->size);

    RUBh_minus_Q(x->data, params, f->data);
    f->data[3] = tth - 2 * fmod(omega,M_PI);

    return  GSL_SUCCESS;
}

static const HklFunction bissector_func = {
    .function = _bissector_func,
    .size = 4,
};

static HklMode *bissector(void)
{
    static const char* axes[] = {OMEGA, CHI, PHI, TTH};
    static const HklFunction *functions[] = {&bissector_func};
    static const HklModeAutoInfo info = {
        HKL_MODE_AUTO_INFO(__func__, axes, axes, functions),
    };

    return hkl_mode_auto_new(&info,
                 &hkl_full_mode_operations,
                 TRUE);
}

static HklEngine *hkl_engine_e4cgv2_hkl_new(HklEngineList *engines)
{
	HklEngine *self;
	HklMode *default_mode;

	self = hkl_engine_hkl_new(engines);

	default_mode = bissector()();
	hkl_engine_add_mode(self, default_mode);
	hkl_engine_mode_set(self, default_mode);

	return self;
}

/*****************/
/* mode readonly */
/*****************/

REGISTER_READONLY_INCIDENCE(hkl_engine_e4cgv2_incidence_new,
			    P99_PROTECT({BASEPITCH, THETAH, ALPHAY, ALPHAX}),
			    surface_parameters_z);

REGISTER_READONLY_EMERGENCE(hkl_engine_soleil_sirius_turret_emergence_new,
			    P99_PROTECT({BASEPITCH, THETAH, ALPHAY, ALPHAX, DELTA, GAMMA}),
			    surface_parameters_z);

/************************/
/*       E4CVG2         */
/************************/

#define HKL_GEOMETRY_TYPE_E4CVG2_DESCRIPTION		\
	"+ xrays source fix allong the :math:`\\vec{x}` direction (1, 0, 0)\n" \
	"\ntest 4-circle with gamma out-of-place crystal orientation alignment" \
	"\n"

static const char* hkl_geometry_e4cvg2_axes[] = {OMEGA, PHI, CHI, TTH, GAMMA};

static HklGeometry *hkl_geometry_new_e4cvg2(const HklFactory *factory)
{
	HklGeometry *self = hkl_geometry_new(factory, &hkl_geometry_operations_defaults);
	HklHolder *h;

	h = hkl_geometry_add_holder(self);
	hkl_holder_add_rotation(h, OMEGA, 0, -1, 0, &hkl_unit_angle_mrad);
	hkl_holder_add_rotation(h, PHI, 1, 0, 0, &hkl_unit_angle_deg);
	hkl_holder_add_rotation(h, CHI, 0, -1, 0, &hkl_unit_angle_deg);

	h = hkl_geometry_add_holder(self);
	hkl_holder_add_rotation(h, TTH, 0, -1, 0, &hkl_unit_angle_deg);
	hkl_holder_add_rotation(h, GAMMA, 0, 0, 1, &hkl_unit_angle_mrad);
	return self;
}

static HklEngineList *hkl_engine_list_new_e4cvg2(const HklFactory *factory)
{
	HklEngineList *self = hkl_engine_list_new();

	hkl_engine_e4cvg2_hkl_new(self);
	hkl_engine_q2_new(self);
	hkl_engine_qper_qpar_new(self);
	hkl_engine_tth2_new(self);
	hkl_engine_e4cvg2_incidence_new(self);
	hkl_engine_e4cvg2_emergence_new(self);

	return self;
}

REGISTER_DIFFRACTOMETER(e4cvg2, "E4CVG2", HKL_GEOMETRY_TYPE_E4CVG2_DESCRIPTION);
