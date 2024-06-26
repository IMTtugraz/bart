
#include <complex.h>

extern void compute_crb(int P, float rCRB[P], complex float A[P][P], int M, int N, const complex float derivatives[M][N], const complex float signal[N], const int idx_unknowns[P - 1]);
extern void normalize_crb(int P, float rCRB[P], int N, float TR, float T1, float T2, float B1, float omega, const int idx_unknowns[P - 1]);
extern void getidxunknowns(int P, int idx_unknowns[P - 1], unsigned long unknowns);
extern void display_crb(int P, float rCRB[P], complex float fisher[P][P], const int idx_unknowns[P - 1]);

