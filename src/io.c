#include "s2.h"
static void quoted(FILE *f, const char *s) {
  unsigned char c;
  fputc('"', f);
  while (*s) {
    c = (unsigned char)*s++;
    if (c == '"' || c == '\\')
      fputc('\\', f);
    if (c == '\n')
      fputs("\\n", f);
    else if (c == '\t')
      fputs("\\t", f);
    else if (c == '\r')
      fputs("\\r", f);
    else if (c < 32)
      fprintf(f, "\\%03o", (unsigned)c);
    else
      fputc(c, f);
  }
  fputc('"', f);
}
void s2_write_node(FILE *f, Node *n) {
  Node *p;
  if (!n) {
    fputs("NULL", f);
    return;
  }
  switch (n->kind) {
  case N_NUM:
    if (s2_missing(n->number))
      fputs("NA", f);
    else
      fprintf(f, "%.17g", n->number);
    break;
  case N_STR:
    quoted(f, n->text);
    break;
  case N_NAME:
    fputs(n->text, f);
    break;
  case N_VALUE:
    s2_write_value(f, n->value);
    break;
  case N_PROMISE:
    s2_write_node(f, n->a);
    break;
  case N_BIN:
    fputc('(', f);
    s2_write_node(f, n->a);
    fprintf(f, " %s ", n->text ? n->text : s2_operator(n->op));
    s2_write_node(f, n->b);
    fputc(')', f);
    break;
  case N_UNARY:
    fprintf(f, "(%c", n->op);
    s2_write_node(f, n->a);
    fputc(')', f);
    break;
  case N_CALL:
  case N_INDEX:
    s2_write_node(f, n->a);
    fputs(n->kind == N_CALL ? "(" : s2_token_ldb(n->op) ? "[[" : "[", f);
    for (p = n->b; p; p = p->next) {
      if (p != n->b)
        fputc(',', f);
      if (p->text) {
        quoted(f, p->text);
        fputc('=', f);
      }
      if (p->a)
        s2_write_node(f, p->a);
    }
    fputs(n->kind == N_CALL ? ")" : s2_token_ldb(n->op) ? "]]" : "]", f);
    break;
  case N_MEMBER:
    s2_write_node(f, n->a);
    fputc('$', f);
    quoted(f, n->text);
    break;
  case N_BLOCK:
    fputc('{', f);
    for (p = n->a; p; p = p->next) {
      s2_write_node(f, p);
      fputc(';', f);
    }
    fputc('}', f);
    break;
  case N_FUNC:
    fputs("function(", f);
    for (p = n->a; p; p = p->next) {
      if (p != n->a)
        fputc(',', f);
      fputs(p->text, f);
      if (p->a) {
        fputc('=', f);
        s2_write_node(f, p->a);
      }
    }
    fputs(") ", f);
    s2_write_node(f, n->b);
    break;
  case N_IF:
    fputs("if(", f);
    s2_write_node(f, n->a);
    fputs(") ", f);
    s2_write_node(f, n->b);
    if (n->c) {
      fputs(" else ", f);
      s2_write_node(f, n->c);
    }
    break;
  case N_WHILE:
    fputs("while(", f);
    s2_write_node(f, n->a);
    fputs(") ", f);
    s2_write_node(f, n->b);
    break;
  case N_REPEAT:
    fputs("repeat ", f);
    s2_write_node(f, n->b);
    break;
  case N_FOR:
    fprintf(f, "for(%s in ", n->text);
    s2_write_node(f, n->a);
    fputs(") ", f);
    s2_write_node(f, n->b);
    break;
  default:
    s2_fail("cannot deparse language object");
  }
}
static void write_data(FILE *f, Value *v) {
  int i;
  if (v->type == V_NULL) {
    fputs("NULL", f);
    return;
  }
  if (v->type == V_FUNC) {
    if (v->builtin) {
      fputs("get(", f);
      quoted(f, v->builtin);
      fputs(",mode=\"function\")", f);
    } else {
      Node n;
      memset(&n, 0, sizeof(n));
      n.kind = N_FUNC;
      n.a = v->formals;
      n.b = v->body;
      s2_write_node(f, &n);
    }
    return;
  }
  if (v->type == V_LANG) {
    fputs("substitute(", f);
    s2_write_node(f, v->body);
    fputs(",list())", f);
    return;
  }
  if (v->type == V_EXPR) {
    fputs("expression(", f);
    for (i = 0; i < v->length; i++) {
      if (i)
        fputc(',', f);
      if (v->items[i]->type == V_LANG)
        s2_write_node(f, v->items[i]->body);
      else
        s2_write_value(f, v->items[i]);
    }
    fputc(')', f);
    return;
  }
  if (!v->length) {
    fputs(v->type == V_LIST      ? "list()"
          : v->type == V_STR     ? "character(0)"
          : v->type == V_COMPLEX ? "complex(0)"
          : v->logical           ? "logical(0)"
                                 : "numeric(0)",
          f);
    return;
  }
  if (v->integer)
    fputs("as.integer(", f);
  fputs(v->type == V_LIST ? "list(" : "c(", f);
  for (i = 0; i < v->length; i++) {
    if (i)
      fputc(',', f);
    if (v->type == V_LIST)
      s2_write_value(f, v->items[i]);
    else if (v->type == V_STR) {
      if (v->str[i])
        quoted(f, v->str[i]);
      else
        fputs("as.character(NA)", f);
    } else if (s2_missing(v->num[i]))
      fputs(v->type == V_COMPLEX ? "as.complex(NA)" : "NA", f);
    else if (v->type == V_COMPLEX)
      fprintf(f, "(%.17g%+.17gi)", v->num[i], v->imag[i]);
    else if (v->logical)
      fputs(v->num[i] ? "TRUE" : "FALSE", f);
    else
      fprintf(f, "%.17g", v->num[i]);
  }
  fputc(')', f);
  if (v->integer)
    fputc(')', f);
}
void s2_write_value(FILE *f, Value *v) {
  Attr *a;
  int i, hasdim;
  if (!v->attrs && !v->names && !v->rows) {
    write_data(f, v);
    return;
  }
  fputs("structure(", f);
  write_data(f, v);
  hasdim = 0;
  if (v->names) {
    fputs(",names=c(", f);
    for (i = 0; i < v->length; i++) {
      if (i)
        fputc(',', f);
      if (v->names[i])
        quoted(f, v->names[i]);
      else
        fputs("as.character(NA)", f);
    }
    fputc(')', f);
  }
  for (a = v->attrs; a; a = a->next) {
    if (!strcmp(a->name, "dim"))
      hasdim = 1;
    fputc(',', f);
    quoted(f, a->name);
    fputc('=', f);
    s2_write_value(f, a->value);
  }
  if (v->rows && !hasdim)
    fprintf(f, ",dim=c(%d,%d)", v->rows, v->cols);
  fputc(')', f);
}
static char *read_file(const char *file) {
  FILE *f;
  char *s;
  size_t n;
  f = fopen(file, "r");
  if (!f)
    s2_fail("cannot open source file");
  s = (char *)s2_alloc(S2_MAX_SOURCE + 1);
  n = fread(s, 1, S2_MAX_SOURCE, f);
  if (ferror(f) || (!feof(f) && fgetc(f) != EOF)) {
    fclose(f);
    s2_fail("source file exceeds limit or read failed");
  }
  fclose(f);
  s[n] = 0;
  return s;
}
void s2_execute(Node *program, Env *env) {
  Node *p;
  for (p = program->a; p; p = p->next)
    s2_eval(p, env);
}
void s2_source(const char *file, Env *env) {
  Node *p;
  p = s2_parse(read_file(file));
  s2_execute(p, env);
  s2_visible = 0;
}
static const char *string(Value *v) {
  if (!v || v->type != V_STR || v->length != 1 || !v->str[0])
    s2_fail("character scalar required");
  return v->str[0];
}
static FILE *output(const char *file, int append) {
  FILE *f;
  if (!*file)
    return stdout;
  f = fopen(file, append ? "a" : "w");
  if (!f)
    s2_fail("cannot open output file");
  return f;
}
static void close_output(FILE *f) {
  int bad;
  bad = ferror(f);
  if (f != stdout && fclose(f))
    bad = 1;
  if (bad)
    s2_fail("output write failed");
}
Value *s2_io(const char *name, Node *args, Env *env) {
  Value *v, *w, *r, *sep, *what, *column[S2_MAX_ARGS];
  Node *p, *program;
  FILE *f;
  const char *file, *s;
  char *text, *end, buf[4097];
  int i, j, n, m, limit, nfields, field, c, quote, first, append, columns,
      count, skipped;
  double x;
  if (!strcmp(name, "source") || !strcmp(name, "restore")) {
    file = string(s2_arg(args, 0, "file", env, NULL));
    s2_source(file, !strcmp(name, "restore") ? s2_expression : env);
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "parse")) {
    v = s2_arg(args, 2, "text", env, NULL);
    if (v) {
      v = s2_coerce(v, V_STR, 0);
      text = (char *)s2_alloc(S2_MAX_SOURCE + 1);
      n = 0;
      for (i = 0; i < v->length; i++) {
        if (!v->str[i])
          s2_fail("missing source text");
        m = (int)strlen(v->str[i]);
        if ((long)n + m + 1 > S2_MAX_SOURCE)
          s2_fail("source text too long");
        memcpy(text + n, v->str[i], (size_t)m);
        n += m;
        text[n++] = '\n';
      }
      text[n] = 0;
    } else {
      file = string(s2_arg(args, 0, "file", env, s2_string("")));
      if (*file)
        text = read_file(file);
      else {
        text = (char *)s2_alloc(S2_MAX_SOURCE + 1);
        if (!fgets(text, S2_MAX_SOURCE, stdin))
          text[0] = 0;
      }
    }
    program = s2_parse(text);
    n = 0;
    for (p = program->a; p; p = p->next)
      n++;
    w = s2_arg(args, 1, "n", env, NULL);
    if (w && s2_count(w) < n)
      n = s2_count(w);
    r = s2_value(V_EXPR, n);
    p = program->a;
    for (i = 0; i < n; i++, p = p->next) {
      r->items[i] = s2_value(V_LANG, 1);
      r->items[i]->body = p;
    }
    return r;
  }
  if (!strcmp(name, "deparse")) {
    v = s2_arg(args, 0, "expr", env, NULL);
    if (!v)
      s2_fail("expression required");
    f = tmpfile();
    if (!f)
      s2_fail("cannot create temporary file");
    if (v->type == V_LANG)
      s2_write_node(f, v->body);
    else
      s2_write_value(f, v);
    rewind(f);
    text = (char *)s2_alloc(S2_MAX_SOURCE + 1);
    n = (int)fread(text, 1, S2_MAX_SOURCE, f);
    text[n] = 0;
    if (!feof(f) && fgetc(f) != EOF) {
      fclose(f);
      s2_fail("deparse exceeds source limit");
    }
    fclose(f);
    return s2_string(text);
  }
  if (!strcmp(name, "dput") || !strcmp(name, "dump")) {
    v = s2_arg(args, 0, !strcmp(name, "dump") ? "list" : "x", env, NULL);
    if (!v)
      s2_fail("data required");
    file = string(s2_arg(args, 1, "file", env,
                         s2_string(!strcmp(name, "dump") ? "dumpdata" : "")));
    f = output(file, 0);
    if (!strcmp(name, "dput")) {
      s2_write_value(f, v);
      fputc('\n', f);
    } else {
      if (v->type != V_STR) {
        if (f != stdout)
          fclose(f);
        s2_fail("dump needs object names");
      }
      for (i = 0; i < v->length; i++) {
        w = s2_lookup(env, v->str[i]);
        if (!w) {
          if (f != stdout)
            fclose(f);
          s2_fail("dump object not found");
        }
        quoted(f, v->str[i]);
        fputs(" <- ", f);
        s2_write_value(f, w);
        fputc('\n', f);
      }
    }
    close_output(f);
    s2_visible = 0;
    return !strcmp(name, "dump") ? s2_string(file) : v;
  }
  if (!strcmp(name, "dget")) {
    file = string(s2_arg(args, 0, "file", env, NULL));
    program = s2_parse(read_file(file));
    r = s2_value(V_NULL, 0);
    for (p = program->a; p; p = p->next)
      r = s2_eval(p, env);
    return r;
  }
  if (!strcmp(name, "cat") || !strcmp(name, "write")) {
    file = string(s2_arg(args, -1, "file", env, s2_string("")));
    append = s2_scalar(s2_arg(args, -1, "append", env, s2_number(0))) != 0;
    sep = s2_arg(args, -1, "sep", env, s2_string(" "));
    s = string(sep);
    f = output(file, append);
    count = first = 0;
    columns = !strcmp(name, "write")
                  ? s2_count(s2_arg(args, 2, "ncolumns", env, s2_number(5)))
                  : 0;
    if (!strcmp(name, "write") && !columns) {
      if (f != stdout)
        fclose(f);
      s2_fail("ncolumns must be positive");
    }
    for (p = args; p; p = p->next) {
      if (p->text && strcmp(p->text, "x"))
        continue;
      v = s2_eval(p->a, p->scope ? p->scope : env);
      v = s2_coerce(v, V_STR, 0);
      for (i = 0; i < v->length; i++) {
        if (first && (!columns || count % columns))
          fputs(s, f);
        fputs(v->str[i] ? v->str[i] : "NA", f);
        first = 1;
        count++;
        if (columns && !(count % columns))
          fputc('\n', f);
      }
    }
    if (columns && count % columns)
      fputc('\n', f);
    close_output(f);
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "paste")) {
    Value *values[S2_MAX_ARGS];
    int nv, maxlen, total;
    const char *collapse;
    sep = s2_arg(args, -1, "sep", env, s2_string(" "));
    s = string(sep);
    w = s2_arg(args, -1, "collapse", env, NULL);
    collapse = w ? string(w) : NULL;
    nv = 0;
    maxlen = 0;
    for (p = args; p; p = p->next)
      if (!p->text || (strcmp(p->text, "sep") && strcmp(p->text, "collapse"))) {
        if (nv == S2_MAX_ARGS)
          s2_fail("too many paste arguments");
        values[nv] =
            s2_coerce(s2_eval(p->a, p->scope ? p->scope : env), V_STR, 0);
        if (values[nv]->length > maxlen)
          maxlen = values[nv]->length;
        nv++;
      }
    r = s2_value(V_STR, maxlen);
    for (i = 0; i < maxlen; i++) {
      total = 0;
      buf[0] = 0;
      for (j = 0; j < nv; j++) {
        v = values[j];
        text = v->length ? v->str[i % v->length] : "";
        if (!text)
          text = "NA";
        n = (int)strlen(text);
        m = j ? (int)strlen(s) : 0;
        if (total + n + m > 4096)
          s2_fail("paste string too long");
        if (j) {
          strcpy(buf + total, s);
          total += m;
        }
        strcpy(buf + total, text);
        total += n;
      }
      r->str[i] = s2_strdup(buf);
    }
    if (collapse) {
      total = 0;
      buf[0] = 0;
      for (i = 0; i < r->length; i++) {
        m = i ? (int)strlen(collapse) : 0;
        n = (int)strlen(r->str[i]);
        if (total + m + n > 4096)
          s2_fail("collapsed string too long");
        if (i) {
          strcpy(buf + total, collapse);
          total += m;
        }
        strcpy(buf + total, r->str[i]);
        total += n;
      }
      return s2_string(buf);
    }
    return r;
  }
  if (!strcmp(name, "scan")) {
    file = string(s2_arg(args, 0, "file", env, s2_string("")));
    f = *file ? fopen(file, "r") : stdin;
    if (!f)
      s2_fail("cannot open scan file");
    what = s2_arg(args, 1, "what", env, s2_number(0));
    nfields = what->type == V_LIST ? what->length : 1;
    if (!nfields || nfields > S2_MAX_ARGS) {
      if (f != stdin)
        fclose(f);
      s2_fail("invalid scan template");
    }
    sep = s2_arg(args, 3, "sep", env, s2_string(""));
    s = string(sep);
    if (strlen(s) > 1) {
      if (f != stdin)
        fclose(f);
      s2_fail("scan separator must be one byte");
    }
    limit = s2_count(s2_arg(args, 2, "n", env, s2_number(S2_MAX_VECTOR)));
    skipped = s2_count(s2_arg(args, 4, "skip", env, s2_number(0)));
    while (skipped && (c = fgetc(f)) != EOF)
      if (c == '\n')
        skipped--;
    for (i = 0; i < nfields; i++) {
      w = what->type == V_LIST ? what->items[i] : what;
      if (w->type != V_NUM && w->type != V_STR) {
        if (f != stdin)
          fclose(f);
        s2_fail("scan supports numeric, logical and character fields");
      }
      column[i] = s2_value(w->type, S2_MAX_VECTOR);
      column[i]->logical = w->logical;
    }
    n = field = 0;
    while (n < limit) {
      c = fgetc(f);
      while (c != EOF &&
             (!*s ? isspace((unsigned char)c) : c == '\n' || c == '\r'))
        c = fgetc(f);
      if (c == EOF)
        break;
      quote = 0;
      j = 0;
      if (c == '"' || c == '\'') {
        quote = c;
        c = fgetc(f);
      }
      while (c != EOF && (quote ? c != quote
                          : *s  ? c != *s && c != '\n' && c != '\r'
                                : !isspace((unsigned char)c))) {
        if (j == 4096) {
          if (f != stdin)
            fclose(f);
          s2_fail("scan token too long");
        }
        buf[j++] = (char)c;
        c = fgetc(f);
      }
      buf[j] = 0;
      if (quote) {
        if (c != quote) {
          if (f != stdin)
            fclose(f);
          s2_fail("unclosed scan quote");
        }
        c = fgetc(f);
        if (c != EOF && (!*s ? !isspace((unsigned char)c)
                             : c != *s && c != '\n' && c != '\r'))
          ungetc(c, f);
      }
      v = column[field];
      if (v->type == V_STR)
        v->str[n] = !strcmp(buf, "NA") ? NULL : s2_strdup(buf);
      else {
        if (!strcmp(buf, "NA"))
          x = S2_NA;
        else if (v->logical && (!strcmp(buf, "T") || !strcmp(buf, "TRUE")))
          x = 1;
        else if (v->logical && (!strcmp(buf, "F") || !strcmp(buf, "FALSE")))
          x = 0;
        else {
          x = strtod(buf, &end);
          while (isspace((unsigned char)*end))
            end++;
          if (end == buf || *end) {
            if (f != stdin)
              fclose(f);
            s2_fail("invalid numeric scan field");
          }
        }
        v->num[n] = x;
      }
      field++;
      if (field == nfields) {
        field = 0;
        n++;
      }
    }
    if (f != stdin)
      fclose(f);
    if (field)
      s2_fail("incomplete scan record");
    for (i = 0; i < nfields; i++)
      column[i]->length = n;
    if (what->type != V_LIST)
      return column[0];
    r = s2_value(V_LIST, nfields);
    r->names = what->names;
    for (i = 0; i < nfields; i++)
      r->items[i] = column[i];
    return r;
  }
  if (!strcmp(name, "help")) {
    const char *topic, *signature;
    topic = NULL;
    if (args && args->a) {
      if (args->a->kind == N_NAME)
        topic = args->a->text;
      else {
        v = s2_arg(args, 0, "topic", env, NULL);
        if (v && v->type == V_STR && v->length == 1)
          topic = v->str[0];
      }
      if (!topic)
        s2_fail("help topic must be name or string");
      signature = s2_signature(topic);
      if (signature)
        printf("%s(%s)\n", topic, signature);
      else
        printf("No built-in help for %s\n", topic);
    }
    puts("free-s2: see docs/guide.md and docs/features.md. source(file) runs a "
         "script; q() exits.");
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  return NULL;
}
