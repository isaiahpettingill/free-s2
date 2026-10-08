#include "s2.h"
static double rounded(double x) {
  double a, b;
  a = fabs(x);
  b = floor(a);
  if (a - b > .5 || (a - b == .5 && fmod(b, 2) != 0))
    b++;
  return x < 0 ? -b : b;
}
Value *s2_complex_binary(Value *a, Value *b, int op) {
  Value *r, *longer;
  int i, n, code;
  double ar, ai, br, bi, d, t, u, v;
  code = s2_token_binary(op);
  n = !a->length || !b->length ? 0
      : a->length > b->length  ? a->length
                               : b->length;
  if (op != '+' && op != '-' && op != '*' && op != '/' && op != '^' &&
      code != 1 && code != 2)
    s2_fail("operator not defined for complex data");
  a = s2_coerce(a, V_COMPLEX, 0);
  b = s2_coerce(b, V_COMPLEX, 0);
  r = s2_value(code == 1 || code == 2 ? V_NUM : V_COMPLEX, n);
  r->logical = code == 1 || code == 2;
  longer = a->length >= b->length ? a : b;
  r->names = longer->names;
  r->attrs = longer->attrs;
  r->rows = longer->rows;
  r->cols = longer->cols;
  if (n && (n % a->length || n % b->length))
    fprintf(stderr, "Warning: fractional vector recycling\n");
  for (i = 0; i < n; i++) {
    ar = a->num[i % a->length];
    ai = a->imag[i % a->length];
    br = b->num[i % b->length];
    bi = b->imag[i % b->length];
    u = v = 0;
    if (s2_missing(ar) || s2_missing(ai) || s2_missing(br) || s2_missing(bi))
      u = v = S2_NA;
    else if (code == 1 || code == 2)
      u = code == 1 ? (ar == br && ai == bi) : (ar != br || ai != bi);
    else if (op == '+') {
      u = ar + br;
      v = ai + bi;
    } else if (op == '-') {
      u = ar - br;
      v = ai - bi;
    } else if (op == '*') {
      u = ar * br - ai * bi;
      v = ar * bi + ai * br;
    } else if (op == '/') {
      d = br * br + bi * bi;
      if (d == 0)
        u = v = S2_NA;
      else {
        u = (ar * br + ai * bi) / d;
        v = (ai * br - ar * bi) / d;
      }
    } else if (ar == 0 && ai == 0) {
      if (bi == 0 && br > 0)
        u = v = 0;
      else if (br == 0 && bi == 0) {
        u = 1;
        v = 0;
      } else
        u = v = S2_NA;
    } else {
      d = log(sqrt(ar * ar + ai * ai));
      t = atan2(ai, ar);
      u = exp(br * d - bi * t);
      v = bi * d + br * t;
      t = u;
      u = t * cos(v);
      v = t * sin(v);
    }
    r->num[i] = u;
    if (r->type == V_COMPLEX)
      r->imag[i] = v;
  }
  return r;
}
Value *s2_math(const char *name, Node *args, Env *env) {
  static const char *names[] = {
      "sqrt",  "abs",   "sin",    "cos",  "tan",   "asin",  "acos",  "atan",
      "sinh",  "cosh",  "tanh",   "exp",  "log",   "log10", "floor", "ceiling",
      "trunc", "round", "signif", "sign", "asinh", "acosh", "atanh", NULL};
  Value *v, *w, *r, *a, *b;
  double x, y, z, d, u, t;
  int i, n, code, missing;
  if (!strcmp(name, "complex") || !strcmp(name, "integer") ||
      !strcmp(name, "vector")) {
    if (!strcmp(name, "complex")) {
      a = s2_arg(args, 1, "real", env, NULL);
      b = s2_arg(args, 2, "imaginary", env, NULL);
      n = s2_count(s2_arg(args, 0, "length", env, s2_number(0)));
      if (!a)
        a = s2_value(V_NUM, b && b->length > n ? b->length : n);
      if (!b)
        b = s2_value(V_NUM, a->length > n ? a->length : n);
      if (a->type != V_NUM || b->type != V_NUM)
        s2_fail("numeric complex components required");
      if (a->length > n)
        n = a->length;
      if (b->length > n)
        n = b->length;
      if (!a->length || !b->length)
        n = 0;
      r = s2_value(V_COMPLEX, n);
      for (i = 0; i < n; i++) {
        r->num[i] = a->num[i % a->length];
        r->imag[i] = b->num[i % b->length];
      }
      return r;
    }
    if (!strcmp(name, "integer")) {
      n = s2_count(s2_arg(args, 0, "length", env, s2_number(0)));
      r = s2_value(V_NUM, n);
      r->integer = 1;
      return r;
    }
    v = s2_arg(args, 0, "mode", env, s2_string("logical"));
    if (v->type != V_STR || v->length != 1)
      s2_fail("mode string required");
    n = s2_count(s2_arg(args, 1, "length", env, s2_number(0)));
    code = !strcmp(v->str[0], "numeric") || !strcmp(v->str[0], "integer") ||
                   !strcmp(v->str[0], "logical")
               ? V_NUM
           : !strcmp(v->str[0], "character") ? V_STR
           : !strcmp(v->str[0], "complex")   ? V_COMPLEX
           : !strcmp(v->str[0], "list")      ? V_LIST
                                             : -1;
    if (code < 0)
      s2_fail("unsupported vector mode");
    r = s2_value(code, n);
    r->logical = !strcmp(v->str[0], "logical");
    r->integer = !strcmp(v->str[0], "integer");
    for (i = 0; i < n; i++) {
      if (code == V_STR)
        r->str[i] = s2_strdup("");
      if (code == V_LIST)
        r->items[i] = s2_value(V_NULL, 0);
    }
    return r;
  }
  if (!strcmp(name, "atan2")) {
    a = s2_arg(args, 0, "y", env, NULL);
    b = s2_arg(args, 1, "x", env, NULL);
    if (!a || !b || a->type != V_NUM || b->type != V_NUM)
      s2_fail("numeric atan2 data required");
    n = !a->length || !b->length ? 0
        : a->length > b->length  ? a->length
                                 : b->length;
    r = s2_value(V_NUM, n);
    for (i = 0; i < n; i++) {
      x = a->num[i % a->length];
      y = b->num[i % b->length];
      r->num[i] = s2_missing(x) || s2_missing(y) ? S2_NA : atan2(x, y);
    }
    return r;
  }
  if (!strcmp(name, "pmin") || !strcmp(name, "pmax")) {
    Node *p;
    n = 0;
    for (p = args; p; p = p->next) {
      v = s2_eval(p->a, p->scope ? p->scope : env);
      if (v->type != V_NUM)
        s2_fail("numeric pmin/pmax data required");
      if (!v->length)
        return s2_value(V_NUM, 0);
      if (v->length > n)
        n = v->length;
      p->value = v;
    }
    r = s2_value(V_NUM, n);
    for (i = 0; i < n; i++) {
      z = 0;
      missing = 0;
      for (p = args; p; p = p->next) {
        v = p->value;
        x = v->num[i % v->length];
        if (s2_missing(x)) {
          missing = 1;
          break;
        }
        if (p == args || (!strcmp(name, "pmin") ? x < z : x > z))
          z = x;
      }
      r->num[i] = missing ? S2_NA : z;
    }
    return r;
  }
  for (code = 0; names[code]; code++)
    if (!strcmp(name, names[code]))
      break;
  if (!names[code])
    return NULL;
  v = s2_arg(args, 0, "x", env, NULL);
  if (!v || (v->type != V_NUM && v->type != V_COMPLEX))
    s2_fail("numeric or complex data required");
  r = s2_copy(v);
  r->logical = r->integer = 0;
  w = s2_arg(args, 1, "digits", env,
             s2_number(!strcmp(name, "signif") ? 6 : 0));
  d = s2_scalar(w);
  if ((!strcmp(name, "round") || !strcmp(name, "signif")) &&
      (fabs(d) > 15 || floor(d) != d))
    s2_fail("invalid rounding digits");
  w = s2_arg(args, 1, "base", env, s2_number(exp(1.0)));
  t = s2_scalar(w);
  if (!strcmp(name, "log") && (t <= 0 || t == 1))
    s2_fail("invalid logarithm base");
  if (v->type == V_COMPLEX && !strcmp(name, "abs")) {
    r = s2_value(V_NUM, v->length);
    for (i = 0; i < v->length; i++)
      r->num[i] = s2_missing(v->num[i]) || s2_missing(v->imag[i])
                      ? S2_NA
                      : sqrt(v->num[i] * v->num[i] + v->imag[i] * v->imag[i]);
    return r;
  }
  for (i = 0; i < v->length; i++) {
    x = v->num[i];
    y = v->type == V_COMPLEX ? v->imag[i] : 0;
    z = u = 0;
    if (s2_missing(x) || s2_missing(y)) {
      r->num[i] = S2_NA;
      if (r->type == V_COMPLEX)
        r->imag[i] = S2_NA;
      continue;
    }
    if (v->type == V_COMPLEX) {
      if (!strcmp(name, "sqrt")) {
        z = sqrt((sqrt(x * x + y * y) + x) / 2);
        u = (y < 0 ? -1 : 1) * sqrt((sqrt(x * x + y * y) - x) / 2);
      } else if (!strcmp(name, "exp")) {
        z = exp(x) * cos(y);
        u = exp(x) * sin(y);
      } else if (!strcmp(name, "log") || !strcmp(name, "log10")) {
        z = log(sqrt(x * x + y * y));
        u = atan2(y, x);
        z /= log(!strcmp(name, "log10") ? 10 : t);
        u /= log(!strcmp(name, "log10") ? 10 : t);
      } else if (!strcmp(name, "sin")) {
        z = sin(x) * cosh(y);
        u = cos(x) * sinh(y);
      } else if (!strcmp(name, "cos")) {
        z = cos(x) * cosh(y);
        u = -sin(x) * sinh(y);
      } else if (!strcmp(name, "sinh")) {
        z = sinh(x) * cos(y);
        u = cosh(x) * sin(y);
      } else if (!strcmp(name, "cosh")) {
        z = cosh(x) * cos(y);
        u = sinh(x) * sin(y);
      } else
        s2_fail("complex transformation not implemented");
      r->num[i] = z;
      r->imag[i] = u;
      continue;
    }
    if (!strcmp(name, "sqrt"))
      z = x < 0 ? S2_NA : sqrt(x);
    else if (!strcmp(name, "abs"))
      z = fabs(x);
    else if (!strcmp(name, "sin"))
      z = sin(x);
    else if (!strcmp(name, "cos"))
      z = cos(x);
    else if (!strcmp(name, "tan"))
      z = tan(x);
    else if (!strcmp(name, "asin"))
      z = fabs(x) > 1 ? S2_NA : asin(x);
    else if (!strcmp(name, "acos"))
      z = fabs(x) > 1 ? S2_NA : acos(x);
    else if (!strcmp(name, "atan"))
      z = atan(x);
    else if (!strcmp(name, "sinh"))
      z = sinh(x);
    else if (!strcmp(name, "cosh"))
      z = cosh(x);
    else if (!strcmp(name, "tanh"))
      z = tanh(x);
    else if (!strcmp(name, "exp"))
      z = exp(x);
    else if (!strcmp(name, "log"))
      z = x <= 0 ? S2_NA : log(x) / log(t);
    else if (!strcmp(name, "log10"))
      z = x <= 0 ? S2_NA : log10(x);
    else if (!strcmp(name, "floor"))
      z = floor(x);
    else if (!strcmp(name, "ceiling"))
      z = ceil(x);
    else if (!strcmp(name, "trunc"))
      z = x < 0 ? ceil(x) : floor(x);
    else if (!strcmp(name, "sign"))
      z = x > 0 ? 1 : x < 0 ? -1 : 0;
    else if (!strcmp(name, "asinh"))
      z = (x < 0 ? -1 : 1) * log(fabs(x) + sqrt(x * x + 1));
    else if (!strcmp(name, "acosh"))
      z = x < 1 ? S2_NA : log(x + sqrt(x * x - 1));
    else if (!strcmp(name, "atanh"))
      z = fabs(x) >= 1 ? S2_NA : .5 * log((1 + x) / (1 - x));
    else {
      y = d;
      if (!strcmp(name, "signif")) {
        if (d < 1)
          s2_fail("significant digits must be positive");
        y = x == 0 ? 0 : d - 1 - floor(log10(fabs(x)));
      }
      y = pow(10, y);
      z = rounded(x * y) / y;
    }
    r->num[i] = z;
  }
  return r;
}
