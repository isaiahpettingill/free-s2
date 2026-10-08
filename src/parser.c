#include "s2.h"
enum { T_END = 256, T_NUM, T_STR, T_NAME, T_SEP, T_ASSIGN, T_GLOBAL,
       T_RIGHT, T_EQ, T_NE, T_LE, T_GE, T_AND, T_OR, T_LDB, T_RDB, T_MOD, T_IDIV };
static const char *input;
static int token, nesting, parsing_depth;
static double number;
static char word[256];
static void advance(void)
{
    int c, n, quote;
    char *end;
    for (;;) {
        while (*input == ' ' || *input == '\t' || *input == '\r') input++;
        if (*input != '#') break;
        while (*input && *input != '\n') input++;
    }
    c = (unsigned char)*input;
    if (!c) { token = T_END; return; }
    input++;
    if (c == '\n' || c == ';') { token = T_SEP; return; }
    if (isdigit(c) || (c == '.' && isdigit((unsigned char)*input))) {
        number = strtod(input - 1, &end); input = end; token = T_NUM; return;
    }
    if (isalpha(c) || c == '.') {
        n = 0; word[n++] = (char)c;
        while (isalnum((unsigned char)*input) || *input == '.') {
            if (n == 255) s2_fail("identifier too long");
            word[n++] = *input++;
        }
        word[n] = 0; token = T_NAME; return;
    }
    if (c == '"' || c == '\'') {
        quote = c; n = 0;
        while (*input && *input != quote) {
            c = (unsigned char)*input++;
            if (c == '\\') {
                if (!*input) s2_fail("unterminated escape");
                c = (unsigned char)*input++;
                if (c == 'n') c = '\n'; else if (c == 't') c = '\t';
            }
            if (n == 255) s2_fail("string too long");
            word[n++] = (char)c;
        }
        if (*input != quote) s2_fail("unterminated string");
        input++; word[n] = 0; token = T_STR; return;
    }
    token = c;
    if (c == '_') token = T_ASSIGN;
    if (c == '-' && *input == '>') { input++; token = T_RIGHT; }
    else
    if (c == '<' && *input == '<' && input[1] == '-') { input += 2; token = T_GLOBAL; }
    else if (c == '<' && *input == '-') { input++; token = T_ASSIGN; }
    else if (c == '=' && *input == '=') { input++; token = T_EQ; }
    else if (c == '!' && *input == '=') { input++; token = T_NE; }
    else if (c == '<' && *input == '=') { input++; token = T_LE; }
    else if (c == '>' && *input == '=') { input++; token = T_GE; }
    else if (c == '&' && *input == '&') { input++; token = T_AND; }
    else if (c == '|' && *input == '|') { input++; token = T_OR; }
    else if (c == '[' && *input == '[') { input++; token = T_LDB; }
    else if (c == ']' && *input == ']') { input++; token = T_RDB; }
    else if (c == '%' && *input == '%') { input++; token = T_MOD; }
    else if (c == '%' && input[0] == '/' && input[1] == '%') { input += 2; token = T_IDIV; }
}
static Node *make(int kind)
{
    Node *p; p = (Node *)s2_alloc(sizeof(Node)); p->kind = kind; return p;
}
static void skip(void) { while (token == T_SEP) advance(); }
static void expect(int t)
{
    if (token != t) s2_fail("unexpected token");
    advance();
}
static int keyword(const char *s) { return token == T_NAME && !strcmp(word, s); }
static Node *expression(int minimum);
static Node *group(void)
{
    Node *p; expect('('); nesting++; skip(); p = expression(0);
    skip(); expect(')'); nesting--; return p;
}
static Node *primary(void)
{
    Node *p, *tail, *q;
    int kind, saved_nesting;
    if (++parsing_depth > S2_MAX_DEPTH) s2_fail("parser nesting limit");
    if (nesting) skip();
    if (keyword("if")) {
        advance(); p = make(N_IF); p->a = group(); skip(); p->b = expression(0);
        /* Leave separators to the enclosing block unless else follows. */
        if (token == T_SEP) {
            const char *saved; int saved_token;
            saved = input; saved_token = token; skip();
            if (!keyword("else")) { input = saved; token = saved_token; }
        }
        if (keyword("else")) { advance(); skip(); p->c = expression(0); }
    } else if (keyword("while")) {
        advance(); p = make(N_WHILE); p->a = group(); skip(); p->b = expression(0);
    } else if (keyword("for")) {
        advance(); p = make(N_FOR); expect('('); nesting++; skip();
        if (token != T_NAME) s2_fail("for requires a variable");
        p->text = s2_strdup(word); advance();
        if (!keyword("in")) s2_fail("for requires in");
        advance(); p->a = expression(0); skip(); expect(')'); nesting--;
        skip(); p->b = expression(0);
    } else if (keyword("function")) {
        advance(); p = make(N_FUNC); expect('('); nesting++; skip(); tail = NULL;
        while (token != ')') {
            if (token != T_NAME) s2_fail("expected formal argument");
            q = make(N_NAME); q->text = s2_strdup(word); advance();
            if (token == '=') { advance(); q->a = expression(1); }
            if (tail) tail->next = q; else p->a = q; tail = q;
            skip(); if (token != ',') break; advance(); skip();
        }
        expect(')'); nesting--; skip(); p->b = expression(0);
    } else if (token == '{') {
        advance(); saved_nesting = nesting; nesting = 0; p = make(N_BLOCK); tail = NULL; skip();
        while (token != '}') {
            if (token == T_END) s2_fail("unclosed block");
            q = expression(0); if (tail) tail->next = q; else p->a = q; tail = q;
            if (token != '}' && token != T_SEP) s2_fail("expected statement separator");
            skip();
        }
        advance(); nesting = saved_nesting;
    } else if (token == T_NUM) {
        p = make(N_NUM); p->number = number; advance();
    } else if (token == T_STR || token == T_NAME) {
        kind = token; p = make(kind == T_STR ? N_STR : N_NAME);
        p->text = s2_strdup(word); advance();
    } else if (token == '(') p = group();
    else if (token == '+' || token == '-' || token == '!') {
        p = make(N_UNARY); p->op = token; advance(); p->a = expression(p->op == '!' ? 4 : 10);
    } else { s2_fail("expected expression"); p = NULL; }
    parsing_depth--; return p;
}
static int precedence(int t)
{
    if (t == T_ASSIGN || t == T_GLOBAL || t == T_RIGHT || t == '=') return 1;
    if (t == T_OR || t == '|' || t == T_AND || t == '&') return 3;
    if (t == T_EQ || t == T_NE || t == T_LE || t == T_GE || t == '<' || t == '>') return 5;
    if (t == '+' || t == '-') return 6;
    if (t == '*' || t == '/') return 7;
    if (t == T_MOD || t == T_IDIV) return 8;
    if (t == ':') return 9;
    if (t == '^') return 11;
    return -1;
}
static Node *expression(int minimum)
{
    Node *left, *p, *q, *tail;
    int op, prec, end;
    left = primary();
    for (;;) {
        if (nesting) skip();
        if (token == '(' || token == '[' || token == T_LDB) {
            op = token; advance(); nesting++; skip();
            p = make(op == '(' ? N_CALL : N_INDEX); p->op = op; p->a = left; tail = NULL;
            end = op == '(' ? ')' : (op == '[' ? ']' : T_RDB);
            while (token != end) {
                q = make(N_NAME);
                if (token != ',' && token != end) {
                    q->a = expression(0);
                    if (q->a->kind == N_BIN && q->a->op == '=' && q->a->a->kind == N_NAME) {
                        q->text = q->a->a->text; q->a = q->a->b;
                    }
                }
                if (tail) tail->next = q; else p->b = q; tail = q;
                skip(); if (token != ',') break; advance(); skip();
                if (token == end) { q = make(N_NAME); tail->next = q; }
            }
            expect(end); nesting--; left = p; continue;
        }
        if (token == '$') {
            advance(); if (token != T_NAME) s2_fail("expected member name");
            p = make(N_MEMBER); p->a = left; p->text = s2_strdup(word); advance(); left = p; continue;
        }
        prec = precedence(token); if (prec < minimum) break;
        op = token; advance(); skip(); p = make(N_BIN); p->op = op; p->a = left;
        p->b = expression(prec + (prec == 1 ? 0 : 1)); if (op == T_RIGHT) { q = p->a; p->a = p->b; p->b = q; p->op = T_ASSIGN; }
        left = p;
    }
    return left;
}
Node *s2_parse(const char *source)
{
    Node *p, *tail, *q;
    input = source; nesting = parsing_depth = 0; advance(); skip();
    p = make(N_BLOCK); tail = NULL;
    while (token != T_END) {
        q = expression(0); if (tail) tail->next = q; else p->a = q; tail = q;
        if (token != T_END && token != T_SEP) s2_fail("expected statement separator");
        skip();
    }
    return p;
}
/* Keep evaluator operator constants synchronized without exposing lexer. */
int s2_token_assign(int op) { return op == T_ASSIGN || op == T_GLOBAL; }
int s2_token_global(int op) { return op == T_GLOBAL; }
int s2_token_ldb(int op) { return op == T_LDB; }
int s2_token_binary(int op)
{
    if (op == T_EQ) return 1;
    if (op == T_NE) return 2;
    if (op == T_LE) return 3;
    if (op == T_GE) return 4;
    if (op == T_AND) return 5;
    if (op == T_OR) return 6;
    if (op == T_MOD) return 7;
    if (op == T_IDIV) return 8;
    return 0;
}
