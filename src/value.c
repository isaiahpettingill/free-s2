#include "s2.h"
Env s2_global, s2_session;
Env *s2_expression;
Control *s2_function_control, *s2_loop_control;
int s2_depth, s2_flow, s2_visible;
Value *s2_return;
Value *s2_value(int type, int length) {
  Value *v;
  if (length < 0 || length > S2_MAX_VECTOR)
    s2_fail("vector length limit (2048)");
  v = (Value *)s2_alloc(sizeof(Value));
  v->type = type;
  v->length = length;
  if (length && (type == V_NUM || type == V_COMPLEX))
    v->num = (double *)s2_alloc((size_t)length * sizeof(double));
  if (length && type == V_COMPLEX)
    v->imag = (double *)s2_alloc((size_t)length * sizeof(double));
  if (length && type == V_STR)
    v->str = (char **)s2_alloc((size_t)length * sizeof(char *));
  if (length && (type == V_LIST || type == V_EXPR))
    v->items = (Value **)s2_alloc((size_t)length * sizeof(Value *));
  return v;
}
Value *s2_number(double x) {
  Value *v;
  v = s2_value(V_NUM, 1);
  v->num[0] = x;
  return v;
}
Value *s2_copy(Value *v) {
  Value *r;
  int i;
  r = s2_value(v->type, v->length);
  r->logical = v->logical;
  r->rows = v->rows;
  r->cols = v->cols;
  r->names = v->names;
  r->formals = v->formals;
  r->body = v->body;
  r->builtin = v->builtin;
  r->integer = v->integer;
  r->attrs = v->attrs;
  for (i = 0; i < v->length; i++) {
    if (v->type == V_NUM || v->type == V_COMPLEX)
      r->num[i] = v->num[i];
    if (v->type == V_COMPLEX)
      r->imag[i] = v->imag[i];
    if (v->type == V_STR)
      r->str[i] = v->str[i];
    if (v->type == V_LIST || v->type == V_EXPR)
      r->items[i] = v->items[i];
  }
  return r;
}
Binding *s2_binding(Env *env, const char *name) {
  Binding *b;
  for (b = env->bindings; b; b = b->next)
    if (!strcmp(b->name, name))
      return b;
  return NULL;
}
void s2_set(Env *env, const char *name, Value *v) {
  Binding *b;
  if (!strcmp(name, "T") || !strcmp(name, "F") || !strcmp(name, "TRUE") ||
      !strcmp(name, "FALSE") || !strcmp(name, "NA") || !strcmp(name, "NULL") ||
      !strcmp(name, "if") || !strcmp(name, "else") || !strcmp(name, "for") ||
      !strcmp(name, "while") || !strcmp(name, "repeat") ||
      !strcmp(name, "next") || !strcmp(name, "break") || !strcmp(name, "in") ||
      !strcmp(name, "function") || !strcmp(name, "return"))
    s2_fail("assignment to reserved name");
  b = s2_binding(env, name);
  if (!b) {
    b = (Binding *)s2_alloc(sizeof(Binding));
    b->name = s2_strdup(name);
    b->next = env->bindings;
    env->bindings = b;
  }
  b->value = v;
  b->promise = NULL;
  b->forcing = 0;
}
Value *s2_lookup(Env *env, const char *name) {
  Binding *b;
  b = s2_binding(env, name);
  if (!b && s2_expression && env != s2_expression)
    b = s2_binding(s2_expression, name);
  if (!b)
    b = s2_binding(&s2_session, name);
  if (!b && env != &s2_global)
    b = s2_binding(&s2_global, name);
  if (!b) {
    if (!s2_builtin_exists(name))
      return NULL;
    {
      Value *fn;
      fn = s2_value(V_FUNC, 1);
      fn->builtin = s2_strdup(name);
      return fn;
    }
  }
  if (!b->value) {
    if (!b->promise)
      s2_fail("argument is missing");
    if (b->forcing)
      s2_fail("recursive promise evaluation");
    b->forcing = 1;
    b->value = s2_eval(b->promise, b->caller);
    b->forcing = 0;
  }
  return b->value;
}
static void print_num(double x, int logical) {
  if (x == S2_NA || x != x)
    printf("NA");
  else if (logical)
    printf(x != 0 ? "T" : "F");
  else
    printf("%.10g", x);
}
void s2_print(Value *v) {
  int i, j;
  if (v->type == V_NULL) {
    puts("NULL");
    return;
  }
  if (v->type == V_FUNC) {
    puts("<function>");
    return;
  }
  if (v->type == V_EXPR || v->type == V_LANG) {
    s2_write_value(stdout, v);
    putchar('\n');
    return;
  }
  if (v->type == V_LIST) {
    for (i = 0; i < v->length; i++) {
      if (v->names && v->names[i])
        printf("$%s\n", v->names[i]);
      else
        printf("[[%d]]\n", i + 1);
      s2_print(v->items[i]);
    }
    if (!v->length)
      puts("list()");
    return;
  }
  if (v->rows && v->type == V_NUM) {
    for (i = 0; i < v->rows; i++) {
      printf("[%d,]", i + 1);
      for (j = 0; j < v->cols; j++) {
        putchar(' ');
        print_num(v->num[j * v->rows + i], v->logical);
      }
      putchar('\n');
    }
    return;
  }
  if (!v->length) {
    puts(v->type == V_STR ? "character(0)"
                          : (v->logical ? "logical(0)" : "numeric(0)"));
    return;
  }
  for (i = 0; i < v->length; i++) {
    if (i % 8 == 0) {
      if (i)
        putchar('\n');
      printf("[%d]", i + 1);
    }
    putchar(' ');
    if (v->type == V_NUM || v->type == V_COMPLEX) {
      print_num(v->num[i], v->logical);
      if (v->type == V_COMPLEX) {
        if (v->imag[i] >= 0)
          putchar('+');
        print_num(v->imag[i], 0);
        putchar('i');
      }
    } else
      printf("\"%s\"", v->str[i] ? v->str[i] : "NA");
  }
  putchar('\n');
}

