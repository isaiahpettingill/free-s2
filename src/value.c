#include "s2.h"
Env s2_global;
int s2_depth, s2_flow, s2_visible;
Value *s2_return;
Value *s2_value(int type, int length)
{
    Value *v;
    if (length < 0 || length > S2_MAX_VECTOR) s2_fail("vector length limit (2048)");
    v = (Value *)s2_alloc(sizeof(Value)); v->type = type; v->length = length;
    if (length && type == V_NUM) v->num = (double *)s2_alloc((size_t)length * sizeof(double));
    if (length && type == V_STR) v->str = (char **)s2_alloc((size_t)length * sizeof(char *));
    if (length && type == V_LIST) v->items = (Value **)s2_alloc((size_t)length * sizeof(Value *));
    return v;
}
Value *s2_number(double x)
{
    Value *v; v = s2_value(V_NUM, 1); v->num[0] = x; return v;
}
Value *s2_copy(Value *v)
{
    Value *r; int i;
    r = s2_value(v->type, v->length); r->logical = v->logical;
    r->rows = v->rows; r->cols = v->cols; r->names = v->names;
    r->formals = v->formals; r->body = v->body;
    for (i = 0; i < v->length; i++) {
        if (v->type == V_NUM) r->num[i] = v->num[i];
        if (v->type == V_STR) r->str[i] = v->str[i];
        if (v->type == V_LIST) r->items[i] = v->items[i];
    }
    return r;
}
static Binding *find(Env *env, const char *name)
{
    Binding *b; for (b = env->bindings; b; b = b->next) if (!strcmp(b->name, name)) return b;
    return NULL;
}
void s2_set(Env *env, const char *name, Value *v)
{
    Binding *b;
    if (!strcmp(name,"T") || !strcmp(name,"F") || !strcmp(name,"TRUE") ||
        !strcmp(name,"FALSE") || !strcmp(name,"NA") || !strcmp(name,"NULL") ||
        !strcmp(name,"if") || !strcmp(name,"else") || !strcmp(name,"for") ||
        !strcmp(name,"while") || !strcmp(name,"repeat") || !strcmp(name,"next") ||
        !strcmp(name,"break") || !strcmp(name,"in") || !strcmp(name,"function") ||
        !strcmp(name,"return")) s2_fail("assignment to reserved name");
    b = find(env, name);
    if (!b) { b = (Binding *)s2_alloc(sizeof(Binding)); b->name = s2_strdup(name);
              b->next = env->bindings; env->bindings = b; }
    b->value = v; b->promise = NULL; b->forcing = 0;
}
Value *s2_lookup(Env *env, const char *name)
{
    Binding *b;
    b = find(env, name); if (!b && env != &s2_global) b = find(&s2_global, name);
    if (!b) return NULL;
    if (!b->value) {
        if (!b->promise) s2_fail("argument is missing");
        if (b->forcing) s2_fail("recursive promise evaluation");
        b->forcing = 1; b->value = s2_eval(b->promise, b->caller); b->forcing = 0;
    }
    return b->value;
}
static void print_num(double x, int logical)
{
    if (x == S2_NA || x != x) printf("NA");
    else if (logical) printf(x != 0 ? "T" : "F");
    else printf("%.10g", x);
}
void s2_print(Value *v)
{
    int i, j;
    if (v->type == V_NULL) { puts("NULL"); return; }
    if (v->type == V_FUNC) { puts("<function>"); return; }
    if (v->type == V_EXPR) { puts("<expression>"); return; }
    if (v->type == V_LIST) {
        for (i = 0; i < v->length; i++) {
            if (v->names && v->names[i]) printf("$%s\n", v->names[i]); else printf("[[%d]]\n", i+1);
            s2_print(v->items[i]);
        }
        if (!v->length) puts("list()");
        return;
    }
    if (v->rows && v->type == V_NUM) {
        for (i = 0; i < v->rows; i++) {
            printf("[%d,]", i+1);
            for (j = 0; j < v->cols; j++) { putchar(' '); print_num(v->num[j*v->rows+i], v->logical); }
            putchar('\n');
        }
        return;
    }
    if (!v->length) { puts(v->type == V_STR ? "character(0)" : (v->logical ? "logical(0)" : "numeric(0)")); return; }
    for (i = 0; i < v->length; i++) {
        if (i % 8 == 0) { if (i) putchar('\n'); printf("[%d]", i+1); }
        putchar(' ');
        if (v->type == V_NUM) print_num(v->num[i], v->logical);
        else printf("\"%s\"", v->str[i] ? v->str[i] : "NA");
    }
    putchar('\n');
}
