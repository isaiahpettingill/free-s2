#include "s2.h"
extern int s2_token_assign(int), s2_token_global(int), s2_token_ldb(int),
    s2_token_binary(int);
static int truth(Value *v) {
  if (v->type != V_NUM)
    v = s2_coerce(v, V_NUM, 1);
  if (v->length > 1)
    fprintf(stderr, "Warning: condition has length > 1\n");
  if (!v->length || v->num[0] == S2_NA || v->num[0] != v->num[0])
    s2_fail("invalid or missing condition");
  return v->num[0] != 0;
}
static Value *binary(Node *node, Env *env) {
  Value *a, *b, *r;
  double x, y, z;
  int op, special, n, i, missing;
  op = node->op;
  if (op == '=')
    s2_fail("= names arguments; use <- or _ for assignment");
  special = s2_token_binary(op);
  if (s2_token_assign(op)) {
    b = s2_eval(node->b, env);
    s2_assign(node->a, b, env, s2_token_global(op));
    s2_visible = 0;
    return b;
  }
  if (node->text) {
    Node *args, *second;
    Value *fn;
    args = (Node *)s2_alloc(sizeof(Node));
    second = (Node *)s2_alloc(sizeof(Node));
    args->a = node->a;
    args->next = second;
    second->a = node->b;
    fn = s2_function(env, node->text);
    if (!fn)
      s2_fail("unknown infix function");
    return s2_call(fn, args, env);
  }
  a = s2_eval(node->a, env);
  if (special == 5 || special == 6) {
    i = truth(a);
    if ((special == 5 && !i) || (special == 6 && i)) {
      r = s2_number(i);
      r->logical = 1;
      return r;
    }
    b = s2_eval(node->b, env);
    r = s2_number(truth(b));
    r->logical = 1;
    return r;
  }
  b = s2_eval(node->b, env);
  if (op == ':') {
    if (a->type != V_NUM || b->type != V_NUM || !a->length || !b->length)
      s2_fail("invalid sequence endpoints");
    x = a->num[0];
    y = b->num[0];
    if (x == S2_NA || y == S2_NA || x != x || y != y ||
        fabs(y - x) >= S2_MAX_VECTOR)
      s2_fail("sequence too long or missing");
    n = (int)fabs(y - x) + 1;
    r = s2_value(V_NUM, n);
    for (i = 0; i < n; i++)
      r->num[i] = x + (x <= y ? i : -i);
    return r;
  }
  if (a->type == V_STR && b->type == V_STR &&
      ((special >= 1 && special <= 4) || op == '<' || op == '>')) {
    n = !a->length || !b->length
            ? 0
            : (a->length > b->length ? a->length : b->length);
    r = s2_value(V_NUM, n);
    r->logical = 1;
    for (i = 0; i < n; i++) {
      if (!a->str[i % a->length] || !b->str[i % b->length]) {
        r->num[i] = S2_NA;
        continue;
      }
      z = strcmp(a->str[i % a->length], b->str[i % b->length]);
      r->num[i] = special == 1   ? z == 0
                  : special == 2 ? z != 0
                  : special == 3 ? z <= 0
                  : special == 4 ? z >= 0
                  : op == '<'    ? z < 0
                                 : z > 0;
    }
    return r;
  }
  if (special == 5 || special == 6 || op == '&' || op == '|') {
    a = s2_coerce(a, V_NUM, 1);
    b = s2_coerce(b, V_NUM, 1);
  }
  if (a->type == V_COMPLEX || b->type == V_COMPLEX)
    return s2_complex_binary(a, b, op);
  if (a->type != V_NUM || b->type != V_NUM)
    s2_fail("numeric operands required");
  if (a->rows && b->rows && (a->rows != b->rows || a->cols != b->cols))
    s2_fail("dimension attributes do not match");
  n = !a->length || !b->length
          ? 0
          : (a->length > b->length ? a->length : b->length);
  if (special == 5 || special == 6)
    n = n ? 1 : 0;
  if (n && (n % a->length || n % b->length) && special != 5 && special != 6)
    fprintf(stderr, "Warning: fractional vector recycling\n");
  r = s2_value(V_NUM, n);
  r->logical = (special >= 1 && special <= 6) || op == '<' || op == '>' ||
               op == '&' || op == '|';
  if (a->rows && a->length == n) {
    r->rows = a->rows;
    r->cols = a->cols;
  } else if (b->rows && b->length == n) {
    r->rows = b->rows;
    r->cols = b->cols;
  }
  if (n) {
    Value *longer;
    Attr *at;
    longer = a->length >= b->length ? a : b;
    r->names = longer->names;
    r->attrs = longer->attrs;
    if (a->length == b->length)
      for (at = b->attrs; at; at = at->next)
        if (!s2_attr(r, at->name)->length)
          r = s2_with_attr(r, at->name, at->value);
    if (s2_attr(r, "levels")->length) {
      r = s2_with_attr(r, "levels", s2_value(V_NULL, 0));
      r = s2_with_attr(r, "class", s2_value(V_NULL, 0));
    }
  }
  for (i = 0; i < n; i++) {
    x = a->num[i % a->length];
    y = b->num[i % b->length];
    missing = x == S2_NA || y == S2_NA || x != x || y != y;
    z = S2_NA;
    if ((op == '&' || special == 5) && (x == 0 || y == 0))
      z = 0;
    else if ((op == '|' || special == 6) && ((x != S2_NA && x == x && x != 0) ||
                                             (y != S2_NA && y == y && y != 0)))
      z = 1;
    else if (!missing) {
      switch (op) {
      case '+':
        z = x + y;
        break;
      case '-':
        z = x - y;
        break;
      case '*':
        z = x * y;
        break;
      case '/':
        z = y == 0 ? S2_NA : x / y;
        break;
      case '^':
        z = (x < 0 && floor(y) != y) ? S2_NA : pow(x, y);
        break;
      case '<':
        z = x < y;
        break;
      case '>':
        z = x > y;
        break;
      case '&':
        z = x != 0 && y != 0;
        break;
      case '|':
        z = x != 0 || y != 0;
        break;
      default:
        if (special == 1)
          z = x == y;
        else if (special == 2)
          z = x != y;
        else if (special == 3)
          z = x <= y;
        else if (special == 4)
          z = x >= y;
        else if (special == 5)
          z = x != 0 && y != 0;
        else if (special == 6)
          z = x != 0 || y != 0;
        else if (special == 7)
          z = y == 0 ? S2_NA : x - y * floor(x / y);
        else if (special == 8)
          z = y == 0 ? S2_NA : floor(x / y);
        else
          s2_fail("unknown operator");
      }
    }
    r->num[i] = z;
  }
  return r;
}
Node *s2_expand(Node *args, Env *env) {
  Node *p, *q, *head, *tail, *d;
  head = tail = NULL;
  for (p = args; p; p = p->next) {
    if (!p->text && p->a && p->a->kind == N_NAME &&
        !strcmp(p->a->text, "...")) {
      if (!env->dots && !s2_binding(env, "..."))
        s2_fail("... outside variadic function");
      for (d = env->dots; d; d = d->next) {
        q = (Node *)s2_alloc(sizeof(Node));
        *q = *d;
        q->next = NULL;
        if (tail)
          tail->next = q;
        else
          head = q;
        tail = q;
      }
    } else {
      q = (Node *)s2_alloc(sizeof(Node));
      *q = *p;
      q->scope = p->scope ? p->scope : env;
      q->next = NULL;
      if (tail)
        tail->next = q;
      else
        head = q;
      tail = q;
    }
  }
  return head;
}
void s2_transfer(int code, Value *v) {
  Control *c;
  c = code == 1 ? s2_function_control : s2_loop_control;
  if (!c)
    s2_fail(code == 1 ? "return outside function" : "break/next outside loop");
  if (code == 1)
    c->result = v;
  s2_depth = c->depth;
  longjmp(c->jump, code);
}
Value *s2_call(Value *volatile fn, Node *args, Env *caller) {
  Env *local;
  Node *formal, *arg, *formals[S2_MAX_ARGS], *actuals[S2_MAX_ARGS], *tail;
  Binding *binding;
  Control *control, *oldloop;
  int n, i, j, position, dot, limit, matched[S2_MAX_ARGS], na;
  if (fn->type == V_STR && fn->length == 1) {
    fn = s2_function(caller, fn->str[0]);
    if (!fn)
      s2_fail("function not found");
  }
  if (fn->type != V_FUNC)
    s2_fail("not a function");
  args = s2_expand(args, caller);
  if (fn->builtin)
    return s2_builtin(fn->builtin, args, caller);
  local = (Env *)s2_alloc(sizeof(Env));
  n = na = 0;
  dot = -1;
  for (formal = fn->formals; formal; formal = formal->next) {
    if (n == S2_MAX_ARGS)
      s2_fail("too many formals");
    for (i = 0; i < n; i++)
      if (!strcmp(formals[i]->text, formal->text))
        s2_fail("duplicate formal argument");
    if (!strcmp(formal->text, "..."))
      dot = n;
    formals[n] = formal;
    actuals[n++] = NULL;
  }
  limit = dot < 0 ? n : dot;
  for (arg = args; arg; arg = arg->next) {
    if (na == S2_MAX_ARGS)
      s2_fail("too many arguments");
    matched[na++] = -1;
  }
  j = 0;
  for (arg = args; arg; arg = arg->next, j++)
    if (arg->text) {
      for (i = 0; i < n; i++)
        if (i != dot && !strcmp(formals[i]->text, arg->text))
          break;
      if (i < n) {
        if (actuals[i])
          s2_fail("argument matched more than once");
        actuals[i] = arg;
        matched[j] = i;
      }
    }
  j = 0;
  for (arg = args; arg; arg = arg->next, j++)
    if (arg->text && matched[j] < 0) {
      position = -1;
      for (i = 0; i < limit; i++)
        if (!strncmp(formals[i]->text, arg->text, strlen(arg->text))) {
          if (position >= 0)
            s2_fail("ambiguous partial argument name");
          position = i;
        }
      if (position >= 0) {
        if (actuals[position])
          s2_fail("argument matched more than once");
        actuals[position] = arg;
        matched[j] = position;
      } else if (dot < 0)
        s2_fail("unknown named argument");
    }
  position = j = 0;
  for (arg = args; arg; arg = arg->next, j++)
    if (!arg->text) {
      while (position < limit && actuals[position])
        position++;
      if (position < limit) {
        actuals[position] = arg;
        matched[j] = position++;
      } else if (dot < 0)
        s2_fail("unused argument");
    }
  tail = NULL;
  j = 0;
  for (arg = args; arg; arg = arg->next, j++)
    if (matched[j] < 0) {
      Node *q;
      q = (Node *)s2_alloc(sizeof(Node));
      *q = *arg;
      q->next = NULL;
      q->a = (Node *)s2_alloc(sizeof(Node));
      q->a->kind = N_PROMISE;
      q->a->a = arg->a;
      q->a->scope = arg->scope ? arg->scope : caller;
      q->a->origin = q;
      if (tail)
        tail->next = q;
      else
        local->dots = q;
      tail = q;
    }
  local->nargs = na;
  for (i = 0; i < n; i++) {
    binding = (Binding *)s2_alloc(sizeof(Binding));
    binding->name = formals[i]->text;
    binding->formal = 1;
    binding->missing = !(actuals[i] && actuals[i]->a);
    binding->promise = binding->missing ? formals[i]->a : actuals[i]->a;
    binding->caller = binding->missing
                          ? local
                          : (actuals[i]->scope ? actuals[i]->scope : caller);
    if (i == dot)
      binding->value = s2_value(V_NULL, 0);
    binding->next = local->bindings;
    local->bindings = binding;
  }
  control = (Control *)s2_alloc(sizeof(Control));
  control->depth = s2_depth;
  control->previous = s2_function_control;
  oldloop = s2_loop_control;
  s2_loop_control = NULL;
  s2_function_control = control;
  if (!setjmp(control->jump))
    control->result = s2_eval(fn->body, local);
  s2_function_control = control->previous;
  s2_loop_control = oldloop;
  return control->result;
}
Value *s2_eval(Node *node, Env *env) {
  Value *r, *v, *last;
  Node *p;
  Binding *loop, saved, **link;
  Control *control;
  int transfer;
  int i, had_loop;
  if (++s2_depth > S2_MAX_DEPTH)
    s2_fail("evaluation depth limit");
  s2_visible = 1;
  r = NULL;
  if (!node)
    r = s2_value(V_NULL, 0);
  else
    switch (node->kind) {
    case N_PROMISE:
      if (node->origin->value)
        r = node->origin->value;
      else {
        if (!node->a)
          s2_fail("argument is missing");
        r = s2_eval(node->a, node->scope);
        node->origin->value = r;
      }
      break;
    case N_VALUE:
      r = node->value;
      break;
    case N_NUM:
      r = s2_number(node->number);
      break;
    case N_STR:
      r = s2_value(V_STR, 1);
      r->str[0] = node->text;
      break;
    case N_NAME:
      if (!strcmp(node->text, "NULL"))
        r = s2_value(V_NULL, 0);
      else if (!strcmp(node->text, "NA"))
        r = s2_number(S2_NA);
      else if (!strcmp(node->text, "TRUE") || !strcmp(node->text, "FALSE")) {
        r = s2_number(!strcmp(node->text, "TRUE"));
        r->logical = 1;
      } else if (!strcmp(node->text, "break") || !strcmp(node->text, "next")) {
        s2_transfer(!strcmp(node->text, "break") ? 2 : 3, NULL);
        r = s2_value(V_NULL, 0);
      } else {
        r = s2_lookup(env, node->text);
        if (!r && (!strcmp(node->text, "T") || !strcmp(node->text, "F"))) {
          r = s2_number(!strcmp(node->text, "T"));
          r->logical = 1;
        }
        if (!r && !strcmp(node->text, "pi"))
          r = s2_number(3.14159265358979323846);
        if (!r)
          s2_fail("object not found");
      }
      break;
    case N_BIN:
      r = binary(node, env);
      break;
    case N_UNARY:
      v = s2_eval(node->a, env);
      if (v->type != V_NUM)
        s2_fail("numeric operand required");
      r = s2_copy(v);
      r->logical = node->op == '!';
      for (i = 0; i < r->length; i++)
        if (r->num[i] != S2_NA) {
          if (node->op == '-')
            r->num[i] = -r->num[i];
          if (node->op == '!')
            r->num[i] = r->num[i] == 0;
        }
      break;
    case N_BLOCK:
      r = s2_value(V_NULL, 0);
      for (p = node->a; p; p = p->next) {
        r = s2_eval(p, env);
        if (s2_flow)
          break;
      }
      break;
    case N_IF:
      r = s2_eval(truth(s2_eval(node->a, env)) ? node->b : node->c, env);
      break;
    case N_REPEAT:
    case N_WHILE:
      control = (Control *)s2_alloc(sizeof(Control));
      control->previous = s2_loop_control;
      control->depth = s2_depth;
      control->result = s2_value(V_NULL, 0);
      s2_loop_control = control;
      for (;;) {
        transfer = setjmp(control->jump);
        if (transfer == 2)
          break;
        if (node->kind == N_WHILE && !truth(s2_eval(node->a, env)))
          break;
        last = s2_eval(node->b, env);
        control->result = last;
      }
      r = control->result;
      s2_loop_control = control->previous;
      s2_visible = 0;
      break;
    case N_FOR:
      v = s2_eval(node->a, env);
      loop = s2_binding(env, node->text);
      had_loop = loop != NULL;
      if (had_loop)
        saved = *loop;
      control = (Control *)s2_alloc(sizeof(Control));
      control->previous = s2_loop_control;
      control->depth = s2_depth;
      control->result = s2_value(V_NULL, 0);
      s2_loop_control = control;
      while (control->index < v->length) {
        transfer = setjmp(control->jump);
        if (transfer == 2)
          break;
        if (transfer == 3) {
          control->index++;
          continue;
        }
        s2_set(env, node->text, s2_element(v, control->index));
        control->result = s2_eval(node->b, env);
        control->index++;
      }
      if (had_loop)
        *loop = saved;
      else if (env == s2_expression || env == &s2_global) {
        link = &env->bindings;
        while (*link && strcmp((*link)->name, node->text))
          link = &(*link)->next;
        if (*link)
          *link = (*link)->next;
      }
      r = control->result;
      s2_loop_control = control->previous;
      s2_visible = 0;
      break;
    case N_FUNC:
      r = s2_value(V_FUNC, 1);
      r->formals = node->a;
      r->body = node->b;
      break;
    case N_CALL:
      if ((node->a->kind == N_NAME || node->a->kind == N_STR)) {
        v = s2_function(env, node->a->text);
        r = v ? s2_call(v, node->b, env)
              : s2_builtin(node->a->text, s2_expand(node->b, env), env);
      } else
        r = s2_call(s2_eval(node->a, env), node->b, env);
      break;
    case N_INDEX:
      r = s2_subset(s2_eval(node->a, env), node->b, env, s2_token_ldb(node->op),
                    NULL);
      break;
    case N_MEMBER:
      v = s2_eval(node->a, env);
      if (v->type != V_LIST) {
        r = s2_value(V_NULL, 0);
        break;
      }
      {
        int match, ambiguous;
        match = -1;
        ambiguous = 0;
        for (i = 0; i < v->length; i++)
          if (v->names && v->names[i] && !strcmp(v->names[i], node->text)) {
            match = i;
            break;
          }
        if (match < 0)
          for (i = 0; i < v->length; i++)
            if (v->names && v->names[i] &&
                !strncmp(v->names[i], node->text, strlen(node->text))) {
              match = i;
              ambiguous++;
            }
        r = match < 0 || ambiguous > 1 ? s2_value(V_NULL, 0) : v->items[match];
      }
      break;
    default:
      s2_fail("unknown expression");
    }
  s2_depth--;
  return r;
}
