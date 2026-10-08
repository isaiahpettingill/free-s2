#include "s2.h"
#define PI 3.14159265358979323846
static double log_gamma(double x) {
  static const double c[] = {676.5203681218851,    -1259.1392167224028,
                             771.3234287776531,    -176.6150291621406,
                             12.507343278686905,   -0.13857109526572012,
                             9.984369578019572e-6, 1.5056327351493116e-7};
  double y, t;
  int i;
  if (x <= 0)
    return S2_NA;
  if (x < 0.5)
    return log(PI) - log(sin(PI * x)) - log_gamma(1 - x);
  x -= 1;
  y = 0.99999999999980993;
  for (i = 0; i < 8; i++)
    y += c[i] / (x + i + 1);
  t = x + 7.5;
  return 0.91893853320467274178 + (x + 0.5) * log(t) - t + log(y);
}
static double gamma_cdf(double a, double x) {
  int i;
  double sum, term, b, c, d, h, an, delta, scale;
  if (a <= 0)
    return S2_NA;
  if (x <= 0)
    return 0;
  scale = exp(a * log(x) - x - log_gamma(a));
  if (x < a + 1) {
    sum = term = 1 / a;
    for (i = 1; i < 10000; i++) {
      term *= x / (a + i);
      sum += term;
      if (fabs(term) < fabs(sum) * 1e-14)
        break;
    }
    return sum * scale;
  }
  b = x + 1 - a;
  c = 1e300;
  d = 1 / b;
  h = d;
  for (i = 1; i < 10000; i++) {
    an = -i * (i - a);
    b += 2;
    d = an * d + b;
    if (fabs(d) < 1e-300)
      d = 1e-300;
    c = b + an / c;
    if (fabs(c) < 1e-300)
      c = 1e-300;
    d = 1 / d;
    delta = d * c;
    h *= delta;
    if (fabs(delta - 1) < 1e-14)
      break;
  }
  return 1 - scale * h;
}
static double beta_fraction(double a, double b, double x) {
  int m;
  double c, d, h, aa, delta;
  c = 1;
  d = 1 - (a + b) * x / (a + 1);
  if (fabs(d) < 1e-300)
    d = 1e-300;
  d = 1 / d;
  h = d;
  for (m = 1; m < 10000; m++) {
    aa = m * (b - m) * x / ((a + 2 * m - 1) * (a + 2 * m));
    d = 1 + aa * d;
    if (fabs(d) < 1e-300)
      d = 1e-300;
    c = 1 + aa / c;
    if (fabs(c) < 1e-300)
      c = 1e-300;
    d = 1 / d;
    h *= d * c;
    aa = -(a + m) * (a + b + m) * x / ((a + 2 * m) * (a + 2 * m + 1));
    d = 1 + aa * d;
    if (fabs(d) < 1e-300)
      d = 1e-300;
    c = 1 + aa / c;
    if (fabs(c) < 1e-300)
      c = 1e-300;
    d = 1 / d;
    delta = d * c;
    h *= delta;
    if (fabs(delta - 1) < 1e-14)
      break;
  }
  return h;
}
static double beta_cdf(double a, double b, double x) {
  double scale;
  if (a <= 0 || b <= 0)
    return S2_NA;
  if (x <= 0)
    return 0;
  if (x >= 1)
    return 1;
  scale = exp(log_gamma(a + b) - log_gamma(a) - log_gamma(b) + a * log(x) +
              b * log(1 - x));
  if (x < (a + 1) / (a + b + 2))
    return scale * beta_fraction(a, b, x) / a;
  return 1 - scale * beta_fraction(b, a, 1 - x) / b;
}
static double normal_cdf(double z) {
  double p; /* Gamma identity avoids an approximate normal-polynomial table. */
  if (z == 0)
    return .5;
  p = gamma_cdf(.5, z * z / 2);
  return z > 0 ? .5 + .5 * p : .5 - .5 * p;
}
static double uniform(void) {
  Value *v, *r;
  unsigned long state;
  v = s2_lookup(s2_expression ? s2_expression : &s2_global, ".Random.seed");
  if (v && (v->type != V_NUM || v->length != 1 || s2_missing(v->num[0]) ||
            v->num[0] < 1 || v->num[0] >= 2147483647.0))
    s2_fail("invalid .Random.seed for free-s2 generator");
  state = v ? (unsigned long)v->num[0] : 1UL;
  if (state < 1 || state >= 2147483647UL)
    state = 1;
  /* Schrage form of Park-Miller; 32-bit unsigned long on both targets. */
  {
    long hi, lo, next;
    hi = (long)(state / 127773UL);
    lo = (long)(state % 127773UL);
    next = 16807L * lo - 2836L * hi;
    if (next <= 0)
      next += 2147483647L;
    state = (unsigned long)next;
  }
  r = s2_number((double)state);
  s2_set(s2_expression ? s2_expression : &s2_global, ".Random.seed", r);
  return state / 2147483647.0;
}
static double normal_random(void) {
  return sqrt(-2 * log(uniform())) * cos(2 * PI * uniform());
}
static double gamma_random(double a) {
  double d, c, z, v, u;
  if (a < 1)
    return gamma_random(a + 1) * pow(uniform(), 1 / a);
  d = a - 1.0 / 3;
  c = 1 / sqrt(9 * d);
  for (;;) {
    z = normal_random();
    v = 1 + c * z;
    if (v <= 0)
      continue;
    v = v * v * v;
    u = uniform();
    if (u < 1 - .0331 * z * z * z * z ||
        log(u) < .5 * z * z + d * (1 - v + log(v)))
      return d * v;
  }
}
static int family(const char *s) {
  static const char *f[] = {"norm",   "unif",  "exp",     "binom", "pois",
                            "t",      "chisq", "gamma",   "beta",  "f",
                            "cauchy", "logis", "weibull", NULL};
  int i;
  if (!strchr("dpqr", s[0]))
    return 0;
  for (i = 0; f[i]; i++)
    if (!strcmp(s + 1, f[i]))
      return i + 1;
  return 0;
}
static double density(int f, double x, double a, double b) {
  double z;
  if (f == 1) {
    if (b <= 0)
      return S2_NA;
    z = (x - a) / b;
    return exp(-z * z / 2) / (sqrt(2 * PI) * b);
  }
  if (f == 2)
    return b <= a ? S2_NA : x < a || x > b ? 0 : 1 / (b - a);
  if (f == 3)
    return x < 0 ? 0 : exp(-x);
  if (f == 4) {
    if (a < 0 || floor(a) != a || b < 0 || b > 1)
      return S2_NA;
    if (x < 0 || x > a || floor(x) != x)
      return 0;
    if (b == 0)
      return x == 0;
    if (b == 1)
      return x == a;
    return exp(log_gamma(a + 1) - log_gamma(x + 1) - log_gamma(a - x + 1) +
               x * log(b) + (a - x) * log(1 - b));
  }
  if (f == 5) {
    if (a < 0)
      return S2_NA;
    if (x < 0 || floor(x) != x)
      return 0;
    if (a == 0)
      return x == 0;
    return exp(x * log(a) - a - log_gamma(x + 1));
  }
  if (f == 6) {
    if (a <= 0)
      return S2_NA;
    return exp(log_gamma((a + 1) / 2) - log_gamma(a / 2) - .5 * log(a * PI) -
               (a + 1) / 2 * log(1 + x * x / a));
  }
  if (f == 7) {
    a /= 2;
    if (a <= 0)
      return S2_NA;
    if (x < 0)
      return 0;
    if (x == 0)
      return a == 1 ? .5 : a > 1 ? 0 : S2_NA;
    return exp((a - 1) * log(x) - x / 2 - a * log(2) - log_gamma(a));
  }
  if (f == 8) {
    if (a <= 0)
      return S2_NA;
    if (x < 0)
      return 0;
    if (x == 0)
      return a == 1 ? 1 : a > 1 ? 0 : S2_NA;
    return exp((a - 1) * log(x) - x - log_gamma(a));
  }
  if (f == 9) {
    if (a <= 0 || b <= 0)
      return S2_NA;
    if (x < 0 || x > 1)
      return 0;
    if (x == 0)
      return a == 1 ? b : a > 1 ? 0 : S2_NA;
    if (x == 1)
      return b == 1 ? a : b > 1 ? 0 : S2_NA;
    return exp((a - 1) * log(x) + (b - 1) * log(1 - x) + log_gamma(a + b) -
               log_gamma(a) - log_gamma(b));
  }
  if (f == 10) {
    if (a <= 0 || b <= 0)
      return S2_NA;
    if (x <= 0)
      return 0;
    return exp((a / 2) * log(a / b) + (a / 2 - 1) * log(x) -
               (a + b) / 2 * log(1 + a * x / b) + log_gamma((a + b) / 2) -
               log_gamma(a / 2) - log_gamma(b / 2));
  }
  if (f == 11) {
    if (b <= 0)
      return S2_NA;
    z = (x - a) / b;
    return 1 / (PI * b * (1 + z * z));
  }
  if (f == 12) {
    if (b <= 0)
      return S2_NA;
    z = exp(-fabs((x - a) / b));
    return z / ((1 + z) * (1 + z) * b);
  }
  if (f == 13) {
    if (a <= 0)
      return S2_NA;
    if (x < 0)
      return 0;
    if (x == 0)
      return a == 1 ? 1 : a > 1 ? 0 : S2_NA;
    return a * pow(x, a - 1) * exp(-pow(x, a));
  }
  return S2_NA;
}
static double cdf(int f, double x, double a, double b) {
  double z;
  if (f == 1)
    return b <= 0 ? S2_NA : normal_cdf((x - a) / b);
  if (f == 2)
    return b <= a ? S2_NA : x <= a ? 0 : x >= b ? 1 : (x - a) / (b - a);
  if (f == 3)
    return x <= 0 ? 0 : 1 - exp(-x);
  if (f == 4) {
    if (a < 0 || floor(a) != a || b < 0 || b > 1)
      return S2_NA;
    if (x < 0)
      return 0;
    if (x >= a)
      return 1;
    if (b == 0)
      return 1;
    if (b == 1)
      return 0;
    return beta_cdf(a - floor(x), floor(x) + 1, 1 - b);
  }
  if (f == 5) {
    if (a < 0)
      return S2_NA;
    if (x < 0)
      return 0;
    if (a == 0)
      return 1;
    return 1 - gamma_cdf(floor(x) + 1, a);
  }
  if (f == 6) {
    if (a <= 0)
      return S2_NA;
    z = beta_cdf(a / 2, .5, a / (a + x * x));
    return x < 0 ? z / 2 : 1 - z / 2;
  }
  if (f == 7)
    return gamma_cdf(a / 2, x / 2);
  if (f == 8)
    return gamma_cdf(a, x);
  if (f == 9)
    return beta_cdf(a, b, x);
  if (f == 10)
    return a <= 0 || b <= 0 ? S2_NA
           : x <= 0         ? 0
                            : beta_cdf(a / 2, b / 2, a * x / (b + a * x));
  if (f == 11)
    return b <= 0 ? S2_NA : .5 + atan((x - a) / b) / PI;
  if (f == 12) {
    if (b <= 0)
      return S2_NA;
    z = (x - a) / b;
    return z >= 0 ? 1 / (1 + exp(-z)) : exp(z) / (1 + exp(z));
  }
  if (f == 13)
    return a <= 0 ? S2_NA : x <= 0 ? 0 : 1 - exp(-pow(x, a));
  return S2_NA;
}
static double quant(int f, double p, double a, double b) {
  double lo, hi, mid, value;
  int i;
  if (p < 0 || p > 1)
    return S2_NA;
  if (f == 2)
    return b <= a ? S2_NA : a + (b - a) * p;
  if (f == 3)
    return p == 1 ? S2_NA : -log(1 - p);
  if (f == 11)
    return p == 0 || p == 1 || b <= 0 ? S2_NA : a + b * tan(PI * (p - .5));
  if (f == 12)
    return p == 0 || p == 1 || b <= 0 ? S2_NA : a + b * log(p / (1 - p));
  if (f == 13)
    return p == 1 || a <= 0 ? S2_NA : pow(-log(1 - p), 1 / a);
  if (f == 4 || f == 5) {
    if (p == 0)
      return 0;
    if (p == 1)
      return f == 4 ? a : S2_NA;
    lo = 0;
    hi = f == 4 ? a : 1;
    while (f == 5 && cdf(f, hi, a, b) < p) {
      hi *= 2;
      if (hi > 30000)
        return S2_NA;
    }
    while (lo < hi) {
      mid = floor((lo + hi) / 2);
      value = cdf(f, mid, a, b);
      if (s2_missing(value))
        return S2_NA;
      if (value >= p)
        hi = mid;
      else
        lo = mid + 1;
    }
    return lo;
  }
  if (p == 0)
    return f == 1 || f == 6 ? S2_NA : 0;
  if (p == 1)
    return f == 9 ? 1 : S2_NA;
  if (f == 1) {
    if (b <= 0)
      return S2_NA;
    lo = a - 40 * b;
    hi = a + 40 * b;
  } else {
    lo = f == 6 ? -1 : 0;
    hi = 1;
    if (f == 9)
      hi = 1;
    else {
      while (cdf(f, hi, a, b) < p) {
        hi *= 2;
        if (hi > 1e100)
          return S2_NA;
      }
      if (f == 6)
        while (cdf(f, lo, a, b) > p) {
          lo *= 2;
          if (lo < -1e100)
            return S2_NA;
        }
    }
  }
  for (i = 0; i < 100; i++) {
    mid = lo + (hi - lo) / 2;
    value = cdf(f, mid, a, b);
    if (s2_missing(value))
      return S2_NA;
    if (value < p)
      lo = mid;
    else
      hi = mid;
  }
  return (lo + hi) / 2;
}
static double random_value(int f, double a, double b) {
  double x, y;
  if (f == 1)
    return b <= 0 ? S2_NA : a + b * normal_random();
  if (f == 7 || f == 8)
    return a <= 0 ? S2_NA : gamma_random(f == 7 ? a / 2 : a) * (f == 7 ? 2 : 1);
  if (f == 9) {
    if (a <= 0 || b <= 0)
      return S2_NA;
    x = gamma_random(a);
    y = gamma_random(b);
    return x / (x + y);
  }
  if (f == 6)
    return a <= 0 ? S2_NA : normal_random() / sqrt(gamma_random(a / 2) * 2 / a);
  if (f == 10)
    return a <= 0 || b <= 0
               ? S2_NA
               : (gamma_random(a / 2) / a) / (gamma_random(b / 2) / b);
  return quant(f, uniform(), a, b);
}
static void sorted(double *x, int n) {
  int i, j;
  double t;
  for (i = 1; i < n; i++)
    for (j = i; j > 0 && x[j] < x[j - 1]; j--) {
      t = x[j];
      x[j] = x[j - 1];
      x[j - 1] = t;
    }
}
static double empirical(double *x, int n, double p) {
  double h;
  int i;
  if (!n || p < 0 || p > 1 || s2_missing(p))
    return S2_NA;
  h = n * p - .5;
  if (h <= 0)
    return x[0];
  if (h >= n - 1)
    return x[n - 1];
  i = (int)floor(h);
  return x[i] + (h - i) * (x[i + 1] - x[i]);
}
Value *s2_statistics(const char *name, Node *args, Env *env) {
  Value *v, *w, *r, *a, *b, *probs;
  double *x, mean, trim, h, sx, sy, xx, yy, xy, z;
  int i, j, k, n, m, f, cx, cy, nx, ny;
  f = family(name);
  if (f) {
    static const char *an[] = {
        "",   "mean",  "min",    "",    "size",     "lambda",   "df",
        "df", "shape", "shape1", "df1", "location", "location", "shape"};
    static const char *bn[] = {"", "sd", "max",    "",    "prob",  "",      "",
                               "", "",   "shape2", "df2", "scale", "scale", ""};
    v = s2_arg(args, 0,
               name[0] == 'r'   ? "n"
               : name[0] == 'q' ? "p"
                                : "q",
               env, NULL);
    if (!v || v->type != V_NUM)
      s2_fail("numeric distribution argument required");
    if ((f >= 4 && f <= 10) || f == 13) {
      if (!s2_arg(args, 1, an[f], env, NULL))
        s2_fail("required distribution parameter missing");
      if ((f == 4 || f == 9 || f == 10) && !s2_arg(args, 2, bn[f], env, NULL))
        s2_fail("required distribution parameter missing");
    }
    a = s2_arg(args, 1, an[f], env,
               s2_number(f == 1 || f == 2 || f == 11 || f == 12 ? 0 : 1));
    b = s2_arg(args, 2, bn[f], env, s2_number(f == 4 ? .5 : 1));
    if (a->type != V_NUM || b->type != V_NUM || !a->length || !b->length)
      s2_fail("numeric parameters required");
    if (name[0] == 'r')
      n = v->length == 1 ? s2_count(v) : v->length;
    else {
      n = v->length;
      if (a->length > n)
        n = a->length;
      if (b->length > n)
        n = b->length;
      if (!v->length)
        n = 0;
    }
    r = s2_value(V_NUM, n);
    for (i = 0; i < n; i++) {
      z = name[0] == 'r' ? 0 : v->num[i % v->length];
      sx = a->num[i % a->length];
      sy = b->num[i % b->length];
      r->num[i] = s2_missing(z) || s2_missing(sx) || s2_missing(sy) ? S2_NA
                  : name[0] == 'd' ? density(f, z, sx, sy)
                  : name[0] == 'p' ? cdf(f, z, sx, sy)
                  : name[0] == 'q' ? quant(f, z, sx, sy)
                                   : random_value(f, sx, sy);
    }
    return r;
  }
  if (!strcmp(name, "set.seed")) {
    z = s2_scalar(s2_arg(args, 0, "i", env, NULL));
    if (z < 0 || z > 1000 || floor(z) != z)
      s2_fail("seed must be integer from 0 to 1000");
    s2_set(s2_expression ? s2_expression : &s2_global, ".Random.seed",
           s2_number(z + 1));
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "sample")) {
    int *indices;
    v = s2_arg(args, 0, "x", env, NULL);
    if (!v)
      s2_fail("sample data required");
    n = v->length;
    if (n == 1 && v->type == V_NUM) {
      n = s2_count(v);
      v = s2_value(V_NUM, n);
      for (i = 0; i < n; i++)
        v->num[i] = i + 1;
    }
    m = s2_count(s2_arg(args, 1, "size", env, s2_number(n)));
    k = s2_scalar(s2_arg(args, 2, "replace", env, s2_number(0))) != 0;
    if ((!n && m) || (!k && m > n))
      s2_fail("invalid sample size");
    indices = (int *)s2_alloc((size_t)(n ? n : 1) * sizeof(int));
    for (i = 0; i < n; i++)
      indices[i] = i;
    r = s2_value(v->type, m);
    for (i = 0; i < m; i++) {
      j = (int)(uniform() * (k ? n : n - i));
      f = indices[j];
      if (!k)
        indices[j] = indices[n - i - 1];
      w = s2_element(v, f);
      if (v->type == V_NUM)
        r->num[i] = w->num[0];
      else if (v->type == V_STR)
        r->str[i] = w->str[0];
      else
        s2_fail("sample needs numeric or character vector");
    }
    return r;
  }
  if (!strcmp(name, "gamma") || !strcmp(name, "lgamma") ||
      !strcmp(name, "beta") || !strcmp(name, "lbeta")) {
    v = s2_arg(args, 0,
               (!strcmp(name, "beta") || !strcmp(name, "lbeta")) ? "a" : "x",
               env, NULL);
    if (!v || v->type != V_NUM)
      s2_fail("numeric data required");
    w = s2_arg(args, 1, "b", env, s2_number(1));
    n = v->length > w->length ? v->length : w->length;
    if (!v->length || !w->length)
      n = 0;
    r = s2_value(V_NUM, n);
    for (i = 0; i < n; i++) {
      sx = v->num[i % v->length];
      sy = w->num[i % w->length];
      z = s2_missing(sx) || s2_missing(sy) ? S2_NA : log_gamma(sx);
      if (strstr(name, "beta"))
        z = sx <= 0 || sy <= 0 ? S2_NA : z + log_gamma(sy) - log_gamma(sx + sy);
      r->num[i] = s2_missing(z)                                     ? S2_NA
                  : !strcmp(name, "gamma") || !strcmp(name, "beta") ? exp(z)
                                                                    : z;
    }
    return r;
  }
  if (strcmp(name, "mean") && strcmp(name, "median") &&
      strcmp(name, "quantile") && strcmp(name, "summary") &&
      strcmp(name, "var") && strcmp(name, "cor"))
    return NULL;
  v = s2_arg(args, 0, "x", env, NULL);
  if (!v || v->type != V_NUM)
    s2_fail("numeric data required");
  n = v->length;
  if (!strcmp(name, "var") || !strcmp(name, "cor")) {
    if (!strcmp(name, "cor") &&
        s2_scalar(s2_arg(args, 2, "trim", env, s2_number(0))) != 0)
      s2_fail("trimmed correlation is not implemented");
    w = s2_arg(args, 1, "y", env, v);
    if (w->type != V_NUM)
      s2_fail("numeric y required");
    if (!strcmp(name, "var") && w == v && !v->rows) {
      for (i = 0; i < n; i++)
        if (s2_missing(v->num[i]))
          return s2_number(S2_NA);
      return s2_number(n < 2 ? S2_NA : s2_var(v->num, n));
    }
    nx = v->rows ? v->rows : n;
    ny = w->rows ? w->rows : w->length;
    if (nx != ny)
      s2_fail("incompatible observation counts");
    cx = v->rows ? v->cols : 1;
    cy = w->rows ? w->cols : 1;
    r = s2_value(V_NUM, cx * cy);
    for (i = 0; i < cx; i++)
      for (j = 0; j < cy; j++) {
        sx = sy = xx = yy = xy = 0;
        f = 0;
        for (k = 0; k < nx; k++) {
          z = v->num[i * nx + k];
          h = w->num[j * nx + k];
          if (s2_missing(z) || s2_missing(h)) {
            f = 1;
            break;
          }
          mean = z - sx;
          trim = h - sy;
          sx += mean / (k + 1);
          sy += trim / (k + 1);
          xx += mean * (z - sx);
          yy += trim * (h - sy);
          xy += mean * (h - sy);
        }
        r->num[j * cx + i] =
            f || nx < 2 ? S2_NA
            : !strcmp(name, "cor")
                ? (xx <= 0 || yy <= 0 ? S2_NA : xy / sqrt(xx * yy))
                : xy / (nx - 1);
      }
    if (v->rows || w->rows) {
      r->rows = cx;
      r->cols = cy;
    }
    return r;
  }
  x = (double *)s2_alloc((size_t)(n ? n : 1) * sizeof(double));
  for (i = 0; i < n; i++) {
    x[i] = v->num[i];
    if (s2_missing(x[i])) {
      if (!strcmp(name, "mean"))
        return s2_number(S2_NA);
      s2_fail("missing data: filter with x[!is.na(x)]");
    }
  }
  if (!strcmp(name, "mean")) {
    trim = s2_scalar(s2_arg(args, 1, "trim", env, s2_number(0)));
    if (trim < 0 || trim > .5)
      s2_fail("trim must be between 0 and 0.5");
    if (!n)
      return s2_number(S2_NA);
    if (trim == .5) {
      sorted(x, n);
      return s2_number(empirical(x, n, .5));
    }
    k = (int)floor(n * trim);
    if (k)
      sorted(x, n);
    return s2_number(s2_mean(x + k, n - 2 * k));
  }
  sorted(x, n);
  if (!strcmp(name, "median"))
    return s2_number(empirical(x, n, .5));
  if (!strcmp(name, "summary")) {
    r = s2_value(V_NUM, 6);
    r->names = (char **)s2_alloc(6 * sizeof(char *));
    r->num[0] = empirical(x, n, 0);
    r->num[1] = empirical(x, n, .25);
    r->num[2] = empirical(x, n, .5);
    r->num[3] = n ? s2_mean(x, n) : S2_NA;
    r->num[4] = empirical(x, n, .75);
    r->num[5] = empirical(x, n, 1);
    {
      const char *labels[] = {"Min",  "1st Qu", "Median",
                              "Mean", "3rd Qu", "Max"};
      for (i = 0; i < 6; i++)
        r->names[i] = s2_strdup(labels[i]);
    }
    return r;
  }
  probs = s2_arg(args, 1, "probs", env, NULL);
  if (!probs) {
    probs = s2_value(V_NUM, 5);
    for (i = 0; i < 5; i++)
      probs->num[i] = i * .25;
  }
  if (probs->type != V_NUM)
    s2_fail("numeric probabilities required");
  r = s2_value(V_NUM, probs->length);
  for (i = 0; i < probs->length; i++)
    r->num[i] = empirical(x, n, probs->num[i]);
  return r;
}
