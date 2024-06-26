
#include <complex.h>

struct bin_conf_s {

	int n_resp;
	int n_card;
	int mavg_window;
	int mavg_window_card;
	int cluster_dim;

	long resp_labels_idx[2];
	long card_labels_idx[2];

	const char* card_out;

	float offset_angle[2];

	_Bool amplitude;

};

extern const struct bin_conf_s bin_defaults;

extern int bin_quadrature(const long bins_dims[DIMS], float* bins,
			const long labels_dims[DIMS], complex float* labels,
			const struct bin_conf_s conf);
	
