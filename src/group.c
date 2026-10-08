#include "s2.h"
static int same(Value *x, int i, int j) {
  if (x->type == V_STR)
    return x->str[i] && x->str[j] && !strcmp(x->str[i], x->str[j]);
  return !s2_missing(x->num[i]) && x->num[i] == x->num[j];
}
static Value *category(Value *v) {
  Value *r, *levels, *existing;
  int *order, i, j, k, n;
  double x;
  char *s;
  existing = s2_attr(v, "levels");
  if (v->type == V_NUM && existing->type == V_STR)
    return v;
  if (v->type != V_NUM && v->type != V_STR)
    s2_fail("category needs numeric/character data");
  order = (int *)s2_alloc((size_t)(v->length ? v->length : 1) * sizeof(int));
  n = 0;
  for (i = 0; i < v->length; i++) {
    if (v->type == V_STR ? !v->str[i] : s2_missing(v->num[i]))
      continue;
    for (j = 0; j < n; j++)
      if (same(v, i, order[j]))
        break;
    if (j == n)
      order[n++] = i;
  }
  for (i = 1; i < n; i++)
    for (j = i; j > 0; j--) {
      if (v->type == V_STR ? strcmp(v->str[order[j - 1]], v->str[order[j]]) <= 0
                           : v->num[order[j - 1]] <= v->num[order[j]])
        break;
      k = order[j];
      order[j] = order[j - 1];
      order[j - 1] = k;
    }
  levels = s2_value(V_STR, n);
  for (i = 0; i < n; i++) {
    Value *w;
    w = s2_coerce(s2_element(v, order[i]), V_STR, 0);
    levels->str[i] = w->str[0];
  }
  r = s2_value(V_NUM, v->length);
  r->integer = 1;
  for (i = 0; i < v->length; i++) {
    for (j = 0; j < n; j++)
      if (same(v, i, order[j]))
        break;
    r->num[i] = j < n ? j + 1 : S2_NA;
  }
  r = s2_with_attr(r, "levels", levels);
  r = s2_with_attr(r, "class", s2_string("category"));
  (void)x;
  (void)s;
  return r;
}
static Node *arg(Value *v) {
  Node *a;
  a = (Node *)s2_alloc(sizeof(Node));
  a->a = (Node *)s2_alloc(sizeof(Node));
  a->a->kind = N_VALUE;
  a->a->value = v;
  return a;
}
Value *s2_group(const char *name, Node *args, Env *env) {
  Value *v, *w, *r, *groups, *cats[16], *dims, *dn, *index, *fn, *subset,
      *labels, *breaks, *levels;
  Node *p, *rest, *a;
  int ncat, n, i, j, k, m, cell, missing, ncells, *counts, num;
  double x, lo, hi;
  if (!strcmp(name, "category") || !strcmp(name, "as.category")) {
    v = s2_arg(args, 0, "x", env, NULL);
    if (!v)
      s2_fail("category data required");
    return category(v);
  }
  if (!strcmp(name, "is.category")) {
    v = s2_arg(args, 0, "x", env, NULL);
    r = s2_number(v && v->type == V_NUM && s2_attr(v, "levels")->type == V_STR);
    r->logical = 1;
    return r;
  }
  if (!strcmp(name, "levels")) {
    v = s2_arg(args, 0, "x", env, NULL);
    if (!v)
      s2_fail("object required");
    return s2_attr(v, "levels");
  }
  if (!strcmp(name, "cut")) {
    v = s2_arg(args, 0, "x", env, NULL);
    breaks = s2_arg(args, 1, "breaks", env, NULL);
    if (!v || v->type != V_NUM || !breaks || breaks->type != V_NUM)
      s2_fail("numeric cut data/breaks required");
    if (breaks->length == 1) {
      m = s2_count(breaks);
      if (!m || m >= S2_MAX_VECTOR)
        s2_fail("invalid cut groups");
      lo = hi = 0;
      missing = 1;
      for (i = 0; i < v->length; i++)
        if (!s2_missing(v->num[i])) {
          x = v->num[i];
          if (missing) {
            lo = hi = x;
            missing = 0;
          }
          if (x < lo)
            lo = x;
          if (x > hi)
            hi = x;
        }
      if (missing)
        s2_fail("no finite cut data");
      if (lo == hi) {
        lo -= .5;
        hi += .5;
      }
      lo -= (hi - lo) * .001;
      w = s2_value(V_NUM, m + 1);
      for (i = 0; i <= m; i++)
        w->num[i] = lo + (hi - lo) * i / m;
      breaks = w;
      num = 1;
    } else {
      breaks = s2_copy(breaks);
      for (i = 1; i < breaks->length; i++)
        for (j = i; j > 0 && breaks->num[j] < breaks->num[j - 1]; j--) {
          x = breaks->num[j];
          breaks->num[j] = breaks->num[j - 1];
          breaks->num[j - 1] = x;
        }
      m = breaks->length - 1;
      num = 0;
    }
    if (m < 1)
      s2_fail("at least two cut breaks needed");
    for (i = 0; i < m; i++)
      if (s2_missing(breaks->num[i]) || breaks->num[i] >= breaks->num[i + 1])
        s2_fail("cut breaks must be distinct");
    labels = s2_arg(args, 2, "labels", env, NULL);
    if (!labels) {
      labels = s2_value(V_STR, m);
      for (i = 0; i < m; i++) {
        char buf[100];
        if (num)
          sprintf(buf, "Range %d", i + 1);
        else
          sprintf(buf, "(%.6g,%.6g]", breaks->num[i], breaks->num[i + 1]);
        labels->str[i] = s2_strdup(buf);
      }
    }
    if (labels->type != V_STR || labels->length != m)
      s2_fail("wrong cut label count");
    r = s2_value(V_NUM, v->length);
    r->integer = 1;
    for (i = 0; i < v->length; i++) {
      r->num[i] = S2_NA;
      if (!s2_missing(v->num[i]))
        for (j = 0; j < m; j++)
          if (v->num[i] > breaks->num[j] && v->num[i] <= breaks->num[j + 1]) {
            r->num[i] = j + 1;
            break;
          }
    }
    r = s2_with_attr(r, "levels", labels);
    return s2_with_attr(r, "class", s2_string("category"));
  }
  if (strcmp(name, "table") && strcmp(name, "tapply") && strcmp(name, "split"))
    return NULL;
  ncat = 0;
  v = NULL;
  fn = NULL;
  rest = NULL;
  if (!strcmp(name, "table")) {
    for (p = args; p; p = p->next) {
      if (ncat == 16)
        s2_fail("too many table dimensions");
      cats[ncat++] = category(s2_eval(p->a, p->scope ? p->scope : env));
    }
  } else {
    v = s2_arg(args, 0, !strcmp(name, "tapply") ? "X" : "x", env, NULL);
    groups =
        s2_arg(args, 1, !strcmp(name, "tapply") ? "INDICES" : "f", env, NULL);
    if (!v || !groups)
      s2_fail("data and groups required");
    if (groups->type == V_LIST) {
      if (groups->length > 16)
        s2_fail("too many grouping dimensions");
      for (i = 0; i < groups->length; i++)
        cats[ncat++] = category(groups->items[i]);
    } else
      cats[ncat++] = category(groups);
    if (!strcmp(name, "tapply")) {
      fn = s2_arg(args, 2, "FUN", env, NULL);
      for (p = args; p; p = p->next)
        if (!p->text || (strcmp(p->text, "X") && strcmp(p->text, "INDICES") &&
                         strcmp(p->text, "FUN"))) {
          a = (Node *)s2_alloc(sizeof(Node));
          *a = *p;
          a->next = rest;
          rest = a;
        }
    }
  }
  if (!ncat)
    s2_fail("at least one grouping vector required");
  n = v ? v->length : cats[0]->length;
  dims = s2_value(V_NUM, ncat);
  dn = s2_value(V_LIST, ncat);
  ncells = 1;
  for (i = 0; i < ncat; i++) {
    if (cats[i]->length != n)
      s2_fail("grouping lengths differ");
    levels = s2_attr(cats[i], "levels");
    dims->num[i] = levels->length;
    dn->items[i] = levels;
    if ((long)ncells * levels->length > S2_MAX_VECTOR)
      s2_fail("table too large");
    ncells *= levels->length;
  }
  counts = (int *)s2_alloc((size_t)(ncells ? ncells : 1) * sizeof(int));
  index = s2_value(V_NUM, n);
  for (i = 0; i < ncells; i++)
    counts[i] = 0;
  for (i = 0; i < n; i++) {
    cell = 0;
    m = 1;
    missing = 0;
    for (j = 0; j < ncat; j++) {
      x = cats[j]->num[i];
      if (s2_missing(x)) {
        missing = 1;
        break;
      }
      cell += ((int)x - 1) * m;
      m *= (int)dims->num[j];
    }
    index->num[i] = missing ? S2_NA : cell + 1;
    if (!missing)
      counts[cell]++;
  }
  if (!strcmp(name, "table")) {
    r = s2_value(V_NUM, ncells);
    r->integer = 1;
    for (i = 0; i < ncells; i++)
      r->num[i] = counts[i];
    r = s2_with_attr(r, "dim", dims);
    return s2_with_attr(r, "dimnames", dn);
  }
  if (!strcmp(name, "tapply") && !fn)
    return index;
  r = s2_value(V_LIST, ncells);
  r->names = ncat == 1 ? dn->items[0]->str : NULL;
  for (cell = 0; cell < ncells; cell++) {
    if (!counts[cell] && !strcmp(name, "tapply")) {
      r->items[cell] = s2_number(S2_NA);
      continue;
    }
    w = s2_value(V_NUM, n);
    w->logical = 1;
    for (i = 0; i < n; i++)
      w->num[i] = index->num[i] == cell + 1;
    a = arg(w);
    subset = s2_subset(v, a, env, 0, NULL);
    if (!strcmp(name, "split"))
      r->items[cell] = subset;
    else {
      a = arg(subset);
      a->next = rest;
      r->items[cell] = s2_call(fn, a, env);
    }
  }
  if (!strcmp(name, "split"))
    return r;
  k = V_NULL;
  for (i = 0; i < ncells; i++) {
    w = r->items[i];
    if (w->length != 1 ||
        (w->type != V_NUM && w->type != V_STR && w->type != V_COMPLEX)) {
      k = V_LIST;
      break;
    }
    k = s2_common(s2_value(k, 0), w);
  }
  if (k != V_LIST && k != V_NULL) {
    w = s2_value(k, ncells);
    for (i = 0; i < ncells; i++) {
      subset = s2_coerce(r->items[i], k, 0);
      if (k == V_NUM || k == V_COMPLEX)
        w->num[i] = subset->num[0];
      if (k == V_COMPLEX)
        w->imag[i] = subset->imag[0];
      if (k == V_STR)
        w->str[i] = subset->str[0];
    }
    r = w;
  }
  r = s2_with_attr(r, "dim", dims);
  return s2_with_attr(r, "dimnames", dn);
}
