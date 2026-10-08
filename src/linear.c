#include "s2.h"
#if !defined(S2_FORTRAN) && !defined(S2_MSFORTRAN)
static void qr_c(double *x, double *y, int n, int p, double tol, double *coef,
                 double *res) {
  double *q, *r, *b, *norm, z, scale;
  int pivot[32], i, j, k, pass, best, t;
  q = (double *)s2_alloc((size_t)n * p * sizeof(double));
  r = (double *)s2_alloc((size_t)p * p * sizeof(double));
  b = (double *)s2_alloc((size_t)p * sizeof(double));
  norm = (double *)s2_alloc((size_t)p * sizeof(double));
  memcpy(q, x, (size_t)n * p * sizeof(double));
  scale = 0;
  for (j = 0; j < p; j++) {
    pivot[j] = j;
    for (i = 0; i < n; i++)
      norm[j] += q[j * n + i] * q[j * n + i];
    if (norm[j] > scale)
      scale = norm[j];
  }
  for (k = 0; k < p; k++) {
    best = k;
    for (j = k + 1; j < p; j++)
      if (norm[j] > norm[best])
        best = j;
    if (best != k) {
      for (i = 0; i < n; i++) {
        z = q[k * n + i];
        q[k * n + i] = q[best * n + i];
        q[best * n + i] = z;
      }
      for (i = 0; i < k; i++) {
        z = r[i * p + k];
        r[i * p + k] = r[i * p + best];
        r[i * p + best] = z;
      }
      t = pivot[k];
      pivot[k] = pivot[best];
      pivot[best] = t;
      z = norm[k];
      norm[k] = norm[best];
      norm[best] = z;
    }
    z = 0;
    for (i = 0; i < n; i++)
      z += q[k * n + i] * q[k * n + i];
    z = sqrt(z);
    if (z <= tol * sqrt(scale))
      s2_fail("rank-deficient least-squares design");
    r[k * p + k] = z;
    for (i = 0; i < n; i++) {
      q[k * n + i] /= z;
      b[k] += q[k * n + i] * y[i];
    }
    for (j = k + 1; j < p; j++) {
      for (pass = 0; pass < 2; pass++) {
        z = 0;
        for (i = 0; i < n; i++)
          z += q[k * n + i] * q[j * n + i];
        r[k * p + j] += z;
        for (i = 0; i < n; i++)
          q[j * n + i] -= z * q[k * n + i];
      }
      norm[j] = 0;
      for (i = 0; i < n; i++)
        norm[j] += q[j * n + i] * q[j * n + i];
    }
  }
  for (k = p - 1; k >= 0; k--) {
    for (j = k + 1; j < p; j++)
      b[k] -= r[k * p + j] * b[j];
    b[k] /= r[k * p + k];
    coef[pivot[k]] = b[k];
  }
  for (i = 0; i < n; i++) {
    z = y[i];
    for (j = 0; j < p; j++)
      z -= x[j * n + i] * coef[j];
    res[i] = z;
  }
}
#endif
static void least_squares(double *x, double *y, int n, int p, double tol,
                          double *coef, double *res) {
#ifdef S2_FORTRAN
  extern void s2qr_(double *, double *, int *, int *, double *, double *,
                    double *, int *);
  int status;
  double *workspace;
  workspace = (double *)s2_alloc((size_t)n * p * sizeof(double));
  memcpy(workspace, x, (size_t)n * p * sizeof(double));
  status = 0;
  s2qr_(workspace, y, &n, &p, &tol, coef, res, &status);
  if (status)
    s2_fail("rank-deficient least-squares design");
#elif defined(S2_MSFORTRAN)
  FILE *f;
  int i, status;
  if (n > 1024)
    s2_fail("DOS Fortran regression allows at most 1024 rows");
  f = fopen("S2REG.IN", "w");
  if (!f)
    s2_fail("cannot write Fortran regression input");
  fprintf(f, "%d %d %.17E\n", n, p, tol);
  for (i = 0; i < n * p; i++)
    fprintf(f, "%.17E\n", x[i]);
  for (i = 0; i < n; i++)
    fprintf(f, "%.17E\n", y[i]);
  fclose(f);
  if (system("S2REG.EXE < S2REG.IN > S2REG.OUT"))
    s2_fail("Fortran regression worker failed");
  f = fopen("S2REG.OUT", "r");
  if (!f)
    s2_fail("Fortran regression output missing");
  if (fscanf(f, "%d", &status) != 1 || status) {
    fclose(f);
    s2_fail("rank-deficient least-squares design");
  }
  for (i = 0; i < p; i++)
    if (fscanf(f, "%lf", coef + i) != 1) {
      fclose(f);
      s2_fail("bad Fortran coefficients");
    }
  for (i = 0; i < n; i++)
    if (fscanf(f, "%lf", res + i) != 1) {
      fclose(f);
      s2_fail("bad Fortran residuals");
    }
  fclose(f);
#else
  qr_c(x, y, n, p, tol, coef, res);
#endif
}
Value *s2_linear(const char *name, Node *args, Env *env) {
  Value *v, *w, *r, *a, *b, *weights, *coef, *res, *dim, *fn;
  Node *p, *aa, *bb;
  int i, j, k, n, m, rows, cols, pr, pc, intercept, type, nv;
  double x, y, z, tol, *design, *response, *solution, *residuals;
  if (!strcmp(name, "cbind") || !strcmp(name, "rbind")) {
    Value *values[S2_MAX_ARGS];
    nv = rows = cols = 0;
    type = V_NUM;
    for (p = args; p; p = p->next) {
      if (nv == S2_MAX_ARGS)
        s2_fail("too many matrix arguments");
      v = s2_eval(p->a, p->scope ? p->scope : env);
      values[nv++] = v;
      type = s2_common(s2_value(type, 0), v);
      if (!strcmp(name, "cbind")) {
        if ((v->rows ? v->rows : v->length) > rows)
          rows = v->rows ? v->rows : v->length;
        cols += v->rows ? v->cols : 1;
      } else {
        if ((v->rows ? v->cols : v->length) > cols)
          cols = v->rows ? v->cols : v->length;
        rows += v->rows ? v->rows : 1;
      }
    }
    if ((long)rows * cols > S2_MAX_VECTOR)
      s2_fail("matrix too large");
    r = s2_value(type, rows * cols);
    r->rows = rows;
    r->cols = cols;
    k = 0;
    for (n = 0; n < nv; n++) {
      v = s2_coerce(values[n], type, 0);
      pr = values[n]->rows          ? values[n]->rows
           : !strcmp(name, "cbind") ? rows
                                    : 1;
      pc = values[n]->rows          ? values[n]->cols
           : !strcmp(name, "cbind") ? 1
                                    : cols;
      if (values[n]->rows && ((!strcmp(name, "cbind") && pr != rows) ||
                              (!strcmp(name, "rbind") && pc != cols)))
        s2_fail("matrix bind dimensions do not match");
      if (!v->length && r->length)
        s2_fail("empty matrix argument");
      for (j = 0; j < pc; j++)
        for (i = 0; i < pr; i++) {
          m = (j * pr + i) % v->length;

          if (!strcmp(name, "cbind")) {
            int dest;
            dest = (k + j) * rows + i;
            if (type == V_NUM)
              r->num[dest] = v->num[m];
            else if (type == V_STR)
              r->str[dest] = v->str[m];
            else
              s2_fail("matrix bind supports numeric/character data");
          } else {
            int dest;
            dest = j * rows + k + i;
            if (type == V_NUM)
              r->num[dest] = v->num[m];
            else if (type == V_STR)
              r->str[dest] = v->str[m];
            else
              s2_fail("matrix bind supports numeric/character data");
          }
        }
      k += !strcmp(name, "cbind") ? pc : pr;
    }
    return r;
  }
  if (!strcmp(name, "diag")) {
    v = s2_arg(args, 0, "x", env, s2_number(1));
    if (v->type != V_NUM)
      s2_fail("numeric diag argument required");
    if (v->rows) {
      n = v->rows < v->cols ? v->rows : v->cols;
      r = s2_value(V_NUM, n);
      for (i = 0; i < n; i++)
        r->num[i] = v->num[i * v->rows + i];
      return r;
    }
    n = v->length == 1 ? s2_count(v) : v->length;
    if ((long)n * n > S2_MAX_VECTOR)
      s2_fail("diagonal matrix too large");
    r = s2_value(V_NUM, n * n);
    r->rows = r->cols = n;
    for (i = 0; i < n; i++)
      r->num[i * n + i] = v->length == 1 ? 1 : v->num[i];
    return r;
  }
  if (!strcmp(name, "%*%") || !strcmp(name, "crossprod") ||
      !strcmp(name, "%o%") || !strcmp(name, "outer")) {
    v = s2_arg(args, 0, !strcmp(name, "outer") ? "X" : "x", env, NULL);
    w = s2_arg(args, 1, !strcmp(name, "outer") ? "Y" : "y", env,
               !strcmp(name, "crossprod") ? v : NULL);
    if (!v || !w)
      s2_fail("two operands required");
    if (!strcmp(name, "outer") || !strcmp(name, "%o%")) {
      if ((long)v->length * w->length > S2_MAX_VECTOR)
        s2_fail("outer result too large");
      fn = s2_arg(args, 2, "FUN", env, s2_string("*"));
      n = v->length * w->length;
      a = s2_value(V_NUM, n);
      b = s2_value(V_NUM, n);
      for (j = 0; j < w->length; j++)
        for (i = 0; i < v->length; i++) {
          a->num[j * v->length + i] = i + 1;
          b->num[j * v->length + i] = j + 1;
        }
      aa = (Node *)s2_alloc(sizeof(Node));
      bb = (Node *)s2_alloc(sizeof(Node));
      aa->a = (Node *)s2_alloc(sizeof(Node));
      bb->a = (Node *)s2_alloc(sizeof(Node));
      aa->a->kind = bb->a->kind = N_VALUE;
      aa->a->value = a;
      bb->a->value = b;
      a = s2_subset(v, aa, env, 0, NULL);
      b = s2_subset(w, bb, env, 0, NULL);
      aa->a->value = a;
      bb->a->value = b;
      aa->next = bb;
      if (args) {
        Node *p;
        for (p = args; p; p = p->next)
          if (!p->text || (strcmp(p->text, "X") && strcmp(p->text, "Y") &&
                           strcmp(p->text, "FUN"))) {
            Node *q = (Node *)s2_alloc(sizeof(Node));
            *q = *p;
            q->next = bb->next;
            bb->next = q;
          }
      }
      r = s2_call(fn, aa, env);
      if (r->length != n)
        s2_fail("outer FUN returned wrong length");
      a = s2_value(V_NUM, 2);
      a->num[0] = v->length;
      a->num[1] = w->length;
      r = s2_with_attr(r, "dim", a);
      return r;
    }
    if (v->type != V_NUM || w->type != V_NUM)
      s2_fail("numeric matrices required");
    if (!strcmp(name, "crossprod")) {
      n = v->rows ? v->rows : v->length;
      m = w->rows ? w->rows : w->length;
      if (n != m)
        s2_fail("nonconformable matrices");
      rows = v->rows ? v->cols : 1;
      cols = w->rows ? w->cols : 1;
    } else {
      rows = v->rows ? v->rows : 1;
      n = v->rows ? v->cols : v->length;
      m = w->rows ? w->rows : w->length;
      cols = w->rows ? w->cols : 1;
      if (n != m)
        s2_fail("nonconformable matrices");
    }
    if ((long)rows * cols > S2_MAX_VECTOR)
      s2_fail("matrix product too large");
    r = s2_value(V_NUM, rows * cols);
    r->rows = rows;
    r->cols = cols;
    for (j = 0; j < cols; j++)
      for (i = 0; i < rows; i++) {
        z = 0;
        for (k = 0; k < n; k++) {
          x = !strcmp(name, "crossprod") ? v->num[i * n + k]
                                         : v->num[k * rows + i];
          y = w->num[j * n + k];
          if (s2_missing(x) || s2_missing(y)) {
            z = S2_NA;
            break;
          }
          z += x * y;
        }
        r->num[j * rows + i] = z;
      }
    return r;
  }
  if (!strcmp(name, "solve")) {
    v = s2_arg(args, 0, "a", env, NULL);
    if (!v || v->type != V_NUM || !v->rows || v->rows != v->cols)
      s2_fail("solve needs square numeric matrix");
    n = v->rows;
    w = s2_arg(args, 1, "b", env, NULL);
    if (!w) {
      w = s2_value(V_NUM, n * n);
      w->rows = w->cols = n;
      for (i = 0; i < n; i++)
        w->num[i * n + i] = 1;
    }
    if (w->type != V_NUM || (w->rows ? w->rows : w->length) != n)
      s2_fail("incompatible right hand side");
    m = w->rows ? w->cols : 1;
    a = s2_copy(v);
    r = s2_copy(w);
    for (k = 0; k < n; k++) {
      pr = k;
      for (i = k + 1; i < n; i++)
        if (fabs(a->num[k * n + i]) > fabs(a->num[k * n + pr]))
          pr = i;
      if (s2_missing(a->num[k * n + pr]) || fabs(a->num[k * n + pr]) < 1e-14)
        s2_fail("singular matrix");
      for (j = 0; j < n; j++) {
        x = a->num[j * n + k];
        a->num[j * n + k] = a->num[j * n + pr];
        a->num[j * n + pr] = x;
      }
      for (j = 0; j < m; j++) {
        x = r->num[j * n + k];
        r->num[j * n + k] = r->num[j * n + pr];
        r->num[j * n + pr] = x;
      }
      x = a->num[k * n + k];
      for (j = 0; j < n; j++)
        a->num[j * n + k] /= x;
      for (j = 0; j < m; j++)
        r->num[j * n + k] /= x;
      for (i = 0; i < n; i++)
        if (i != k) {
          x = a->num[k * n + i];
          for (j = 0; j < n; j++)
            a->num[j * n + i] -= x * a->num[j * n + k];
          for (j = 0; j < m; j++)
            r->num[j * n + i] -= x * r->num[j * n + k];
        }
    }
    return r;
  }
  if (strcmp(name, "lsfit"))
    return NULL;
  v = s2_arg(args, 0, "x", env, NULL);
  w = s2_arg(args, 1, "y", env, NULL);
  if (!v || !w || v->type != V_NUM || w->type != V_NUM)
    s2_fail("numeric regression data required");
  n = v->rows ? v->rows : v->length;
  m = v->rows ? v->cols : 1;
  intercept = s2_scalar(s2_arg(args, 3, "intercept", env, s2_number(1))) != 0;
  m += intercept;
  if (!n || m > 32 || m > n || (long)n * m > S2_MAX_VECTOR ||
      (w->rows ? w->rows : w->length) != n)
    s2_fail("invalid regression dimensions");
  weights = s2_arg(args, 2, "wt", env, NULL);
  if (weights && (weights->type != V_NUM || weights->length != n))
    s2_fail("invalid regression weights");
  tol = s2_scalar(s2_arg(args, 4, "tolerance", env, s2_number(1e-7)));
  if (tol <= 0)
    s2_fail("positive regression tolerance required");
  design = (double *)s2_alloc((size_t)n * m * sizeof(double));
  response = (double *)s2_alloc((size_t)n * sizeof(double));
  solution = (double *)s2_alloc((size_t)m * sizeof(double));
  residuals = (double *)s2_alloc((size_t)n * sizeof(double));
  for (j = 0; j < m; j++)
    for (i = 0; i < n; i++) {
      x = intercept && !j ? 1 : v->num[(j - intercept) * n + i];
      if (s2_missing(x))
        s2_fail("missing regression data");
      if (weights) {
        z = weights->num[i];
        if (s2_missing(z) || z <= 0)
          s2_fail("weights must be positive");
        x *= sqrt(z);
      }
      design[j * n + i] = x;
    }
  cols = w->rows ? w->cols : 1;
  coef = s2_value(V_NUM, m * cols);
  coef->rows = m;
  coef->cols = cols;
  res = s2_value(V_NUM, w->length);
  res->rows = w->rows;
  res->cols = w->cols;
  for (j = 0; j < cols; j++) {
    for (i = 0; i < n; i++) {
      response[i] = w->num[j * n + i];
      if (s2_missing(response[i]))
        s2_fail("missing regression response");
      if (weights)
        response[i] *= sqrt(weights->num[i]);
    }
    least_squares(design, response, n, m, tol, solution, residuals);
    for (k = 0; k < m; k++)
      coef->num[j * m + k] = solution[k];
    for (i = 0; i < n; i++)
      res->num[j * n + i] =
          weights ? residuals[i] / sqrt(weights->num[i]) : residuals[i];
  }
  r = s2_value(V_LIST, 4);
  r->names = (char **)s2_alloc(4 * sizeof(char *));
  r->names[0] = s2_strdup("coef");
  r->items[0] = coef;
  r->names[1] = s2_strdup("residuals");
  r->items[1] = res;
  r->names[2] = s2_strdup("qr");
  r->items[2] = s2_value(V_NULL, 0);
  r->names[3] = s2_strdup("intercept");
  r->items[3] = s2_number(intercept);
  r->items[3]->logical = 1;
  (void)b;
  (void)dim;
  return r;
}
