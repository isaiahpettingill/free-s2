#include "s2.h"
static char source[S2_MAX_SOURCE+1];
/* Shallow binding snapshot: assigned values are immutable/copy-on-write. */
static Binding *checkpoint;
static int transaction;
static void begin_expression(void)
{
    Binding *b, *copy;
    checkpoint = NULL;
    for (b = s2_global.bindings; b; b = b->next) {
        copy = (Binding *)s2_alloc(sizeof(Binding)); *copy = *b;
        copy->next = checkpoint; checkpoint = copy;
    }
    transaction = 1;
}
static int run(const char *text, int echo)
{
    Node *program, *node;
    Value *v;
    if (setjmp(s2_error)) { if (transaction) s2_global.bindings = checkpoint; transaction = 0; s2_depth = s2_flow = 0; s2_collect(NULL); return 1; }
    program = s2_parse(text);
    for (node = program->a; node; node = node->next) {
        begin_expression();
        v = s2_eval(node,&s2_global);
        if (s2_flow) s2_fail("control transfer outside function/loop");
        if (echo && s2_visible) s2_print(v);
        transaction = 0; checkpoint = NULL;
        s2_collect(program);
    }
    s2_collect(NULL); return 0;
}
/* Scan incomplete interactive input without speculative parsing/evaluation. */
static int incomplete(const char *s)
{
    int quote, escape, paren, brace, bracket, last, comment;
    quote = escape = paren = brace = bracket = last = comment = 0;
    for (; *s; s++) {
        if (comment) { if (*s == '\n') comment = 0; continue; }
        if (escape) { escape = 0; continue; }
        if (quote) { if (*s == '\\') escape = 1; else if (*s == quote) quote = 0; continue; }
        if (*s == '#') { comment = 1; continue; }
        if (*s == '"' || *s == '\'') { quote = *s; last = 'a'; continue; }
        if (*s == '(') paren++;
        if (*s == ')') paren--;
        if (*s == '{') brace++;
        if (*s == '}') brace--;
        if (*s == '[') bracket++;
        if (*s == ']') bracket--;
        if (!isspace((unsigned char)*s)) last = *s;
    }
    return quote || paren > 0 || brace > 0 || bracket > 0 || (last && strchr("+-*/^<>=&|,:_",last));
}
int main(int argc, char **argv)
{
    FILE *f;
    size_t length, got;
    int result;
    result = 0;
    if (argc > 1 && !strcmp(argv[1],"--version")) { puts("free-s2 0.1-dev (1988 S subset)"); return 0; }
    if (argc > 2 && !strcmp(argv[1],"-e")) result = run(argv[2],1);
    else if (argc == 2) {
        f = fopen(argv[1],"r"); if (!f) { fprintf(stderr,"Cannot open %s\n",argv[1]); return 1; }
        got = fread(source,1,S2_MAX_SOURCE,f);
        if (ferror(f) || (!feof(f) && fgetc(f) != EOF)) { fclose(f); fputs("Source file too large or unreadable\n",stderr); return 1; }
        fclose(f); source[got] = 0; result = run(source,1);
    } else if (argc != 1) { fputs("Usage: s2 [FILE | -e EXPRESSION | --version]\n",stderr); result = 1; }
    else {
        puts("free-s2 0.1-dev - q() to exit; no workspace persistence");
        for (;;) {
            length = 0; source[0] = 0;
            do {
                fputs(length ? "+ " : "> ",stdout); fflush(stdout);
                if (!fgets(source+length,(int)(S2_MAX_SOURCE-length+1),stdin)) goto done;
                length = strlen(source);
                if (length >= S2_MAX_SOURCE) { fputs("Input too long\n",stderr); goto done; }
            } while (incomplete(source));
            run(source,1);
        }
    }
done:
    s2_free_all(); return result;
}
