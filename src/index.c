#include "s2.h"
static int index_vector(Value *v, Value *idx, int length, char **names,
                        int replace, int *out) {
  int n, i, j, k, positive, negative, unknown, maxlen, matches;
  double x;
  (void)v;
  n = positive = negative = unknown = 0;
  if (!idx) {
    for (i = 0; i < length; i++)
      out[n++] = i;
    return n;
  }
  if (idx->type == V_STR) {
    for (i = 0; i < idx->length; i++) {
      k = -1;
      matches = 0;
      if (idx->str[i]) {
        for (j = 0; j < length; j++)
          if (names && names[j] && !strcmp(names[j], idx->str[i])) {
            k = j;
            break;
          }
        if (k < 0 && !replace)
          for (j = 0; j < length; j++)
            if (names && names[j] &&
                !strncmp(names[j], idx->str[i], strlen(idx->str[i]))) {
              k = j;
              matches++;
            }
        if (matches > 1)
          k = -1;
        if (k < 0 && replace)
          for (j = 0; j < i; j++)
            if (idx->str[j] && !strcmp(idx->str[j], idx->str[i])) {
              k = out[j];
              break;
            }
      }
      if (k < 0)
        k = length + unknown++;
      if (k >= S2_MAX_VECTOR)
        s2_fail("subscript exceeds vector limit");
      out[n++] = k;
    }
    return n;
  }
  if (idx->type != V_NUM && idx->type != V_NULL)
    s2_fail("invalid subscript mode");
  if (!idx->length)
    return 0;
  if (idx->logical) {
    maxlen = length > idx->length ? length : idx->length;
    for (i = 0; i < maxlen; i++) {
      x = idx->num[i % idx->length];
      if (s2_missing(x))
        out[n++] = -1;
      else if (x != 0)
        out[n++] = i;
    }
    return n;
  }
  for (i = 0; i < idx->length; i++) {
    x = idx->num[i];
    if (s2_missing(x)) {
      positive = 1;
      continue;
    }
    if (fabs(x) > S2_MAX_VECTOR)
      s2_fail("subscript exceeds vector limit");
    if ((int)x > 0)
      positive = 1;
    if ((int)x < 0)
      negative = 1;
  }
  if (positive && negative)
    s2_fail("only zeros with negative subscripts");
  if (negative) {
    for (i = 0; i < length; i++) {
      for (j = 0; j < idx->length; j++)
        if ((int)idx->num[j] == -(i + 1))
          break;
      if (j == idx->length)
        out[n++] = i;
    }
  } else
    for (i = 0; i < idx->length; i++) {
      x = idx->num[i];
      if (s2_missing(x))
        out[n++] = -1;
      else if ((int)x)
        out[n++] = (int)x - 1;
    }
  return n;
}
static void put(Value *r, int i, Value *v, int j) {
  if (r->type == V_NUM || r->type == V_COMPLEX)
    r->num[i] = j >= 0 && j < v->length ? v->num[j] : S2_NA;
  if (r->type == V_COMPLEX)
    r->imag[i] = j >= 0 && j < v->length ? v->imag[j] : S2_NA;
  if (r->type == V_STR)
    r->str[i] = j >= 0 && j < v->length ? v->str[j] : NULL;
  if (r->type == V_LIST || r->type == V_EXPR)
    r->items[i] = j >= 0 && j < v->length ? v->items[j] : s2_value(V_NULL, 0);
}
Value *s2_subset(Value *v, Node *args, Env *env, int single, Value *rhs) {
  int *pos, *ix[16], ns[16], nd, n, i, j, k, len, rank, dims[16], drop, missing;
  long stride;
  Value *idx, *dim, *dn, *r, *w, *outdim, *outdn;
  Node *a;
  pos = (int *)s2_alloc(S2_MAX_VECTOR * sizeof(int));
  dim = s2_attr(v, "dim");
  dn = s2_attr(v, "dimnames");
  nd = 0;
  drop = 1;
  idx = NULL;
  for (a = args; a; a = a->next)
    if (a->text && !strcmp(a->text, "drop"))
      drop = s2_scalar(s2_eval(a->a, env)) != 0;
    else
      nd++;
  if (nd > 16)
    s2_fail("too many subscripts");
  if (nd > 1) {
    if (dim->type != V_NUM || dim->length != nd)
      s2_fail("incorrect number of dimensions");
    n = 1;
    i = 0;
    for (a = args; a; a = a->next)
      if (!a->text) {
        idx = a->a ? s2_eval(a->a, a->scope ? a->scope : env) : NULL;
        ix[i] = (int *)s2_alloc(S2_MAX_VECTOR * sizeof(int));
        dims[i] = (int)dim->num[i];
        w = dn->type == V_LIST && i < dn->length ? dn->items[i] : NULL;
        ns[i] =
            index_vector(v, idx, dims[i], w && w->type == V_STR ? w->str : NULL,
                         rhs != NULL, ix[i]);
        if ((long)n * ns[i] > S2_MAX_VECTOR)
          s2_fail("subset too large");
        n *= ns[i];
        i++;
      }
    for (k = 0; k < n; k++) {
      j = k;
      stride = 1;
      pos[k] = 0;
      missing = 0;
      for (i = 0; i < nd; i++) {
        len = ix[i][j % ns[i]];
        j /= ns[i];
        if (len < 0 || len >= dims[i])
          missing = 1;
        else
          pos[k] += (int)(stride * len);
        stride *= dims[i];
      }
      if (missing)
        pos[k] = -1;
    }
  } else {
    a = args;
    idx = a && a->a ? s2_eval(a->a, a->scope ? a->scope : env) : NULL;
    if (idx && idx->rows && dim->type == V_NUM && idx->cols == dim->length &&
        !single) {
      n = idx->rows;
      for (i = 0; i < n; i++) {
        pos[i] = 0;
        stride = 1;
        missing = 0;
        for (j = 0; j < dim->length; j++) {
          double x;
          x = idx->num[j * idx->rows + i];
          if (s2_missing(x) || x < 1 || x > dim->num[j])
            missing = 1;
          else
            pos[i] += (int)(stride * ((int)x - 1));
          stride *= (int)dim->num[j];
        }
        if (missing)
          pos[i] = -1;
      }
    } else
      n = index_vector(v, idx, v->length, v->names, rhs != NULL, pos);
  }
  if (single && n != 1)
    s2_fail("[[ requires one element");
  if (rhs) {
    if (!rhs->length && !(single && v->type == V_LIST))
      return s2_copy(v);
    len = v->length;
    for (i = 0; i < n; i++)
      if (pos[i] >= len)
        len = pos[i] + 1;
    if (len > S2_MAX_VECTOR)
      s2_fail("replacement exceeds vector limit");
    if (single && v->type == V_LIST) {
      w = v;
      r = s2_value(V_LIST, len);
    } else {
      j = s2_common(v, rhs);
      w = s2_coerce(v, j, v->logical && rhs->logical);
      rhs = s2_coerce(rhs, j, v->logical && rhs->logical);
      r = s2_value(j, len);
      r->logical = w->logical;
    }
    for (i = 0; i < len; i++)
      put(r, i, w, i);
    for (i = 0; i < n; i++)
      if (pos[i] >= 0) {
        if (single && r->type == V_LIST)
          r->items[pos[i]] = rhs;
        else
          put(r, pos[i], rhs, i % rhs->length);
      }
    r->attrs = v->attrs;
    r->rows = v->rows;
    r->cols = v->cols;
    if (v->names || (idx && idx->type == V_STR)) {
      r->names = (char **)s2_alloc((size_t)(len ? len : 1) * sizeof(char *));
      for (i = 0; i < v->length; i++)
        if (v->names)
          r->names[i] = v->names[i];
      if (idx && idx->type == V_STR)
        for (i = 0; i < n; i++)
          if (pos[i] >= 0)
            r->names[pos[i]] = idx->str[i];
    }
    return r;
  }
  if (single)
    return s2_element(v, pos[0]);
  r = s2_value(v->type, n);
  r->logical = v->logical;
  r->integer = v->integer;
  for (i = 0; i < n; i++)
    put(r, i, v, pos[i]);
  if (v->names) {
    r->names = (char **)s2_alloc((size_t)(n ? n : 1) * sizeof(char *));
    for (i = 0; i < n; i++)
      r->names[i] = pos[i] >= 0 && pos[i] < v->length ? v->names[pos[i]] : NULL;
  }
  if (nd > 1) {
    rank = 0;
    for (i = 0; i < nd; i++)
      if (!drop || ns[i] != 1)
        rank++;
    if (rank > 1 || !drop) {
      outdim = s2_value(V_NUM, rank);
      outdn = s2_value(V_LIST, rank);
      k = 0;
      for (i = 0; i < nd; i++)
        if (!drop || ns[i] != 1) {
          outdim->num[k] = ns[i];
          w = dn->type == V_LIST && i < dn->length ? dn->items[i] : NULL;
          if (w && w->type == V_STR) {
            outdn->items[k] = s2_value(V_STR, ns[i]);
            for (j = 0; j < ns[i]; j++)
              outdn->items[k]->str[j] = ix[i][j] >= 0 && ix[i][j] < w->length
                                            ? w->str[ix[i][j]]
                                            : NULL;
          } else
            outdn->items[k] = s2_value(V_NULL, 0);
          k++;
        }
      r = s2_with_attr(r, "dim", outdim);
      if (dn->type == V_LIST)
        r = s2_with_attr(r, "dimnames", outdn);
    }
  }
  return r;
}
void s2_assign(Node *target, Value *rhs, Env *env, int global) {
  Value *v, *r, *a;
  Node index, arg, *p, *callargs, *last;
  char buf[260];
  if ((target->kind == N_NAME || target->kind == N_STR)) {
    s2_set(global ? &s2_global : env, target->text, rhs);
    return;
  }
  if (target->kind == N_INDEX) {
    v = s2_eval(target->a, env);
    r = s2_subset(v, target->b, env, s2_token_ldb(target->op), rhs);
    s2_assign(target->a, r, env, global);
    return;
  }
  if (target->kind == N_MEMBER) {
    v = s2_eval(target->a, env);
    if (v->type != V_LIST)
      v = s2_coerce(v, V_LIST, 0);
    memset(&index, 0, sizeof(index));
    memset(&arg, 0, sizeof(arg));
    index.kind = N_VALUE;
    index.value = s2_string(target->text);
    arg.a = &index;
    r = s2_subset(v, &arg, env, 1, rhs);
    s2_assign(target->a, r, env, global);
    return;
  }
  if (target->kind == N_CALL && target->b && target->a->text) {
    p = target->b;
    v = s2_eval(p->a, env);
    if (!strcmp(target->a->text, "names") || !strcmp(target->a->text, "dim") ||
        !strcmp(target->a->text, "dimnames") ||
        !strcmp(target->a->text, "class") || !strcmp(target->a->text, "tsp")) {
      r = s2_with_attr(v, target->a->text, rhs);
      s2_assign(p->a, r, env, global);
      return;
    }
    if (!strcmp(target->a->text, "attr")) {
      if (!p->next)
        s2_fail("attr needs name");
      a = s2_eval(p->next->a, env);
      if (a->type != V_STR || a->length != 1)
        s2_fail("attribute name required");
      r = s2_with_attr(v, a->str[0], rhs);
      s2_assign(p->a, r, env, global);
      return;
    }
    if (!strcmp(target->a->text, "length")) {
      int i, n;
      n = s2_count(rhs);
      r = s2_value(v->type, n);
      r->logical = v->logical;
      for (i = 0; i < n; i++)
        put(r, i, v, i);
      s2_assign(p->a, r, env, global);
      return;
    }
    if (!strcmp(target->a->text, "mode") ||
        !strcmp(target->a->text, "storage.mode")) {
      if (rhs->type != V_STR || rhs->length != 1)
        s2_fail("mode string required");
      r = s2_coerce(v,
                    !strcmp(rhs->str[0], "character") ? V_STR
                    : !strcmp(rhs->str[0], "complex") ? V_COMPLEX
                    : !strcmp(rhs->str[0], "list")    ? V_LIST
                                                      : V_NUM,
                    !strcmp(rhs->str[0], "logical"));
      r->integer = !strcmp(rhs->str[0], "integer");
      s2_assign(p->a, r, env, global);
      return;
    }
    if (strlen(target->a->text) > 250)
      s2_fail("replacement function name too long");
    sprintf(buf, "%s<-", target->a->text);
    a = s2_function(env, buf);
    if (!a)
      s2_fail("replacement function not found");
    callargs = last = NULL;
    for (p = target->b; p; p = p->next) {
      Node *q;
      q = (Node *)s2_alloc(sizeof(Node));
      *q = *p;
      q->next = NULL;
      if (last)
        last->next = q;
      else
        callargs = q;
      last = q;
    }
    p = (Node *)s2_alloc(sizeof(Node));
    p->text = s2_strdup("value");
    p->a = (Node *)s2_alloc(sizeof(Node));
    p->a->kind = N_VALUE;
    p->a->value = rhs;
    last->next = p;
    r = s2_call(a, callargs, env);
    s2_assign(target->b->a, r, env, global);
    return;
  }
  s2_fail("invalid assignment target");
}
