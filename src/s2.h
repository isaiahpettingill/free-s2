#ifndef FREE_S2_H
#define FREE_S2_H
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* C89, 16-bit int safe. Each allocation stays within one DOS segment. */
#define S2_MAX_VECTOR 2048
#define S2_MAX_ARGS 64
#define S2_MAX_DEPTH 128
#define S2_MAX_SOURCE 30000
#define S2_NA DBL_MAX

typedef struct Node Node;
typedef struct Value Value;
typedef struct Binding Binding;
typedef struct Env Env;
typedef struct Attr Attr;
typedef struct Control Control;
enum { V_NULL, V_NUM, V_STR, V_LIST, V_FUNC, V_EXPR, V_LANG, V_COMPLEX };
enum {
  N_NUM,
  N_STR,
  N_NAME,
  N_CALL,
  N_BIN,
  N_UNARY,
  N_BLOCK,
  N_IF,
  N_WHILE,
  N_FOR,
  N_FUNC,
  N_INDEX,
  N_MEMBER,
  N_VALUE,
  N_REPEAT,
  N_PROMISE
};
struct Node {
  int kind, op;
  double number;
  char *text;
  Node *a, *b, *c, *next;
  Value *value;
  Env *scope;
  Node *origin;
};
struct Value {
  int type, length, logical, rows, cols;
  double *num, *imag;
  char **str;
  Value **items;
  char **names;
  Node *formals, *body;
  char *builtin;
  int integer;
  Attr *attrs;
};
struct Binding {
  char *name;
  Value *value;
  Node *promise;
  Env *caller;
  int forcing, formal, missing;
  Binding *next;
};
struct Env {
  Binding *bindings;
  Node *dots;
  int nargs;
};
struct Attr {
  char *name;
  Value *value;
  Attr *next;
};
struct Control {
  jmp_buf jump;
  Control *previous;
  Value *result;
  int depth, index;
};
extern Env s2_global, s2_session;
extern Env *s2_expression;
extern Control *s2_function_control, *s2_loop_control;
extern jmp_buf s2_error;
extern int s2_depth, s2_flow, s2_visible;
extern Value *s2_return;
void *s2_alloc(size_t size);
char *s2_strdup(const char *text);
void s2_free_all(void);
void s2_collect(Node *root);
void s2_fail(const char *message);
Node *s2_parse(const char *source);
Value *s2_eval(Node *node, Env *env);
Value *s2_value(int type, int length);
Value *s2_number(double number);
Value *s2_copy(Value *value);
void s2_set(Env *env, const char *name, Value *value);
Value *s2_lookup(Env *env, const char *name);
void s2_print(Value *value);
Value *s2_builtin(const char *name, Node *args, Env *env);
Value *s2_element(Value *v, int i);
Value *s2_coerce(Value *v, int type, int logical);
int s2_common(Value *a, Value *b);
Value *s2_attr(Value *v, const char *name);
Value *s2_with_attr(Value *v, const char *name, Value *a);
Binding *s2_binding(Env *env, const char *name);
Value *s2_function(Env *env, const char *name);
int s2_builtin_exists(const char *name);
Value *s2_call(Value *fn, Node *args, Env *caller);
Node *s2_normalize(const char *name, Node *args);
Node *s2_expand(Node *args, Env *env);
void s2_transfer(int code, Value *v);
void s2_assign(Node *target, Value *v, Env *env, int global);
Value *s2_subset(Value *v, Node *args, Env *env, int single, Value *rhs);
Value *s2_arg(Node *args, int pos, const char *name, Env *env, Value *def);
double s2_scalar(Value *v);
int s2_count(Value *v);
Value *s2_string(const char *s);
int s2_missing(double x);
Value *s2_extra(const char *name, Node *args, Env *env);
int s2_token_binary(int op);
Value *s2_complex_binary(Value *a, Value *b, int op);
Value *s2_math(const char *name, Node *args, Env *env);
Value *s2_group(const char *name, Node *args, Env *env);
Value *s2_linear(const char *name, Node *args, Env *env);
Value *s2_statistics(const char *name, Node *args, Env *env);
Value *s2_io(const char *name, Node *args, Env *env);
Value *s2_graphics(const char *name, Node *args, Env *env);
void s2_graphics_close(void);
void s2_write_value(FILE *f, Value *v);
void s2_write_node(FILE *f, Node *node);
int s2_token_ldb(int op);
const char *s2_operator(int op);
void s2_execute(Node *program, Env *env);
void s2_source(const char *file, Env *env);
double s2_sum(double *x, int n);
double s2_mean(double *x, int n);
double s2_var(double *x, int n);
const char *s2_signature(const char *name);
#endif
