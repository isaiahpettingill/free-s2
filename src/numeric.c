#include "s2.h"
#ifdef S2_FORTRAN
extern void s2stat_(double *, int *, double *, double *, double *);
#endif
static void statistics(double *x, int n, double *sum, double *mean, double *var)
{
#if defined(S2_FORTRAN)
    s2stat_(x,&n,sum,mean,var);
#elif defined(S2_MSFORTRAN)
    FILE *f;
    int i, status;
    /* Dedicated per-process working directory required for DOS bridge. */
    f = fopen("S2NUM.IN","w"); if (!f) s2_fail("cannot create numeric input");
    fprintf(f,"%d\n",n); for (i = 0; i < n; i++) fprintf(f,"%.17g\n",x[i]); fclose(f);
    remove("S2NUM.OUT");
    status = system("S2NUM.EXE < S2NUM.IN > S2NUM.OUT");
    f = fopen("S2NUM.OUT","r");
    if (status || !f) { if (f) fclose(f); remove("S2NUM.IN"); remove("S2NUM.OUT"); s2_fail("Microsoft Fortran numeric worker failed"); }
    status = fscanf(f,"%lf %lf %lf",sum,mean,var); fclose(f);
    remove("S2NUM.IN"); remove("S2NUM.OUT");
    if (status != 3) s2_fail("invalid Microsoft Fortran numeric result");
#else
    double correction, t, y, delta, ss;
    int i;
    *sum = *mean = ss = correction = 0;
    for (i = 0; i < n; i++) {
        y = x[i]-correction; t = *sum+y; correction = (t-*sum)-y; *sum = t;
        delta = x[i]-*mean; *mean += delta/(i+1); ss += delta*(x[i]-*mean);
    }
    *var = n > 1 ? ss/(n-1) : 0;
#endif
}
double s2_sum(double *x, int n) { double s,m,v; statistics(x,n,&s,&m,&v); return s; }
double s2_mean(double *x, int n) { double s,m,v; statistics(x,n,&s,&m,&v); return m; }
double s2_var(double *x, int n) { double s,m,v; statistics(x,n,&s,&m,&v); return v; }
