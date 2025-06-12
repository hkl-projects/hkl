#include <gsl/gsl_sys.h>                // for gsl_isnan
#include "hkl-factory-private.h"        // for autodata_factories_, etc
#include "hkl-pseudoaxis-common-hkl-private.h"  // for hkl_mode_operations, etc
#include "hkl-pseudoaxis-common-psi-private.h"  // for hkl_engine_psi_new, etc
#include "hkl-pseudoaxis-common-q-private.h"  // for hkl_engine_q2_new, etc
#include "hkl-pseudoaxis-common-tth-private.h"  // for hkl_engine_tth2_new, etc
#include "hkl-pseudoaxis-common-readonly-private.h"

/**************/
/* Axes names */
/**************/

#define OMEGA "omega"
#define CHI "chi"
#define PHI "phi"
#define TTH "tth"
#define GAMMA "gamma"

/************/
/* Geometry */
/************/

#define HKL_GEOMETRY_EULERIAN4CVG_DESCRIPTION				\
	"+ xrays source fix allong the :math:`\\vec{x}` direction (1, 0, 0)\n" \
	"+ 4 axes for the sample\n"					\
	"\n"								\
	"  + **" OMEGA "** : rotating around the :math:`-\\vec{y}` direction (0, -1, 0)\n" \
	"  + **" CHI "** : rotating around the :math:`\\vec{x}` direction (1, 0, 0)\n" \
	"  + **" PHI "** : rotating around the :math:`-\\vec{y}` direction (0, -1, 0)\n" \
   "\n" \
	"+ 2 axes for the detector\n"					\
	"\n"								\
	"  + **" TTH "** : rotating around the :math:`-\\vec{y}` direction (0, -1, 0)\n" \
	"\n"								\
	"  + **" GAMMA "** : rotation around the :math:`\\vec{z}` direction (0, 0, 1)\n"

static const char* hkl_geometry_eulerian4CVG_axes[] = {OMEGA, CHI, PHI, TTH, GAMMA};

static HklGeometry *hkl_geometry_new_eulerian4CVG(const HklFactory *factory)
{
	HklGeometry *self = hkl_geometry_new(factory, &hkl_geometry_operations_defaults);
	HklHolder *h;

	h = hkl_geometry_add_holder(self);
	hkl_holder_add_rotation(h, OMEGA, 0, -1, 0, &hkl_unit_angle_deg);
	hkl_holder_add_rotation(h, CHI, 1, 0, 0, &hkl_unit_angle_deg);
	hkl_holder_add_rotation(h, PHI, 0, -1, 0, &hkl_unit_angle_deg);

	h = hkl_geometry_add_holder(self);
	hkl_holder_add_rotation(h, TTH, 0, -1, 0, &hkl_unit_angle_deg);
	hkl_holder_add_rotation(h, GAMMA, 0, 0, 1, &hkl_unit_angle_deg);

	return self;
}

static HklEngineList *hkl_engine_list_new_eulerian4CVG(const HklFactory *factory)
{
	HklEngineList *self = hkl_engine_list_new();

	hkl_engine_e4cvg_hkl_new(self);
	hkl_engine_e4cvg_psi_new(self);
	hkl_engine_q_new(self);
	hkl_engine_e4cvg_incidence_new(self);
	hkl_engine_e4cvg_emergence_new(self);

	return self;
}

/*********/
/* Modes */
/*********/

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
	static const char* axes[] = {OMEGA, CHI, PHI, TTH, GAMMA};
	static const HklFunction *functions[] = {&bissector_func};
	static const HklModeAutoInfo info = {
		HKL_MODE_AUTO_INFO(__func__, axes, axes, functions),
	};

	return hkl_mode_auto_new(&info,
				 &hkl_full_mode_operations,
				 TRUE);
}

static HklMode *constant_omega(void)
{
    /* DON'T KNOW HOW TO SET THESE AXES, DESCRIPTION IN TEMPLATE*/
    /* JUST ADDING GAMMA TO axes_r BECAUSE LOOKS TO NAME ALL AXES*/
	static const char* axes_r[] = {OMEGA, CHI, PHI, TTH, GAMMA};
	static const char* axes_w[] = {CHI, PHI, TTH};
	static const HklFunction *functions[] = {&RUBh_minus_Q_func};
	static const HklModeAutoInfo info = {
		HKL_MODE_AUTO_INFO(__func__, axes_r, axes_w, functions),
	};

	return hkl_mode_auto_new(&info,
				 &hkl_full_mode_operations,
				 TRUE);
}

