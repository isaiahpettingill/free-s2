#include "s2.h"
typedef struct Allocation Allocation;
struct Allocation { Allocation *next; int marked; };
static Allocation *allocations;
jmp_buf s2_error;
void s2_fail(const char *message)
{
    fprintf(stderr, "Error: %s\n", message);
    longjmp(s2_error, 1);
}
void *s2_alloc(size_t size)
{
    Allocation *p;
    if (size > 60000U - sizeof(Allocation)) s2_fail("allocation exceeds DOS segment");
    p = (Allocation *)calloc(1, sizeof(Allocation) + size);
    if (!p) s2_fail("out of memory");
    p->next = allocations; allocations = p;
    return p + 1;
}
char *s2_strdup(const char *text)
{
    char *p;
    p = (char *)s2_alloc(strlen(text) + 1);
    strcpy(p, text); return p;
}
void s2_free_all(void)
{
    Allocation *p;
    while (allocations) { p = allocations; allocations = p->next; free(p); }
}

static int mark(void *p)
{
    Allocation *a;
    if (!p) return 0;
    a = (Allocation *)p - 1;
    if (a->marked) return 0;
    a->marked = 1; return 1;
}
static void mark_node(Node *p)
{
    while (p && mark(p)) {
        mark(p->text); mark_node(p->a); mark_node(p->b); mark_node(p->c); p = p->next;
    }
}
static void mark_env(Env *env);
static void mark_value(Value *v)
{
    int i;
    if (!v || !mark(v)) return;
    mark(v->num); mark(v->str); mark(v->items); mark(v->names);
    if (v->type == V_STR) for (i = 0; i < v->length; i++) mark(v->str[i]);
    if (v->type == V_LIST) for (i = 0; i < v->length; i++) mark_value(v->items[i]);
    if (v->names) for (i = 0; i < v->length; i++) mark(v->names[i]);
    mark_node(v->formals); mark_node(v->body);
}
static void mark_env(Env *env)
{
    Binding *b;
    if (!env) return;
    if (env != &s2_global && !mark(env)) return;
    for (b = env->bindings; b && mark(b); b = b->next) {
        mark(b->name); mark_value(b->value); mark_node(b->promise); mark_env(b->caller);
    }
}
void s2_collect(Node *root)
{
    Allocation *p, **link;
    for (p = allocations; p; p = p->next) p->marked = 0;
    mark_node(root); mark_env(&s2_global);
    link = &allocations;
    while (*link) {
        p = *link;
        if (!p->marked) { *link = p->next; free(p); } else link = &p->next;
    }
    s2_return = NULL;
}
