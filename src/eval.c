#include "s2.h"
extern int s2_token_assign(int), s2_token_global(int), s2_token_ldb(int), s2_token_binary(int);
static int truth(Value *v)
{
    if (v->type != V_NUM || !v->length || v->num[0] == S2_NA || v->num[0] != v->num[0])
        s2_fail("invalid or missing condition");
    return v->num[0] != 0;
}
static Value *element(Value *v, int i)
{
    Value *r;
    if (v->type == V_LIST) return v->items[i];
    r = s2_value(v->type, 1); r->logical = v->logical;
    if (v->type == V_NUM) r->num[0] = v->num[i];
    else if (v->type == V_STR) r->str[0] = v->str[i];
    else s2_fail("not a vector");
    return r;
}
/* Index expansion is independent of storage; negative indices omit. */
static int indexes(Value *idx, int length, int *out)
{
    int i, j, n, pos, neg;
    double d;
    n = 0; pos = neg = 0;
    if (!idx) { for (i = 0; i < length; i++) out[n++] = i; return n; }
    if (idx->type != V_NUM) s2_fail("only numeric/logical subscripts supported");
    if (!idx->length) return 0;
    if (idx->logical) {
        for (i = 0; i < length; i++) {
            d = idx->num[i % idx->length];
            if (d == S2_NA) s2_fail("missing subscript not yet supported");
            if (d != 0) out[n++] = i;
        }
        return n;
    }
    for (i = 0; i < idx->length; i++) {
        d = idx->num[i];
        if (d == S2_NA || d != d || fabs(d) > S2_MAX_VECTOR) s2_fail("invalid subscript");
        if ((int)d > 0) pos = 1;
        if ((int)d < 0) neg = 1;
    }
    if (pos && neg) s2_fail("cannot mix positive and negative subscripts");
    if (neg) {
        for (i = 0; i < length; i++) {
            for (j = 0; j < idx->length; j++) if ((int)idx->num[j] == -(i+1)) break;
            if (j == idx->length) out[n++] = i;
        }
    } else {
        for (i = 0; i < idx->length; i++) {
            j = (int)idx->num[i]; if (!j) continue;
            if (j > length) s2_fail("subscript out of bounds (provisional policy)");
            out[n++] = j-1;
        }
    }
    return n;
}
static Value *subset(Node *node, Env *env, Value *replacement)
{
    Value *v, *r, *idx, *idx2;
    int *ix, *iy, nx, ny, n, i, j, k, *positions;
    Node *arg;
    v = s2_eval(node->a, env); arg = node->b;
    ix = (int *)s2_alloc(S2_MAX_VECTOR * sizeof(int));
    iy = (int *)s2_alloc(S2_MAX_VECTOR * sizeof(int));
    positions = (int *)s2_alloc(S2_MAX_VECTOR * sizeof(int));
    idx = arg && arg->a ? s2_eval(arg->a, env) : NULL;
    if (arg && arg->next) {
        if (!v->rows || arg->next->next) s2_fail("invalid matrix subscript");
        idx2 = arg->next->a ? s2_eval(arg->next->a, env) : NULL;
        nx = indexes(idx, v->rows, ix); ny = indexes(idx2, v->cols, iy);
        if ((long)nx * ny > S2_MAX_VECTOR) s2_fail("subscript result too large");
        n = 0; for (j = 0; j < ny; j++) for (i = 0; i < nx; i++) positions[n++] = iy[j]*v->rows+ix[i];
    } else {
        nx = ny = 0; n = indexes(idx, v->length, positions);
    }
    if (s2_token_ldb(node->op) && n != 1) s2_fail("[[ requires one element");
    if (replacement) {
        if (node->a->kind != N_NAME) s2_fail("nested subassignment not yet supported");
        if (replacement->type != v->type || (n && !replacement->length)) s2_fail("incompatible replacement");
        r = s2_copy(v);
        for (i = 0; i < n; i++) {
            k = i % replacement->length; j = positions[i];
            if (v->type == V_NUM) r->num[j] = replacement->num[k];
            else if (v->type == V_STR) r->str[j] = replacement->str[k];
            else if (v->type == V_LIST) r->items[j] = replacement->items[k];
            else s2_fail("invalid replacement type");
        }
        s2_set(env, node->a->text, r); return replacement;
    }
    if (s2_token_ldb(node->op)) return element(v, positions[0]);
    r = s2_value(v->type, n); r->logical = v->logical;
    for (i = 0; i < n; i++) {
        j = positions[i];
        if (v->type == V_NUM) r->num[i] = v->num[j];
        else if (v->type == V_STR) r->str[i] = v->str[j];
        else if (v->type == V_LIST) r->items[i] = v->items[j];
        else s2_fail("not a vector");
    }
    if (nx > 1 && ny > 1) { r->rows = nx; r->cols = ny; }
    return r;
}
static Value *binary(Node *node, Env *env)
{
    Value *a, *b, *r;
    double x, y, z;
    int op, special, n, i, missing;
    op = node->op;
    if (op == '=') s2_fail("= names arguments; use <- or _ for assignment");
    special = s2_token_binary(op);
    if (s2_token_assign(op)) {
        b = s2_eval(node->b, env);
        if (node->a->kind == N_NAME) s2_set(s2_token_global(op) ? &s2_global : env, node->a->text, b);
        else if (node->a->kind == N_INDEX && !s2_token_global(op)) subset(node->a, env, b);
        else s2_fail("unsupported assignment target");
        s2_visible = 0; return b;
    }
    a = s2_eval(node->a, env);
    if (special == 5 && a->type == V_NUM && a->length && a->num[0] == 0) { r = s2_number(0); r->logical = 1; return r; }
    if (special == 6 && a->type == V_NUM && a->length && a->num[0] != S2_NA && a->num[0] != 0) { r = s2_number(1); r->logical = 1; return r; }
    b = s2_eval(node->b, env);
    if (op == ':') {
        if (a->type != V_NUM || b->type != V_NUM || !a->length || !b->length) s2_fail("invalid sequence endpoints");
        x = a->num[0]; y = b->num[0];
        if (x == S2_NA || y == S2_NA || x != x || y != y || fabs(y-x) >= S2_MAX_VECTOR) s2_fail("sequence too long or missing");
        n = (int)fabs(y-x)+1; r = s2_value(V_NUM, n);
        for (i = 0; i < n; i++) r->num[i] = x + (x <= y ? i : -i);
        return r;
    }
    if (a->type == V_STR && b->type == V_STR && ((special >= 1 && special <= 4) || op == '<' || op == '>')) {
        n = !a->length || !b->length ? 0 : (a->length > b->length ? a->length : b->length);
        r = s2_value(V_NUM, n); r->logical = 1;
        for (i = 0; i < n; i++) {
            if (!a->str[i%a->length] || !b->str[i%b->length]) { r->num[i] = S2_NA; continue; }
            z = strcmp(a->str[i%a->length], b->str[i%b->length]);
            r->num[i] = special == 1 ? z == 0 : special == 2 ? z != 0 : special == 3 ? z <= 0 : special == 4 ? z >= 0 : op == '<' ? z < 0 : z > 0;
        }
        return r;
    }
    if (a->type != V_NUM || b->type != V_NUM) s2_fail("numeric operands required");
    n = !a->length || !b->length ? 0 : (a->length > b->length ? a->length : b->length);
    if (special == 5 || special == 6) n = n ? 1 : 0;
    if (n && (n % a->length || n % b->length) && special != 5 && special != 6)
        fprintf(stderr, "Warning: fractional vector recycling\n");
    r = s2_value(V_NUM, n);
    r->logical = (special >= 1 && special <= 6) || op == '<' || op == '>' || op == '&' || op == '|';
    if (a->rows && a->length == n) { r->rows = a->rows; r->cols = a->cols; }
    else if (b->rows && b->length == n) { r->rows = b->rows; r->cols = b->cols; }
    for (i = 0; i < n; i++) {
        x = a->num[i % a->length]; y = b->num[i % b->length];
        missing = x == S2_NA || y == S2_NA || x != x || y != y;
        z = S2_NA;
        if ((op == '&' || special == 5) && (x == 0 || y == 0)) z = 0;
        else if ((op == '|' || special == 6) && ((x != S2_NA && x == x && x != 0) || (y != S2_NA && y == y && y != 0))) z = 1;
        else if (!missing) {
            switch (op) {
            case '+': z = x+y; break; case '-': z = x-y; break;
            case '*': z = x*y; break; case '/': z = y == 0 ? S2_NA : x/y; break;
            case '^': z = (x < 0 && floor(y) != y) ? S2_NA : pow(x,y); break;
            case '<': z = x < y; break; case '>': z = x > y; break;
            case '&': z = x != 0 && y != 0; break; case '|': z = x != 0 || y != 0; break;
            default:
                if (special == 1) z = x == y; else if (special == 2) z = x != y;
                else if (special == 3) z = x <= y; else if (special == 4) z = x >= y;
                else if (special == 5) z = x != 0 && y != 0; else if (special == 6) z = x != 0 || y != 0;
                else if (special == 7) z = y == 0 ? S2_NA : x-y*floor(x/y);
                else if (special == 8) z = y == 0 ? S2_NA : floor(x/y);
                else s2_fail("unknown operator");
            }
        }
        r->num[i] = z;
    }
    return r;
}
static Value *call(Value *fn, Node *args, Env *caller)
{
    Env *local;
    Node *formal, *arg, *formals[S2_MAX_ARGS], *actuals[S2_MAX_ARGS];
    Binding *binding;
    int n, i, j, position, saved_flow;
    Value *result;
    if (fn->type != V_FUNC) s2_fail("not a function");
    local = (Env *)s2_alloc(sizeof(Env)); n = 0;
    for (formal = fn->formals; formal; formal = formal->next) {
        if (n == S2_MAX_ARGS) s2_fail("too many formal arguments");
        if (!strcmp(formal->text, "...")) s2_fail("... not yet supported");
        formals[n] = formal; actuals[n++] = NULL;
    }
    for (arg = args; arg; arg = arg->next) if (arg->text) {
        for (i = 0; i < n; i++) if (!strcmp(formals[i]->text, arg->text)) break;
        if (i == n) continue;
        if (actuals[i]) s2_fail("argument matched more than once");
        actuals[i] = arg;
    }
    /* Exact names have priority over all partial matches. */
    for (arg = args; arg; arg = arg->next) if (arg->text) {
        for (i = 0; i < n; i++) if (!strcmp(formals[i]->text,arg->text)) break;
        if (i < n) continue;
        position = -1;
        for (i = 0; i < n; i++) if (!strncmp(formals[i]->text,arg->text,strlen(arg->text))) {
            if (position >= 0) s2_fail("ambiguous partial argument name");
            position = i;
        }
        if (position < 0) s2_fail("unknown named argument");
        if (actuals[position]) s2_fail("argument matched more than once");
        actuals[position] = arg;
    }
    position = 0;
    for (arg = args; arg; arg = arg->next) if (!arg->text) {
        while (position < n && actuals[position]) position++;
        if (position == n) s2_fail("unused argument");
        actuals[position++] = arg;
    }
    for (i = 0; i < n; i++) {
        for (j = 0; j < i; j++) if (!strcmp(formals[j]->text, formals[i]->text)) s2_fail("duplicate formal argument");
        binding = (Binding *)s2_alloc(sizeof(Binding)); binding->name = formals[i]->text;
        binding->promise = actuals[i] && actuals[i]->a ? actuals[i]->a : formals[i]->a;
        binding->caller = actuals[i] && actuals[i]->a ? caller : local;
        binding->next = local->bindings; local->bindings = binding;
    }
    saved_flow = s2_flow; s2_flow = 0; result = s2_eval(fn->body, local);
    if (s2_flow == 1) result = s2_return;
    else if (s2_flow) s2_fail("break/next outside loop");
    s2_flow = saved_flow; return result;
}
Value *s2_eval(Node *node, Env *env)
{
    Value *r, *v, *last;
    Node *p;
    Binding *loop, saved, **link;
    int i, had_loop;
    if (++s2_depth > S2_MAX_DEPTH) s2_fail("evaluation depth limit");
    s2_visible = 1; r = NULL;
    if (!node) r = s2_value(V_NULL, 0);
    else switch (node->kind) {
    case N_NUM: r = s2_number(node->number); break;
    case N_STR: r = s2_value(V_STR, 1); r->str[0] = node->text; break;
    case N_NAME:
        if (!strcmp(node->text, "NULL")) r = s2_value(V_NULL, 0);
        else if (!strcmp(node->text, "NA")) r = s2_number(S2_NA);
        else if (!strcmp(node->text, "TRUE") || !strcmp(node->text, "FALSE")) {
            r = s2_number(!strcmp(node->text, "TRUE")); r->logical = 1;
        } else if (!strcmp(node->text, "break") || !strcmp(node->text, "next")) {
            s2_flow = !strcmp(node->text, "break") ? 2 : 3; r = s2_value(V_NULL,0);
        } else {
            r = s2_lookup(env, node->text);
            if (!r && (!strcmp(node->text, "T") || !strcmp(node->text, "F"))) { r = s2_number(!strcmp(node->text,"T")); r->logical = 1; }
            if (!r && !strcmp(node->text, "pi")) r = s2_number(3.14159265358979323846);
            if (!r) s2_fail("object not found");
        }
        break;
    case N_BIN: r = binary(node, env); break;
    case N_UNARY:
        v = s2_eval(node->a, env); if (v->type != V_NUM) s2_fail("numeric operand required");
        r = s2_copy(v); r->logical = node->op == '!';
        for (i = 0; i < r->length; i++) if (r->num[i] != S2_NA) {
            if (node->op == '-') r->num[i] = -r->num[i];
            if (node->op == '!') r->num[i] = r->num[i] == 0;
        }
        break;
    case N_BLOCK:
        r = s2_value(V_NULL,0);
        for (p = node->a; p; p = p->next) { r = s2_eval(p,env); if (s2_flow) break; }
        break;
    case N_IF: r = s2_eval(truth(s2_eval(node->a,env)) ? node->b : node->c, env); break;
    case N_WHILE:
        r = s2_value(V_NULL,0);
        while (truth(s2_eval(node->a,env))) {
            last = s2_eval(node->b,env); if (!s2_flow) r = last;
            if (s2_flow == 1) break;
            if (s2_flow == 2) { s2_flow = 0; break; } s2_flow = 0;
        }
        s2_visible = 0; break;
    case N_FOR:
        v = s2_eval(node->a,env); r = s2_value(V_NULL,0);
        loop = env->bindings;
        while (loop && strcmp(loop->name,node->text)) loop = loop->next;
        had_loop = loop != NULL;
        if (had_loop) saved = *loop;
        for (i = 0; i < v->length; i++) {
            s2_set(env,node->text,element(v,i));
            last = s2_eval(node->b,env); if (!s2_flow) r = last;
            if (s2_flow == 1) break;
            if (s2_flow == 2) { s2_flow = 0; break; } s2_flow = 0;
        }
        if (had_loop) *loop = saved;
        else if (env == &s2_global) {
            link = &env->bindings;
            while (*link && strcmp((*link)->name,node->text)) link = &(*link)->next;
            if (*link) *link = (*link)->next;
        }
        s2_visible = 0; break;
    case N_FUNC: r = s2_value(V_FUNC,1); r->formals = node->a; r->body = node->b; break;
    case N_CALL:
        if (node->a->kind == N_NAME) {
            v = s2_lookup(env,node->a->text);
            if (v && v->type != V_FUNC) {
                v = env == &s2_global ? NULL : s2_lookup(&s2_global,node->a->text);
                if (v && v->type != V_FUNC) v = NULL;
            }
            r = v ? call(v,node->b,env) : s2_builtin(node->a->text,node->b,env);
        } else r = call(s2_eval(node->a,env),node->b,env);
        break;
    case N_INDEX: r = subset(node,env,NULL); break;
    case N_MEMBER:
        v = s2_eval(node->a,env); if (v->type != V_LIST) s2_fail("$ requires list");
        for (i = 0; i < v->length; i++) if (v->names && v->names[i] && !strcmp(v->names[i],node->text)) break;
        r = i == v->length ? s2_value(V_NULL,0) : v->items[i]; break;
    default: s2_fail("unknown expression");
    }
    s2_depth--; return r;
}