static HklMode *constant_chi(void)
{
	static const char* axes_r[] = {OMEGA, CHI, PHI, TTH, GAMMA};
	static const char* axes_w[] = {OMEGA, PHI, TTH};
	static const HklFunction *functions[] = {&RUBh_minus_Q_func};
	static const HklModeAutoInfo info = {
		HKL_MODE_AUTO_INFO(__func__, axes_r, axes_w, functions),
	};

	return hkl_mode_auto_new(&info,
				 &hkl_full_mode_operations,
				 TRUE);
}

static HklMode *constant_phi(void)
{
	static const char* axes_r[] = {OMEGA, CHI, PHI, TTH, GAMMA};
	static const char* axes_w[] = {OMEGA, CHI, TTH};
	static const HklFunction *functions[] = {&RUBh_minus_Q_func};
	static const HklModeAutoInfo info = {
		HKL_MODE_AUTO_INFO(__func__, axes_r, axes_w, functions),
	};

	return hkl_mode_auto_new(&info,
				 &hkl_full_mode_operations,
				 TRUE);
}

static HklMode *double_diffraction(void)
{
	static const char* axes[] = {OMEGA, CHI, PHI, TTH, GAMMA};
	static const HklFunction *functions[] = {&double_diffraction_func};
	static const HklModeAutoInfo info = {
		HKL_MODE_AUTO_INFO_WITH_PARAMS(__func__, axes, axes,
					       functions, double_diffraction_parameters),
	};

	return hkl_mode_auto_new(&info,
				 &hkl_full_mode_operations,
				 TRUE);
}

static HklMode *psi_constant(void)
{
	static const char* axes[] = {OMEGA, CHI, PHI, TTH, GAMMA};
	static const HklFunction *functions[] = {&psi_constant_vertical_func};
	static const HklModeAutoInfo info = {
		HKL_MODE_AUTO_INFO_WITH_PARAMS(__func__, axes, axes,
					       functions, psi_constant_parameters),
	};

	return hkl_mode_auto_new(&info,
				 &psi_constant_vertical_mode_operations,
				 TRUE);
}

/*static HklEngine *hkl_engine_e4c_hkl_new(HklEngineList *engines)*/
static HklEngine *hkl_engine_e4cvg_hkl_new(HklEngineList *engines)
{
	HklEngine *self;
	HklMode *default_mode;

	self = hkl_engine_hkl_new(engines);

	default_mode = bissector();
	hkl_engine_add_mode(self, default_mode);
	hkl_engine_mode_set(self, default_mode);

	hkl_engine_add_mode(self, constant_omega());
	hkl_engine_add_mode(self, constant_chi());
	hkl_engine_add_mode(self, constant_phi());
	hkl_engine_add_mode(self, double_diffraction());
	hkl_engine_add_mode(self, psi_constant());

	return self;
}

/************/
/* mode psi */
/************/

static HklMode *psi(void)
{
	static const char *axes[] = {OMEGA, CHI, PHI, TTH, GAMMA};
	static const HklFunction *functions[] = {&psi_func};
	static const HklModeAutoInfo info = {
		HKL_MODE_AUTO_INFO_WITH_PARAMS(__func__, axes, axes,
					       functions, psi_parameters),
	};

	return hkl_mode_psi_new(&info);
}

/*static HklEngine *hkl_engine_e4c_psi_new(HklEngineList *engines)*/
static HklEngine *hkl_engine_e4cvg_psi_new(HklEngineList *engines)
{
	HklEngine *self;
	HklMode *default_mode;

	self = hkl_engine_psi_new(engines);

	default_mode = psi();
	hkl_engine_add_mode(self, default_mode);
	hkl_engine_mode_set(self, default_mode);

	return self;
}

/*****************/
/* mode readonly */
/*****************/

REGISTER_READONLY_INCIDENCE(hkl_engine_e4cvg_incidence_new,
			    P99_PROTECT({OMEGA, CHI, PHI}),
			    surface_parameters_y);

REGISTER_READONLY_EMERGENCE(hkl_engine_e4cvg_emergence_new,
			    P99_PROTECT({OMEGA, CHI, PHI, TTH, GAMMA}),
			    surface_parameters_y);




REGISTER_DIFFRACTOMETER(eulerian4CVG, "E4CVG", HKL_GEOMETRY_EULERIAN4CVG_DESCRIPTION);
