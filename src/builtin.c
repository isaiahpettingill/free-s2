#include "s2.h"
static Node *s2_constant_args(Value *v) {
  Node *a = (Node *)s2_alloc(sizeof(Node)), *n = (Node *)s2_alloc(sizeof(Node));
  n->kind = N_VALUE;
  n->value = v;
  a->a = n;
  return a;
}
Value *s2_arg(Node *args, int pos, const char *name, Env *env, Value *def) {
  Node *p;
  int i;
  for (p = args; p; p = p->next)
    if (p->text && name && !strcmp(p->text, name)) {
      if (!p->a)
        return def;
      if (!p->value)
        p->value = s2_eval(p->a, p->scope ? p->scope : env);
      return p->value;
    }
  i = 0;
  for (p = args; p; p = p->next)
    if (!p->text) {
      if (i++ == pos) {
        if (!p->a)
          return def;
        if (!p->value)
          p->value = s2_eval(p->a, p->scope ? p->scope : env);
        return p->value;
      }
    }
  return def;
}
double s2_scalar(Value *v) {
  if (!v || v->type != V_NUM || v->length != 1 || v->num[0] == S2_NA ||
      v->num[0] != v->num[0])
    s2_fail("numeric scalar required");
  return v->num[0];
}
int s2_count(Value *v) {
  double x;
  x = s2_scalar(v);
  if (x < 0 || x > S2_MAX_VECTOR || floor(x) != x)
    s2_fail("invalid length/repetition count");
  return (int)x;
}
static Value *combine(Node *args, Env *env, int list) {
  Value *values[S2_MAX_ARGS], *r, *v, *w;
  Node *p;
  int n, len, type, logical, i, j, k;
  n = len = 0;
  type = V_NUM;
  logical = 1;
  for (p = args; p; p = p->next) {
    if (n == S2_MAX_ARGS)
      s2_fail("too many arguments");
    v = p->a ? s2_eval(p->a, p->scope ? p->scope : env) : s2_value(V_NULL, 0);
    values[n++] = v;
    if ((long)len + (list ? 1 : v->length) > S2_MAX_VECTOR)
      s2_fail("vector too long");
    len += list ? 1 : v->length;
    if (!list && v->type != V_NULL) {
      if (v->type != V_NUM && v->type != V_COMPLEX && v->type != V_STR &&
          v->type != V_LIST)
        s2_fail("cannot combine this mode");
      type = s2_common(s2_value(type, 0), v);
      if (!v->logical)
        logical = 0;
    }
  }
  if (!n && !list)
    return s2_value(V_NULL, 0);
  r = s2_value(list ? V_LIST : type, len);
  r->logical = !list && type == V_NUM && logical;
  r->names = (char **)s2_alloc((size_t)(len ? len : 1) * sizeof(char *));
  k = 0;
  p = args;
  for (i = 0; i < n; i++, p = p->next) {
    v = values[i];
    if (list) {
      r->items[k] = v;
      r->names[k++] = p->text;
      continue;
    }
    w = s2_coerce(v, type, logical);
    for (j = 0; j < w->length; j++) {
      if (type == V_NUM || type == V_COMPLEX)
        r->num[k] = w->num[j];
      if (type == V_COMPLEX)
        r->imag[k] = w->imag[j];
      if (type == V_STR)
        r->str[k] = w->str[j];
      if (type == V_LIST)
        r->items[k] = w->items[j];
      r->names[k] = p->text && w->length == 1 ? p->text
                    : v->names                ? v->names[j]
                                              : NULL;
      k++;
    }
  }
  return r;
}
static int ismissing(double x) { return x == S2_NA || x != x; }
static Value *summary(const char *name, Node *args, Env *env) {
  Value *v, *r, *remove;
  Node *p;
  double *x, z, lo, hi;
  int i, n, missing, rm;
  x = (double *)s2_alloc(S2_MAX_VECTOR * sizeof(double));
  n = missing = 0;
  remove = NULL;
  rm = 0;
  (void)remove;
  for (p = args; p; p = p->next) {

    if (!p->a)
      s2_fail("missing summary argument");
    v = s2_eval(p->a, p->scope ? p->scope : env);
    if (v->type != V_NUM && v->type != V_NULL)
      s2_fail("numeric data required");
    for (i = 0; i < v->length; i++) {
      if (ismissing(v->num[i])) {
        if (!rm)
          missing = 1;
        continue;
      }
      if (n == S2_MAX_VECTOR)
        s2_fail("summary input too long");
      x[n++] = v->num[i];
    }
  }
  if (!strcmp(name, "any") || !strcmp(name, "all")) {
    z = !strcmp(name, "all");
    for (i = 0; i < n; i++) {
      if (!strcmp(name, "any") && x[i] != 0) {
        z = 1;
        missing = 0;
        break;
      }
      if (!strcmp(name, "all") && x[i] == 0) {
        z = 0;
        missing = 0;
        break;
      }
    }
    r = s2_number(missing ? S2_NA : z);
    r->logical = 1;
    return r;
  }
  if (missing)
    return s2_number(S2_NA);
  if (!strcmp(name, "sum"))
    return s2_number(s2_sum(x, n));
  if (!strcmp(name, "mean"))
    return s2_number(n ? s2_mean(x, n) : S2_NA);
  if (!strcmp(name, "var"))
    return s2_number(n > 1 ? s2_var(x, n) : S2_NA);
  if (!strcmp(name, "prod")) {
    z = 1;
    for (i = 0; i < n; i++)
      z *= x[i];
    return s2_number(z);
  }
  if (!n)
    s2_fail("empty min/max/range not yet specified");
  lo = hi = x[0];
  for (i = 1; i < n; i++) {
    if (x[i] < lo)
      lo = x[i];
    if (x[i] > hi)
      hi = x[i];
  }
  if (!strcmp(name, "min"))
    return s2_number(lo);
  if (!strcmp(name, "max"))
    return s2_number(hi);
  r = s2_value(V_NUM, 2);
  r->num[0] = lo;
  r->num[1] = hi;
  return r;
}
static int mathname(const char *name) {
  static const char *names[] = {"sqrt",    "abs",   "sin",   "cos",   "tan",
                                "asin",    "acos",  "atan",  "sinh",  "cosh",
                                "tanh",    "exp",   "log",   "log10", "floor",
                                "ceiling", "trunc", "round", NULL};
  int i;
  for (i = 0; names[i]; i++)
    if (!strcmp(name, names[i]))
      return i + 1;
  return 0;
}
Value *s2_builtin(const char *name, Node *args, Env *env) {
  Value *v, *r, *w;
  Node *p;
  double x, y, z, from, to, by;
  int i, j, k, n, m, rows, cols, byrow, code;
  args = s2_normalize(name, args);
  r = s2_group(name, args, env);
  if (r)
    return r;
  r = s2_math(name, args, env);
  if (r)
    return r;
  r = s2_extra(name, args, env);
  if (r)
    return r;
  r = s2_linear(name, args, env);
  if (r)
    return r;
  r = s2_statistics(name, args, env);
  if (r)
    return r;
  r = s2_io(name, args, env);
  if (r)
    return r;
  r = s2_graphics(name, args, env);
  if (r)
    return r;
  if (!strcmp(name, "c") || !strcmp(name, "list"))
    return combine(args, env, !strcmp(name, "list"));
  if (!strcmp(name, "return")) {

    r = s2_arg(args, 0, "value", env, s2_value(V_NULL, 0));
    s2_transfer(1, r);
    return r;
  }
  if (!strcmp(name, "stop") || !strcmp(name, "warning")) {
    v = s2_arg(args, 0, "message", env, s2_string("stop requested"));
    if (v->type != V_STR || v->length != 1)
      s2_fail("message string required");
    if (!strcmp(name, "stop"))
      s2_fail(v->str[0]);
    fprintf(stderr, "Warning: %s\n", v->str[0]);
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "q")) {
    s2_graphics_close();
    s2_free_all();
    exit(0);
  }
  if (!strcmp(name, "print")) {
    r = s2_arg(args, 0, "x", env, NULL);
    if (!r)
      s2_fail("print needs an object");
    s2_print(r);
    s2_visible = 0;
    return r;
  }
  if (!strcmp(name, "cat")) {
    for (p = args; p; p = p->next) {
      if (p->text)
        s2_fail("cat options not yet supported");
      v = s2_eval(p->a, p->scope ? p->scope : env);
      for (i = 0; i < v->length; i++) {
        if (v->type == V_STR)
          fputs(v->str[i] ? v->str[i] : "NA", stdout);
        else if (v->type == V_NUM) {
          if (ismissing(v->num[i]))
            fputs("NA", stdout);
          else
            printf("%.10g", v->num[i]);
        } else
          s2_fail("cat requires atomic vectors");
        if (i + 1 < v->length)
          putchar(' ');
      }
    }
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "numeric") || !strcmp(name, "logical") ||
      !strcmp(name, "character")) {
    n = s2_count(s2_arg(args, 0, "length", env, s2_number(0)));
    r = s2_value(!strcmp(name, "character") ? V_STR : V_NUM, n);
    r->logical = !strcmp(name, "logical");
    if (r->type == V_STR)
      for (i = 0; i < n; i++)
        r->str[i] = s2_strdup("");
    return r;
  }
  if (!strcmp(name, "sum") || !strcmp(name, "mean") || !strcmp(name, "var") ||
      !strcmp(name, "prod") || !strcmp(name, "min") || !strcmp(name, "max") ||
      !strcmp(name, "range") || !strcmp(name, "any") || !strcmp(name, "all"))
    return summary(name, args, env);
  if (!strcmp(name, "seq")) {
    v = s2_arg(args, -1, "along", env, NULL);
    if (v) {
      r = s2_value(V_NUM, v->length);
      for (i = 0; i < v->length; i++)
        r->num[i] = i + 1;
      return r;
    }
    from = s2_scalar(s2_arg(args, 0, "from", env, s2_number(1)));
    w = s2_arg(args, 1, "to", env, NULL);
    if (!w) {
      to = from;
      from = 1;
    } else
      to = s2_scalar(w);
    v = s2_arg(args, -1, "length", env, NULL);
    if (v) {
      n = s2_count(v);
      by = n > 1 ? (to - from) / (n - 1) : 0;
    } else {
      by =
          s2_scalar(s2_arg(args, 2, "by", env, s2_number(from <= to ? 1 : -1)));
      if (by == 0 || (to - from) / by < 0 || (to - from) / by >= S2_MAX_VECTOR)
        s2_fail("invalid seq step/length");
      n = (int)floor((to - from) / by + 1e-10) + 1;
    }
    r = s2_value(V_NUM, n);
    for (i = 0; i < n; i++)
      r->num[i] = from + i * by;
    return r;
  }
  v = s2_arg(args, 0, "x", env, NULL);
  if (!v && !strcmp(name, "matrix"))
    v = s2_arg(args, 0, "data", env, NULL);
  if (!v)
    s2_fail("unknown function or missing first argument");
  if (!strcmp(name, "length"))
    return s2_number(v->length);
  if (!strcmp(name, "mode") || !strcmp(name, "storage.mode")) {
    r = s2_value(V_STR, 1);
    r->str[0] =
        s2_strdup(v->type == V_NUM       ? (v->logical ? "logical" : "numeric")
                  : v->type == V_STR     ? "character"
                  : v->type == V_LIST    ? "list"
                  : v->type == V_FUNC    ? "function"
                  : v->type == V_EXPR    ? "expression"
                  : v->type == V_COMPLEX ? "complex"
                  : v->type == V_LANG
                      ? (v->body && v->body->kind == N_NAME ? "name" : "call")
                      : "null");
    if (!strcmp(name, "storage.mode") && v->type == V_NUM && !v->logical)
      r->str[0] = s2_strdup(v->integer ? "integer" : "double");
    return r;
  }
  if (!strcmp(name, "names")) {
    if (!v->names)
      return s2_value(V_NULL, 0);
    r = s2_value(V_STR, v->length);
    for (i = 0; i < v->length; i++)
      r->str[i] = v->names[i] ? v->names[i] : s2_strdup("");
    return r;
  }
  if (!strcmp(name, "dim")) {
    if (!v->rows)
      return s2_value(V_NULL, 0);
    r = s2_value(V_NUM, 2);
    r->num[0] = v->rows;
    r->num[1] = v->cols;
    return r;
  }
  if (!strcmp(name, "is.na")) {
    r = s2_value(V_NUM, v->length);
    r->logical = 1;
    for (i = 0; i < v->length; i++)
      r->num[i] = v->type == V_NUM   ? ismissing(v->num[i])
                  : v->type == V_STR ? v->str[i] == NULL
                                     : 0;
    return r;
  }
  if (!strcmp(name, "rep")) {
    w = s2_arg(args, 1, "times", env, NULL);
    r = s2_arg(args, 2, "length", env, NULL);
    if (r) {
      n = s2_count(r);
      if (n && !v->length)
        s2_fail("cannot repeat empty vector");
      w = s2_value(V_NUM, n);
      for (i = 0; i < n; i++)
        w->num[i] = i % v->length + 1;
      r = s2_subset(v, s2_constant_args(w), env, 0, NULL);
      return r;
    }
    if (!w || w->type != V_NUM)
      s2_fail("rep needs times or length");
    if (w->length != 1 && w->length != v->length)
      s2_fail("invalid rep counts");
    n = 0;
    for (i = 0; i < w->length; i++) {
      x = w->num[i];
      if (ismissing(x) || x < 0 || x > S2_MAX_VECTOR || floor(x) != x)
        s2_fail("invalid rep count");
      if ((long)n + (long)x * (w->length == 1 ? v->length : 1) > S2_MAX_VECTOR)
        s2_fail("rep too long");
      n += (int)x * (w->length == 1 ? v->length : 1);
    }
    r = s2_value(V_NUM, n);
    k = 0;
    for (i = 0; i < (w->length == 1 ? (int)w->num[0] : v->length); i++)
      for (j = 0; j < (w->length == 1 ? v->length : (int)w->num[i]); j++)
        r->num[k++] = (w->length == 1 ? j : i) + 1;
    return s2_subset(v, s2_constant_args(r), env, 0, NULL);
  }
  if (!strcmp(name, "rev") || !strcmp(name, "sort")) {
    if (v->type != V_NUM && v->type != V_STR)
      s2_fail("atomic vector required");
    r = s2_copy(v);
    if (!strcmp(name, "rev")) {
      for (i = 0; i < v->length; i++) {
        if (v->type == V_NUM)
          r->num[i] = v->num[v->length - i - 1];
        else
          r->str[i] = v->str[v->length - i - 1];
      }
    } else
      for (i = 1; i < r->length; i++)
        for (j = i; j > 0; j--) {
          if (v->type == V_NUM) {
            if (ismissing(r->num[j]) || ismissing(r->num[j - 1]))
              s2_fail("sort NA policy pending");
            if (r->num[j] >= r->num[j - 1])
              break;
            x = r->num[j];
            r->num[j] = r->num[j - 1];
            r->num[j - 1] = x;
          } else {
            char *s;
            if (!r->str[j] || !r->str[j - 1])
              s2_fail("sort NA policy pending");
            if (strcmp(r->str[j], r->str[j - 1]) >= 0)
              break;
            s = r->str[j];
            r->str[j] = r->str[j - 1];
            r->str[j - 1] = s;
          }
        }
    r->rows = r->cols = 0;
    r->names = NULL;
    return r;
  }
  if (!strcmp(name, "matrix")) {
    w = s2_arg(args, 1, "nrow", env, NULL);
    rows = w ? s2_count(w) : 0;
    w = s2_arg(args, 2, "ncol", env, NULL);
    cols = w ? s2_count(w) : 0;
    if (!rows && !cols) {
      rows = v->length;
      cols = 1;
    } else if (!rows)
      rows = (v->length + cols - 1) / cols;
    else if (!cols)
      cols = (v->length + rows - 1) / rows;
    if ((long)rows * cols > S2_MAX_VECTOR || (rows && cols && !v->length))
      s2_fail("invalid matrix size");
    r = s2_value(v->type, rows * cols);
    r->rows = rows;
    r->cols = cols;
    r->logical = v->logical;
    byrow = s2_scalar(s2_arg(args, 3, "byrow", env, s2_number(0))) != 0;
    for (i = 0; i < r->length; i++) {
      j = byrow ? (i % rows) * cols + i / rows : i;
      k = j % v->length;
      if (v->type == V_NUM)
        r->num[i] = v->num[k];
      else if (v->type == V_STR)
        r->str[i] = v->str[k];
      else if (v->type == V_COMPLEX) {
        r->num[i] = v->num[k];
        r->imag[i] = v->imag[k];
      } else if (v->type == V_LIST)
        r->items[i] = v->items[k];
      else
        s2_fail("matrix requires vector data");
    }
    w = s2_value(V_NUM, 2);
    w->num[0] = rows;
    w->num[1] = cols;
    r = s2_with_attr(r, "dim", w);
    w = s2_arg(args, 4, "dimnames", env, NULL);
    if (w)
      r = s2_with_attr(r, "dimnames", w);
    return r;
  }
  if (v->type != V_NUM)
    s2_fail("numeric vector required");
  if (!strcmp(name, "diff")) {
    m = s2_count(s2_arg(args, 1, "lag", env, s2_number(1)));
    k = s2_count(s2_arg(args, 2, "differences", env, s2_number(1)));
    if (!m || !k)
      s2_fail("positive lag and differences required");
    for (j = 0; j < k; j++) {
      n = v->length > m ? v->length - m : 0;
      r = s2_value(V_NUM, n);
      for (i = 0; i < n; i++)
        r->num[i] = ismissing(v->num[i]) || ismissing(v->num[i + m])
                        ? S2_NA
                        : v->num[i + m] - v->num[i];
      v = r;
    }
    return v;
  }
  if (!strcmp(name, "cumsum") || !strcmp(name, "cumprod") ||
      !strcmp(name, "cummin") || !strcmp(name, "cummax")) {
    n = v->length;
    r = s2_value(V_NUM, n);
    z = !strcmp(name, "cumprod") ? 1 : 0;
    for (i = 0; i < n; i++) {
      x = v->num[i];
      if (ismissing(z) || ismissing(x))
        z = S2_NA;
      else if (!strcmp(name, "cumsum"))
        z += x;
      else if (!strcmp(name, "cumprod"))
        z *= x;
      else if (!i)
        z = x;
      else if (!strcmp(name, "cummin")) {
        if (x < z)
          z = x;
      } else if (x > z)
        z = x;
      r->num[i] = z;
    }
    return r;
  }
  code = mathname(name);
  if (!code)
    s2_fail("unknown function");
  r = s2_copy(v);
  r->logical = 0;
  y = code == 13 ? s2_scalar(s2_arg(args, 1, "base", env, s2_number(exp(1.0))))
      : code == 18 ? s2_scalar(s2_arg(args, 1, "digits", env, s2_number(0)))
                   : 0;
  if (code == 13 && (y <= 0 || y == 1))
    s2_fail("invalid logarithm base");
  if (code == 18 && (fabs(y) > 15 || floor(y) != y))
    s2_fail("round digits must be integer from -15 to 15");
  for (i = 0; i < v->length; i++) {
    x = v->num[i];
    if (ismissing(x)) {
      r->num[i] = S2_NA;
      continue;
    }
    z = S2_NA;
    switch (code) {
    case 1:
      z = x < 0 ? S2_NA : sqrt(x);
      break;
    case 2:
      z = fabs(x);
      break;
    case 3:
      z = sin(x);
      break;
    case 4:
      z = cos(x);
      break;
    case 5:
      z = tan(x);
      break;
    case 6:
      z = fabs(x) > 1 ? S2_NA : asin(x);
      break;
    case 7:
      z = fabs(x) > 1 ? S2_NA : acos(x);
      break;
    case 8:
      z = atan(x);
      break;
    case 9:
      z = sinh(x);
      break;
    case 10:
      z = cosh(x);
      break;
    case 11:
      z = tanh(x);
      break;
    case 12:
      z = exp(x);
      break;
    case 13:
      z = x <= 0 ? S2_NA : log(x) / log(y);
      break;
    case 14:
      z = x <= 0 ? S2_NA : log10(x);
      break;
    case 15:
      z = floor(x);
      break;
    case 16:
      z = ceil(x);
      break;
    case 17:
      z = x < 0 ? ceil(x) : floor(x);
      break;
    case 18:
      z = pow(10, y);
      z = floor(x * z + 0.5) / z;
      break;
    }
    r->num[i] = z;
  }
  return r;
}