int s2_missing(double x) { return x == S2_NA || x != x; }
Value *s2_string(const char *s) {
  Value *r;
  r = s2_value(V_STR, 1);
  r->str[0] = s ? s2_strdup(s) : NULL;
  return r;
}
Value *s2_element(Value *v, int i) {
  Value *r;
  if (v->type == V_LIST || v->type == V_EXPR)
    return i >= 0 && i < v->length ? v->items[i] : s2_value(V_NULL, 0);
  r = s2_value(v->type, 1);
  r->logical = v->logical;
  r->integer = v->integer;
  if (v->type == V_NUM || v->type == V_COMPLEX)
    r->num[0] = i >= 0 && i < v->length ? v->num[i] : S2_NA;
  if (v->type == V_COMPLEX)
    r->imag[0] = i >= 0 && i < v->length ? v->imag[i] : S2_NA;
  if (v->type == V_STR)
    r->str[0] = i >= 0 && i < v->length ? v->str[i] : NULL;
  return r;
}
int s2_common(Value *a, Value *b) {
  if (a->type == V_LIST || b->type == V_LIST)
    return V_LIST;
  if (a->type == V_STR || b->type == V_STR)
    return V_STR;
  if (a->type == V_COMPLEX || b->type == V_COMPLEX)
    return V_COMPLEX;
  return V_NUM;
}
Value *s2_coerce(Value *v, int type, int logical) {
  Value *r, *e;
  int i;
  double x;
  char buf[100], *end;
  r = s2_value(type, v->length);
  r->logical = logical;
  for (i = 0; i < v->length; i++) {
    if (type == V_STR && v->type == V_NUM) {
      Value *levels;
      levels = s2_attr(v, "levels");
      if (levels->type == V_STR) {
        r->str[i] = !s2_missing(v->num[i]) && v->num[i] >= 1 &&
                            v->num[i] <= levels->length
                        ? levels->str[(int)v->num[i] - 1]
                        : NULL;
        continue;
      }
    }
    if (type == V_LIST) {
      r->items[i] = s2_element(v, i);
      continue;
    }
    x = S2_NA;
    e = v->type == V_LIST ? v->items[i] : s2_element(v, i);
    if (e->length != 1)
      s2_fail("cannot coerce nonscalar list component");
    if (type == V_STR) {
      if (e->type == V_STR)
        r->str[i] = e->str[0];
      else if (e->type == V_NUM || e->type == V_COMPLEX) {
        if (s2_missing(e->num[0]))
          r->str[i] = NULL;
        else {
          if (e->type == V_COMPLEX)
            sprintf(buf, "%.15g%+.15gi", e->num[0], e->imag[0]);
          else if (e->logical)
            strcpy(buf, e->num[0] ? "T" : "F");
          else
            sprintf(buf, "%.15g", e->num[0]);
          r->str[i] = s2_strdup(buf);
        }
      } else
        s2_fail("cannot coerce to character");
    } else {
      if (e->type == V_NUM || e->type == V_COMPLEX)
        x = e->num[0];
      else if (e->type == V_STR) {
        if (!e->str[0])
          x = S2_NA;
        else if (logical)
          x = e->str[0][0] != 0;
        else {
          x = strtod(e->str[0], &end);
          if (*end || end == e->str[0])
            x = S2_NA;
        }
      } else
        s2_fail("cannot coerce to numeric");
      if (logical && !s2_missing(x))
        x = x != 0 || (e->type == V_COMPLEX && e->imag[0] != 0);
      r->num[i] = x;
      if (type == V_COMPLEX)
        r->imag[i] = e->type == V_COMPLEX ? e->imag[0] : 0;
    }
  }
  r->names = v->names;
  return r;
}
Value *s2_attr(Value *v, const char *name) {
  Attr *a;
  Value *r;
  int i;
  if (!strcmp(name, "names") && v->names) {
    r = s2_value(V_STR, v->length);
    for (i = 0; i < v->length; i++)
      r->str[i] = v->names[i] ? v->names[i] : s2_strdup("");
    return r;
  }
  for (a = v->attrs; a; a = a->next)
    if (!strcmp(a->name, name))
      return a->value;
  if (!strcmp(name, "dim") && v->rows) {
    r = s2_value(V_NUM, 2);
    r->num[0] = v->rows;
    r->num[1] = v->cols;
    return r;
  }
  return s2_value(V_NULL, 0);
}
Value *s2_with_attr(Value *v, const char *name, Value *value) {
  Value *r;
  Attr *a, *q;
  long prod;
  int i;
  r = s2_copy(v);
  r->attrs = NULL;
  for (a = v->attrs; a; a = a->next)
    if (strcmp(a->name, name)) {
      q = (Attr *)s2_alloc(sizeof(Attr));
      *q = *a;
      q->next = r->attrs;
      r->attrs = q;
    }
  if (!strcmp(name, "names")) {
    if (value->type == V_NULL)
      r->names = NULL;
    else {
      value = s2_coerce(value, V_STR, 0);
      if (value->length > r->length)
        s2_fail("too many names");
      r->names = (char **)s2_alloc((size_t)(r->length ? r->length : 1) *
                                   sizeof(char *));
      for (i = 0; i < r->length; i++)
        r->names[i] = i < value->length ? value->str[i] : NULL;
    }
  } else if (!strcmp(name, "dim")) {
    r->rows = r->cols = 0;
    if (value->type != V_NULL) {
      prod = 1;
      if (value->type != V_NUM || !value->length)
        s2_fail("invalid dimensions");
      for (i = 0; i < value->length; i++) {
        if (s2_missing(value->num[i]) || value->num[i] < 0 ||
            floor(value->num[i]) != value->num[i] ||
            value->num[i] > S2_MAX_VECTOR)
          s2_fail("invalid dimension");
        prod *= (long)value->num[i];
        if (prod > S2_MAX_VECTOR)
          s2_fail("array too large");
      }
      if (prod != v->length)
        s2_fail("dimensions do not match length");
      if (value->length == 2) {
        r->rows = (int)value->num[0];
        r->cols = (int)value->num[1];
      }
    }
  }
  if (value->type != V_NULL && strcmp(name, "names")) {
    q = (Attr *)s2_alloc(sizeof(Attr));
    q->name = s2_strdup(name);
    q->value = value;
    q->next = r->attrs;
    r->attrs = q;
  }
  return r;
}
Value *s2_function(Env *env, const char *name) {
  Value *v;
  Binding *b;
  Env *frames[4];
  int i;
  frames[0] = env;
  frames[1] = s2_expression;
  frames[2] = &s2_session;
  frames[3] = &s2_global;
  for (i = 0; i < 4; i++)
    if (frames[i]) {
      b = s2_binding(frames[i], name);
      if (b) {
        if (!b->value && b->promise)
          s2_lookup(frames[i], name);
        v = b->value;
        if (v && v->type == V_FUNC)
          return v;
      }
    }
  if (s2_builtin_exists(name)) {
    v = s2_value(V_FUNC, 1);
    v->builtin = s2_strdup(name);
    return v;
  }
  return NULL;
}
