#ifndef FREE_S2_H
#define FREE_S2_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <setjmp.h>
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
enum { V_NULL, V_NUM, V_STR, V_LIST, V_FUNC, V_EXPR };
enum { N_NUM, N_STR, N_NAME, N_CALL, N_BIN, N_UNARY, N_BLOCK,
       N_IF, N_WHILE, N_FOR, N_FUNC, N_INDEX, N_MEMBER };
struct Node { int kind, op; double number; char *text;
              Node *a, *b, *c, *next; };
struct Value { int type, length, logical, rows, cols;
               double *num; char **str; Value **items; char **names;
               Node *formals, *body; };
struct Binding { char *name; Value *value; Node *promise;
                 Env *caller; int forcing; Binding *next; };
struct Env { Binding *bindings; };
extern Env s2_global;
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
double s2_sum(double *x, int n);
double s2_mean(double *x, int n);
double s2_var(double *x, int n);
#endif
