#include "s2.h"
typedef struct {
  const char *name, *signature;
} Signature;
static const Signature signatures[] = {
    {"c", "..."},
    {"list", "..."},
    {"expression", "..."},
    {"quote", "x"},
    {"substitute", "expr,list"},
    {"missing", "x"},
    {"eval", "expression,local,parent"},
    {"parse", "file,n,text,prompt"},
    {"deparse", "expr"},
    {"return", "value"},
    {"stop", "message"},
    {"warning", "message"},
    {"q", ""},
    {"print", "x"},
    {"cat", "...,file,sep,append"},
    {"paste", "...,sep,collapse"},
    {"numeric", "length"},
    {"logical", "length"},
    {"character", "length"},
    {"complex", "length,real,imaginary"},
    {"integer", "length"},
    {"vector", "mode,length"},
    {"sum", "..."},
    {"prod", "..."},
    {"min", "..."},
    {"max", "..."},
    {"range", "..."},
    {"any", "..."},
    {"all", "..."},
    {"mean", "x,trim"},
    {"var", "x,y"},
    {"cor", "x,y,trim"},
    {"median", "x"},
    {"quantile", "x,probs"},
    {"summary", "x"},
    {"seq", "from,to,by,length,along"},
    {"length", "x"},
    {"mode", "x"},
    {"storage.mode", "x"},
    {"names", "x"},
    {"dim", "x"},
    {"dimnames", "x"},
    {"class", "x"},
    {"tsp", "x"},
    {"attr", "x,which"},
    {"attributes", "x"},
    {"structure", ".Data,..."},
    {"is.na", "x"},
    {"is.numeric", "x"},
    {"is.logical", "x"},
    {"is.character", "x"},
    {"is.complex", "x"},
    {"is.list", "x"},
    {"is.function", "x"},
    {"is.null", "x"},
    {"is.matrix", "x"},
    {"is.array", "x"},
    {"is.vector", "x"},
    {"is.expression", "x"},
    {"is.call", "x"},
    {"is.name", "x"},
    {"as.numeric", "x"},
    {"as.integer", "x"},
    {"as.logical", "x"},
    {"as.character", "x"},
    {"as.complex", "x"},
    {"as.list", "x"},
    {"as.vector", "x,mode"},
    {"as.matrix", "x"},
    {"as.array", "x"},
    {"as.expression", "x"},
    {"rep", "x,times,length"},
    {"rev", "x"},
    {"sort", "x"},
    {"order", "..."},
    {"rank", "x"},
    {"unique", "x"},
    {"duplicated", "x"},
    {"match", "x,table,nomatch"},
    {"pmatch", "x,table,nomatch"},
    {"matrix", "data,nrow,ncol,byrow,dimnames"},
    {"array", "data,dim,dimnames"},
    {"t", "x"},
    {"nrow", "x"},
    {"ncol", "x"},
    {"cbind", "..."},
    {"rbind", "..."},
    {"diag", "x"},
    {"crossprod", "x,y"},
    {"outer", "X,Y,FUN,..."},
    {"solve", "a,b"},
    {"lsfit", "x,y,wt,intercept,tolerance"},
    {"lapply", "X,FUN,..."},
    {"sapply", "X,FUN,..."},
    {"apply", "X,MARGIN,FUN,..."},
    {"tapply", "X,INDICES,FUN,..."},
    {"table", "..."},
    {"split", "x,f"},
    {"cut", "x,breaks,labels"},
    {"category", "x"},
    {"as.category", "x"},
    {"is.category", "x"},
    {"levels", "x"},
    {"diff", "x,lag,differences"},
    {"cumsum", "x"},
    {"cumprod", "x"},
    {"cummin", "x"},
    {"cummax", "x"},
    {"pmin", "..."},
    {"pmax", "..."},
    {"sqrt", "x"},
    {"abs", "x"},
    {"sin", "x"},
    {"cos", "x"},
    {"tan", "x"},
    {"asin", "x"},
    {"acos", "x"},
    {"atan", "x"},
    {"sinh", "x"},
    {"cosh", "x"},
    {"tanh", "x"},
    {"exp", "x"},
    {"log", "x,base"},
    {"log10", "x"},
    {"floor", "x"},
    {"ceiling", "x"},
    {"trunc", "x"},
    {"round", "x,digits"},
    {"signif", "x,digits"},
    {"sign", "x"},
    {"atan2", "y,x"},
    {"asinh", "x"},
    {"acosh", "x"},
    {"atanh", "x"},
    {"Re", "z"},
    {"Im", "z"},
    {"Mod", "z"},
    {"Arg", "z"},
    {"Conj", "z"},
    {"gamma", "x"},
    {"lgamma", "x"},
    {"beta", "a,b"},
    {"lbeta", "a,b"},
    {"nchar", "x"},
    {"substring", "text,first,last"},
    {"substr", "text,start,stop"},
    {"source", "file"},
    {"scan", "file,what,n,sep,skip"},
    {"write", "x,file,ncolumns,append,sep"},
    {"dump", "list,file"},
    {"dput", "x,file"},
    {"dget", "file"},
    {"restore", "file"},
    {"objects", "where,pattern"},
    {"ls", "where,pattern"},
    {"remove", "..."},
    {"rm", "..."},
    {"get", "name,where,frame,mode"},
    {"exists", "name,where,frame,mode"},
    {"assign", "name,value,where,frame"},
    {"nargs", ""},
    {"help", "topic"},
    {"dnorm", "q,mean,sd"},
    {"pnorm", "q,mean,sd"},
    {"qnorm", "p,mean,sd"},
    {"rnorm", "n,mean,sd"},
    {"dunif", "q,min,max"},
    {"punif", "q,min,max"},
    {"qunif", "p,min,max"},
    {"runif", "n,min,max"},
    {"dexp", "q"},
    {"pexp", "q"},
    {"qexp", "p"},
    {"rexp", "n"},
    {"dbinom", "q,size,prob"},
    {"pbinom", "q,size,prob"},
    {"qbinom", "p,size,prob"},
    {"rbinom", "n,size,prob"},
    {"dpois", "q,lambda"},
    {"ppois", "q,lambda"},
    {"qpois", "p,lambda"},
    {"rpois", "n,lambda"},
    {"dt", "q,df"},
    {"pt", "q,df"},
    {"qt", "p,df"},
    {"rt", "n,df"},
    {"dchisq", "q,df"},
    {"pchisq", "q,df"},
    {"qchisq", "p,df"},
    {"rchisq", "n,df"},
    {"dgamma", "q,shape"},
    {"pgamma", "q,shape"},
    {"qgamma", "p,shape"},
    {"rgamma", "n,shape"},
    {"dbeta", "q,shape1,shape2"},
    {"pbeta", "q,shape1,shape2"},
    {"qbeta", "p,shape1,shape2"},
    {"rbeta", "n,shape1,shape2"},
    {"df", "q,df1,df2"},
    {"pf", "q,df1,df2"},
    {"qf", "p,df1,df2"},
    {"rf", "n,df1,df2"},
    {"dcauchy", "q,location,scale"},
    {"pcauchy", "q,location,scale"},
    {"qcauchy", "p,location,scale"},
    {"rcauchy", "n,location,scale"},
    {"dlogis", "q,location,scale"},
    {"plogis", "q,location,scale"},
    {"qlogis", "p,location,scale"},
    {"rlogis", "n,location,scale"},
    {"dweibull", "q,shape"},
    {"pweibull", "q,shape"},
    {"qweibull", "p,shape"},
    {"rweibull", "n,shape"},
    {"sample", "x,size,replace"},
    {"set.seed", "i"},
    {"svg", "file,width,height"},
    {"postscript", "file,width,height"},
    {"dev.off", ""},
    {"graphics.off", ""},
    {"plot", "x,y,type,xlim,ylim,xlab,ylab,main,col"},
    {"lines", "x,y,type,col"},
    {"points", "x,y,type,col"},
    {"abline", "a,b,h,v,col"},
    {"text", "x,y,labels,col"},
    {"title", "main,sub,xlab,ylab"},
    {"hist", "x,nclass,breaks,plot,main,col"},
    {"barplot", "height,names,main,col"},
    {"boxplot", "...,main"},
    {"qqnorm", "y,main"},
    {"qqplot", "x,y,main"},
    {"polygon", "x,y,col"},
    {"%*%", "x,y"},
    {"%o%", "x,y"},
    {"%in%", "x,table"},
    {"+", "..."},
    {"-", "..."},
    {"*", "x,y"},
    {"/", "x,y"},
    {"^", "x,y"},
    {"<", "x,y"},
    {">", "x,y"},
    {"<=", "x,y"},
    {">=", "x,y"},
    {"==", "x,y"},
    {"!=", "x,y"},
    {"&", "x,y"},
    {"|", "x,y"},
    {"!", "x"},
    {"%%", "x,y"},
    {"%/%", "x,y"},
    {NULL, NULL}};
int s2_builtin_exists(const char *name) {
  int i;
  for (i = 0; signatures[i].name; i++)
    if (!strcmp(name, signatures[i].name))
      return 1;
  return 0;
}
Node *s2_normalize(const char *name, Node *args) {
  int i, n, dot, pos, j, idx, seen[S2_MAX_ARGS];
  char buf[300], *p, *q, *names[S2_MAX_ARGS];
  Node *a, *r, *head, *tail;
  for (i = 0; signatures[i].name; i++)
    if (!strcmp(name, signatures[i].name))
      break;
  if (!signatures[i].name)
    s2_fail("unknown function");
  strcpy(buf, signatures[i].signature);
  n = 0;
  dot = -1;
  p = buf;
  while (*p) {
    q = strchr(p, ',');
    if (q)
      *q = 0;
    names[n] = p;
    seen[n] = 0;
    if (!strcmp(p, "..."))
      dot = n;
    n++;
    if (!q)
      break;
    p = q + 1;
  }
  /* Reserve exact matches before considering unnamed/partial arguments. */
  for (a = args; a; a = a->next)
    if (a->text)
      for (i = 0; i < n; i++)
        if (i != dot && !strcmp(a->text, names[i])) {
          if (seen[i])
            s2_fail("argument matched more than once");
          seen[i] = 1;
        }
  pos = 0;
  head = tail = NULL;
  for (a = args; a; a = a->next) {
    idx = -1;
    if (a->text) {
      for (i = 0; i < n; i++)
        if (i != dot && !strcmp(a->text, names[i])) {
          idx = i;
          break;
        }
      if (idx < 0)
        for (i = 0; i < (dot < 0 ? n : dot); i++)
          if (!strncmp(names[i], a->text, strlen(a->text))) {
            if (idx >= 0)
              s2_fail("ambiguous partial argument name");
            idx = i;
          }
      if (idx >= 0 && strcmp(a->text, names[idx])) {
        if (seen[idx])
          s2_fail("argument matched more than once");
        seen[idx] = 1;
      }
    } else {
      while (pos < (dot < 0 ? n : dot) && seen[pos])
        pos++;
      if (pos < (dot < 0 ? n : dot)) {
        idx = pos++;
        seen[idx] = 1;
      }
    }
    if (idx < 0 && dot < 0)
      s2_fail("unused argument");
    r = (Node *)s2_alloc(sizeof(Node));
    *r = *a;
    r->next = NULL;
    if (idx >= 0)
      r->text = s2_strdup(names[idx]);
    if (tail)
      tail->next = r;
    else
      head = r;
    tail = r;
  }
  (void)j;
  return head;
}
static Node *constant(Value *v) {
  Node *r;
  r = (Node *)s2_alloc(sizeof(Node));
  r->kind = N_VALUE;
  r->value = v;
  return r;
}
static Node *substitution(Node *p, Env *env) {
  Node *r;
  Binding *b;
  if (!p)
    return NULL;
  if (p->kind == N_NAME && !p->a && p->text) {
    b = s2_binding(env, p->text);
    if (b) {
      if (b->formal && b->promise) {
        Node *source;
        source = b->promise;
        while (source->kind == N_PROMISE)
          source = source->a;
        r = (Node *)s2_alloc(sizeof(Node));
        *r = *source;
        r->next = NULL;
        return r;
      }
      if (b->value)
        return constant(b->value);
    }
  }
  r = (Node *)s2_alloc(sizeof(Node));
  *r = *p;
  r->a = substitution(p->a, env);
  r->b = substitution(p->b, env);
  r->c = substitution(p->c, env);
  r->next = substitution(p->next, env);
  return r;
}
static Value *language(Node *p) {
  Value *r;
  r = s2_value(V_LANG, 1);
  r->body = p;
  return r;
}
static Value *call_one(Value *fn, Value *x, Node *rest, Env *env) {
  Node *a;
  a = (Node *)s2_alloc(sizeof(Node));
  a->a = constant(x);
  a->next = rest;
  return s2_call(fn, a, env);
}
static int compare(Value *a, int i, Value *b, int j) {
  if (a->type == V_STR) {
    if (!a->str[i] || !b->str[j])
      return a->str[i] == b->str[j] ? 0 : !a->str[i] ? 1 : -1;
    return strcmp(a->str[i], b->str[j]);
  }
  if (s2_missing(a->num[i]) || s2_missing(b->num[j]))
    return s2_missing(a->num[i]) == s2_missing(b->num[j]) ? 0
           : s2_missing(a->num[i])                        ? 1
                                                          : -1;
  if (a->type == V_COMPLEX) {
    if (a->num[i] != b->num[j])
      return a->num[i] < b->num[j] ? -1 : 1;
    return a->imag[i] == b->imag[j] ? 0 : a->imag[i] < b->imag[j] ? -1 : 1;
  }
  return a->num[i] == b->num[j] ? 0 : a->num[i] < b->num[j] ? -1 : 1;
}
Value *s2_extra(const char *name, Node *args, Env *env) {
  Value *v, *r, *w, *z, *fn, *dim;
  Node *p, *rest;
  Env *frame;
  Binding *b;
  Attr *attr;
  int i, j, k, n, m, rows, cols, axis, type, flag;
  double x, y;
  char buf[256];
  if (strchr("+-*/^<>=!&|%", name[0]) && strcmp(name, "%in%")) {
    Node *tree;
    Value *left, *right;
    char expression[64];
    if (strlen(name) > 8)
      return NULL;
    left = s2_arg(args, 0, "x", env, NULL);
    right = s2_arg(args, 1, "y", env, NULL);
    if (!left)
      s2_fail("operand required");
    if (!right &&
        (!strcmp(name, "+") || !strcmp(name, "-") || !strcmp(name, "!"))) {
      sprintf(expression, "%sa", name);
      tree = s2_parse(expression)->a;
      tree->a = (Node *)s2_alloc(sizeof(Node));
      tree->a->kind = N_VALUE;
      tree->a->value = left;
    } else {
      if (!right)
        return NULL;
      sprintf(expression, "a %s b", name);
      tree = s2_parse(expression)->a;
      if (tree->text)
        return NULL;
      tree->a = (Node *)s2_alloc(sizeof(Node));
      tree->b = (Node *)s2_alloc(sizeof(Node));
      tree->a->kind = tree->b->kind = N_VALUE;
      tree->a->value = left;
      tree->b->value = right;
    }
    return s2_eval(tree, env);
  }
  sprintf(buf, "|%s|", name);
  if (!strstr("|missing|substitute|quote|expression|eval|nargs|get|exists|"
              "assign|objects|ls|remove|rm|structure|attr|names|dim|dimnames|"
              "class|tsp|attributes|match|pmatch|%in%|unique|duplicated|rank|"
              "order|array|nrow|ncol|t|lapply|sapply|apply|nchar|substring|"
              "substr|Re|Im|Mod|Arg|Conj|",
              buf) &&
      strncmp(name, "as.", 3) &&
      (strncmp(name, "is.", 3) || !strcmp(name, "is.na")))
    return NULL;
  if (!strcmp(name, "missing")) {
    if (!args || !args->a || args->a->kind != N_NAME)
      s2_fail("missing requires formal argument name");
    b = s2_binding(env, args->a->text);
    if (!b || !b->formal)
      s2_fail("missing requires formal argument name");
    r = s2_number(b->missing);
    r->logical = 1;
    return r;
  }
  if (!strcmp(name, "substitute") || !strcmp(name, "quote")) {
    if (!args || !args->a)
      s2_fail("expression required");
    frame = env;
    w = s2_arg(args, 1, "list", env, NULL);
    if (w) {
      if (w->type != V_LIST)
        s2_fail("substitute list required");
      frame = (Env *)s2_alloc(sizeof(Env));
      for (i = 0; i < w->length; i++)
        if (w->names && w->names[i])
          s2_set(frame, w->names[i], w->items[i]);
    }
    return language(!strcmp(name, "quote") ? args->a
                                           : substitution(args->a, frame));
  }
  if (!strcmp(name, "expression")) {
    n = 0;
    for (p = args; p; p = p->next)
      n++;
    r = s2_value(V_EXPR, n);
    i = 0;
    for (p = args; p; p = p->next)
      r->items[i++] = language(p->a);
    return r;
  }
  if (!strcmp(name, "eval")) {
    if (s2_arg(args, 2, "parent", env, NULL))
      s2_fail("eval parent frames are not implemented");
    v = s2_arg(args, 0, "expression", env, NULL);
    if (!v)
      s2_fail("expression required");
    w = s2_arg(args, 1, "local", env, NULL);
    frame = env;
    if (w) {
      if (w->type == V_LIST) {
        frame = (Env *)s2_alloc(sizeof(Env));
        for (i = 0; i < w->length; i++)
          if (w->names && w->names[i])
            s2_set(frame, w->names[i], w->items[i]);
      } else if (w->type == V_NUM && w->length == 1 && w->num[0] == 0)
        frame = &s2_global;
      else
        s2_fail("eval local must be list or FALSE");
    }
    if (v->type == V_LANG)
      return s2_eval(v->body, frame);
    if (v->type == V_EXPR) {
      r = s2_value(V_NULL, 0);
      for (i = 0; i < v->length; i++)
        r = v->items[i]->type == V_LANG ? s2_eval(v->items[i]->body, frame)
                                        : v->items[i];
      return r;
    }
    return v;
  }
  if (!strcmp(name, "nargs"))
    return s2_number(env->nargs);
  if (!strcmp(name, "get") || !strcmp(name, "exists") ||
      !strcmp(name, "assign")) {
    if (s2_arg(args, -1, "where", env, NULL) ||
        s2_arg(args, -1, "frame", env, NULL))
      s2_fail("explicit search/frame selection is not implemented");
    v = s2_arg(args, 0, "name", env, NULL);
    if (!v || v->type != V_STR || v->length != 1)
      s2_fail("name string required");
    if (!strcmp(name, "assign")) {
      r = s2_arg(args, 1, "value", env, NULL);
      if (!r)
        s2_fail("value required");
      s2_set(env, v->str[0], r);
      s2_visible = 0;
      return r;
    }
    w = s2_arg(args, -1, "mode", env, NULL);
    if (w &&
        (w->type != V_STR || w->length != 1 || strcmp(w->str[0], "function")))
      s2_fail("only function lookup mode is supported");
    r = w ? s2_function(env, v->str[0]) : s2_lookup(env, v->str[0]);
    if (!strcmp(name, "exists")) {
      r = s2_number(r != NULL);
      r->logical = 1;
      return r;
    }
    if (!r)
      s2_fail("object not found");
    return r;
  }
  if (!strcmp(name, "objects") || !strcmp(name, "ls")) {
    if (s2_arg(args, 0, "where", env, NULL) ||
        s2_arg(args, 1, "pattern", env, NULL))
      s2_fail("workspace filtering is not implemented");
    n = 0;
    for (b = s2_global.bindings; b; b = b->next)
      n++;
    if (s2_expression)
      for (b = s2_expression->bindings; b; b = b->next)
        if (!s2_binding(&s2_global, b->name))
          n++;
    r = s2_value(V_STR, n);
    i = 0;
    for (b = s2_global.bindings; b; b = b->next)
      r->str[i++] = b->name;
    if (s2_expression)
      for (b = s2_expression->bindings; b; b = b->next)
        if (!s2_binding(&s2_global, b->name))
          r->str[i++] = b->name;
    for (i = 1; i < n; i++)
      for (j = i; j > 0 && strcmp(r->str[j - 1], r->str[j]) > 0; j--) {
        char *t;
        t = r->str[j];
        r->str[j] = r->str[j - 1];
        r->str[j - 1] = t;
      }
    return r;
  }
  if (!strcmp(name, "remove") || !strcmp(name, "rm")) {
    for (p = args; p; p = p->next) {
      Binding **link;
      const char *nm;
      if (p->a && p->a->kind == N_NAME)
        nm = p->a->text;
      else {
        v = s2_eval(p->a, env);
        if (v->type != V_STR || v->length != 1)
          s2_fail("remove needs names");
        nm = v->str[0];
      }
      link = &s2_global.bindings;
      while (*link && strcmp((*link)->name, nm))
        link = &(*link)->next;
      if (*link)
        *link = (*link)->next;
      link = &env->bindings;
      while (*link && strcmp((*link)->name, nm))
        link = &(*link)->next;
      if (*link)
        *link = (*link)->next;
    }
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "structure")) {
    v = s2_arg(args, 0, ".Data", env, NULL);
    if (!v)
      s2_fail("structure needs data");
    r = s2_copy(v);
    for (p = args; p; p = p->next)
      if (p->text && strcmp(p->text, ".Data"))
        r = s2_with_attr(r, p->text, s2_eval(p->a, p->scope ? p->scope : env));
    return r;
  }
  v = s2_arg(args, 0, "x", env, NULL);
  if (!v && (!strcmp(name, "lapply") || !strcmp(name, "sapply") ||
             !strcmp(name, "apply") || !strcmp(name, "outer")))
    v = s2_arg(args, 0, "X", env, NULL);
  if (!v && !strcmp(name, "array"))
    v = s2_arg(args, 0, "data", env, s2_number(S2_NA));
  if (!v &&
      (!strcmp(name, "Re") || !strcmp(name, "Im") || !strcmp(name, "Mod") ||
       !strcmp(name, "Arg") || !strcmp(name, "Conj")))
    v = s2_arg(args, 0, "z", env, NULL);
  if (!v)
    return NULL;
  if (!strcmp(name, "attr")) {
    w = s2_arg(args, 1, "which", env, NULL);
    if (!w || w->type != V_STR || w->length != 1)
      s2_fail("attribute name required");
    return s2_attr(v, w->str[0]);
  }
  if (!strcmp(name, "names") || !strcmp(name, "dim") ||
      !strcmp(name, "dimnames") || !strcmp(name, "class") ||
      !strcmp(name, "tsp"))
    return s2_attr(v, name);
  if (!strcmp(name, "attributes")) {
    n = 0;
    for (attr = v->attrs; attr; attr = attr->next)
      n++;
    if (v->names)
      n++;
    if (v->rows && s2_attr(v, "dim")->type != V_NUM)
      n++;
    r = s2_value(V_LIST, n);
    r->names = (char **)s2_alloc((size_t)(n ? n : 1) * sizeof(char *));
    i = 0;
    for (attr = v->attrs; attr; attr = attr->next) {
      r->names[i] = attr->name;
      r->items[i++] = attr->value;
    }
    if (v->names) {
      r->names[i] = s2_strdup("names");
      r->items[i++] = s2_attr(v, "names");
    }
    return r;
  }
  if (!strncmp(name, "is.", 3) && strcmp(name, "is.na")) {
    flag = 0;
    if (!strcmp(name, "is.numeric"))
      flag = v->type == V_NUM && !v->logical;
    if (!strcmp(name, "is.logical"))
      flag = v->type == V_NUM && v->logical;
    if (!strcmp(name, "is.character"))
      flag = v->type == V_STR;
    if (!strcmp(name, "is.complex"))
      flag = v->type == V_COMPLEX;
    if (!strcmp(name, "is.list"))
      flag = v->type == V_LIST;
    if (!strcmp(name, "is.null"))
      flag = v->type == V_NULL;
    if (!strcmp(name, "is.function"))
      flag = v->type == V_FUNC;
    if (!strcmp(name, "is.matrix"))
      flag = v->rows != 0;
    if (!strcmp(name, "is.array"))
      flag = s2_attr(v, "dim")->length > 0;
    if (!strcmp(name, "is.vector"))
      flag = !v->attrs && !v->rows;
    if (!strcmp(name, "is.expression"))
      flag = v->type == V_EXPR;
    if (!strcmp(name, "is.call"))
      flag = v->type == V_LANG && v->body && v->body->kind == N_CALL;
    if (!strcmp(name, "is.name"))
      flag = v->type == V_LANG && v->body && v->body->kind == N_NAME;
    r = s2_number(flag);
    r->logical = 1;
    return r;
  }
  if (!strncmp(name, "as.", 3)) {
    if (!strcmp(name, "as.expression")) {
      r = s2_value(V_EXPR, v->length);
      for (i = 0; i < v->length; i++) {
        w = s2_element(v, i);
        r->items[i] = w->type == V_LANG ? w : language(constant(w));
      }
      return r;
    }
    if (!strcmp(name, "as.matrix") || !strcmp(name, "as.array")) {
      if (s2_attr(v, "dim")->length)
        return v;
      w = s2_value(V_NUM, 2);
      w->num[0] = v->length;
      w->num[1] = 1;
      return s2_with_attr(v, "dim", w);
    }
    type = !strcmp(name, "as.character") ? V_STR
           : !strcmp(name, "as.list")    ? V_LIST
           : !strcmp(name, "as.complex") ? V_COMPLEX
                                         : V_NUM;
    if (!strcmp(name, "as.vector")) {
      w = s2_arg(args, 1, "mode", env, s2_string("any"));
      if (!strcmp(w->str[0], "any")) {
        r = s2_copy(v);
        r->rows = r->cols = 0;
        r->attrs = NULL;
        return r;
      }
      type = !strcmp(w->str[0], "character") ? V_STR
             : !strcmp(w->str[0], "list")    ? V_LIST
                                             : V_NUM;
    }
    r = s2_coerce(v, type, !strcmp(name, "as.logical"));
    r->integer = !strcmp(name, "as.integer");
    if (r->integer)
      for (i = 0; i < r->length; i++)
        if (!s2_missing(r->num[i]))
          r->num[i] = r->num[i] < 0 ? ceil(r->num[i]) : floor(r->num[i]);
    return r;
  }
  if (!strcmp(name, "match") || !strcmp(name, "pmatch") ||
      !strcmp(name, "%in%")) {
    w = s2_arg(args, 1, "table", env, NULL);
    if (!w)
      s2_fail("table required");
    type = s2_common(v, w);
    v = s2_coerce(v, type, 0);
    w = s2_coerce(w, type, 0);
    r = s2_value(V_NUM, v->length);
    z = s2_arg(args, 2, "nomatch", env,
               s2_number(!strcmp(name, "%in%") ? 0 : S2_NA));
    if (z->type != V_NUM || z->length != 1)
      s2_fail("nomatch scalar required");
    x = z->num[0];
    for (i = 0; i < v->length; i++) {
      k = -1;
      n = 0;
      for (j = 0; j < w->length; j++)
        if (!compare(v, i, w, j)) {
          k = j;
          break;
        }
      if (k < 0 && !strcmp(name, "pmatch") && type == V_STR && v->str[i])
        for (j = 0; j < w->length; j++)
          if (w->str[j] && !strncmp(w->str[j], v->str[i], strlen(v->str[i]))) {
            k = j;
            n++;
          }
      if (n > 1)
        k = -1;
      r->num[i] = !strcmp(name, "%in%") ? k >= 0 : k >= 0 ? k + 1 : x;
    }
    r->logical = !strcmp(name, "%in%");
    return r;
  }
  if (!strcmp(name, "unique") || !strcmp(name, "duplicated")) {
    w = s2_value(V_NUM, v->length);
    w->logical = 1;
    n = 0;
    for (i = 0; i < v->length; i++) {
      for (j = 0; j < i; j++)
        if (!compare(v, i, v, j))
          break;
      w->num[i] = j < i;
      if (j == i)
        n++;
    }
    if (!strcmp(name, "duplicated"))
      return w;
    r = s2_value(v->type, n);
    r->logical = v->logical;
    k = 0;
    for (i = 0; i < v->length; i++)
      if (!w->num[i]) {
        z = s2_element(v, i);
        if (r->type == V_NUM || r->type == V_COMPLEX)
          r->num[k] = z->num[0];
        if (r->type == V_COMPLEX)
          r->imag[k] = z->imag[0];
        if (r->type == V_STR)
          r->str[k] = z->str[0];
        k++;
      }
    return r;
  }
  if (!strcmp(name, "rank") || !strcmp(name, "order")) {
    r = s2_value(V_NUM, v->length);
    for (i = 0; i < v->length; i++)
      r->num[i] = i;
    for (i = 1; i < v->length; i++)
      for (j = i;
           j > 0 && compare(v, (int)r->num[j - 1], v, (int)r->num[j]) > 0;
           j--) {
        x = r->num[j];
        r->num[j] = r->num[j - 1];
        r->num[j - 1] = x;
      }
    if (!strcmp(name, "order")) {
      for (i = 0; i < v->length; i++)
        r->num[i]++;
      return r;
    }
    w = s2_value(V_NUM, v->length);
    for (i = 0; i < v->length; i = j) {
      for (j = i + 1;
           j < v->length && !compare(v, (int)r->num[i], v, (int)r->num[j]); j++)
        ;
      for (k = i; k < j; k++)
        w->num[(int)r->num[k]] = (i + j + 1) / 2.0;
    }
    return w;
  }
  if (!strcmp(name, "array")) {
    w = s2_arg(args, 1, "dim", env, NULL);
    if (!w || w->type != V_NUM)
      s2_fail("array dimensions required");
    n = 1;
    for (i = 0; i < w->length; i++) {
      m = s2_count(s2_element(w, i));
      if ((long)n * m > S2_MAX_VECTOR)
        s2_fail("array too large");
      n *= m;
    }
    if (n && !v->length)
      s2_fail("empty array data");
    r = s2_value(v->type, n);
    r->logical = v->logical;
    for (i = 0; i < n; i++) {
      z = s2_element(v, i % v->length);
      if (v->type == V_NUM || v->type == V_COMPLEX)
        r->num[i] = z->num[0];
      if (v->type == V_COMPLEX)
        r->imag[i] = z->imag[0];
      if (v->type == V_STR)
        r->str[i] = z->str[0];
      if (v->type == V_LIST)
        r->items[i] = z;
    }
    r = s2_with_attr(r, "dim", w);
    w = s2_arg(args, 2, "dimnames", env, NULL);
    if (w)
      r = s2_with_attr(r, "dimnames", w);
    return r;
  }
  if (!strcmp(name, "nrow") || !strcmp(name, "ncol"))
    return v->rows ? s2_number(!strcmp(name, "nrow") ? v->rows : v->cols)
                   : s2_value(V_NULL, 0);
  if (!strcmp(name, "t")) {
    rows = v->rows ? v->rows : v->length;
    cols = v->rows ? v->cols : 1;
    r = s2_value(v->type, v->length);
    r->logical = v->logical;
    for (i = 0; i < rows; i++)
      for (j = 0; j < cols; j++) {
        k = i * cols + j;
        n = j * rows + i;
        if (v->type == V_NUM || v->type == V_COMPLEX)
          r->num[k] = v->num[n];
        if (v->type == V_COMPLEX)
          r->imag[k] = v->imag[n];
        if (v->type == V_STR)
          r->str[k] = v->str[n];
      }
    w = s2_value(V_NUM, 2);
    w->num[0] = cols;
    w->num[1] = rows;
    return s2_with_attr(r, "dim", w);
  }
  if (!strcmp(name, "lapply") || !strcmp(name, "sapply") ||
      !strcmp(name, "apply")) {
    fn = s2_arg(args, 1, "FUN", env, NULL);
    if (!fn)
      s2_fail("FUN required");
    rest = NULL;
    for (p = args; p; p = p->next)
      if (!p->text || (strcmp(p->text, "X") && strcmp(p->text, "FUN") &&
                       strcmp(p->text, "MARGIN"))) {
        Node *q;
        q = (Node *)s2_alloc(sizeof(Node));
        *q = *p;
        q->next = rest;
        rest = q;
      }
    rows = v->rows;
    cols = v->cols;
    axis = 0;
    n = v->length;
    if (!strcmp(name, "apply")) {
      if (!rows)
        s2_fail("apply needs matrix");
      axis = s2_count(s2_arg(args, 1, "MARGIN", env, NULL));
      if (axis != 1 && axis != 2)
        s2_fail("matrix margin must be 1 or 2");
      n = axis == 1 ? rows : cols;
    }
    r = s2_value(V_LIST, n);
    r->names = v->names;
    for (i = 0; i < n; i++) {
      if (axis) {
        w = s2_value(v->type, axis == 1 ? cols : rows);
        for (j = 0; j < w->length; j++) {
          k = axis == 1 ? j * rows + i : i * rows + j;
          if (v->type == V_NUM)
            w->num[j] = v->num[k];
          else if (v->type == V_STR)
            w->str[j] = v->str[k];
          else
            s2_fail("apply needs atomic matrix");
        }
      } else
        w = s2_element(v, i);
      r->items[i] = call_one(fn, w, rest, env);
    }
    if (!strcmp(name, "lapply") || !n)
      return r;
    m = r->items[0]->length;
    type = r->items[0]->type;
    for (i = 1; i < n; i++)
      if (r->items[i]->length != m || r->items[i]->type != type)
        return r;
    if ((long)m * n > S2_MAX_VECTOR)
      s2_fail("apply result too large");
    w = s2_value(type, m * n);
    w->logical = r->items[0]->logical;
    for (i = 0; i < n; i++)
      for (j = 0; j < m; j++) {
        k = i * m + j;
        if (type == V_NUM)
          w->num[k] = r->items[i]->num[j];
        else if (type == V_STR)
          w->str[k] = r->items[i]->str[j];
        else
          return r;
      }
    if (m > 1) {
      dim = s2_value(V_NUM, 2);
      dim->num[0] = m;
      dim->num[1] = n;
      w = s2_with_attr(w, "dim", dim);
    } else
      w->names = r->names;
    return w;
  }
  if (!strcmp(name, "nchar")) {
    v = s2_coerce(v, V_STR, 0);
    r = s2_value(V_NUM, v->length);
    for (i = 0; i < v->length; i++)
      r->num[i] = v->str[i] ? strlen(v->str[i]) : S2_NA;
    return r;
  }
  if (!strcmp(name, "substring") || !strcmp(name, "substr")) {
    v = s2_coerce(v, V_STR, 0);
    w = s2_arg(args, 1, !strcmp(name, "substr") ? "start" : "first", env,
               s2_number(1));
    z = s2_arg(args, 2, !strcmp(name, "substr") ? "stop" : "last", env,
               s2_number(255));
    r = s2_value(V_STR, v->length);
    for (i = 0; i < v->length; i++) {
      if (!v->str[i])
        continue;
      j = (int)w->num[i % w->length] - 1;
      k = (int)z->num[i % z->length];
      n = (int)strlen(v->str[i]);
      if (j < 0)
        j = 0;
      if (j > n)
        j = n;
      if (k > n)
        k = n;
      if (k < j)
        k = j;
      r->str[i] = (char *)s2_alloc((size_t)(k - j + 1));
      memcpy(r->str[i], v->str[i] + j, (size_t)(k - j));
    }
    return r;
  }
  if (!strcmp(name, "Re") || !strcmp(name, "Im") || !strcmp(name, "Mod") ||
      !strcmp(name, "Arg") || !strcmp(name, "Conj")) {
    if (v->type != V_NUM && v->type != V_COMPLEX)
      s2_fail("numeric or complex vector required");
    if (!strcmp(name, "Conj")) {
      r = s2_copy(v);
      if (v->type == V_COMPLEX)
        for (i = 0; i < v->length; i++)
          r->imag[i] = -r->imag[i];
      return r;
    }
    r = s2_value(V_NUM, v->length);
    for (i = 0; i < v->length; i++) {
      x = v->num[i];
      y = v->type == V_COMPLEX ? v->imag[i] : 0;
      r->num[i] = s2_missing(x)          ? S2_NA
                  : !strcmp(name, "Re")  ? x
                  : !strcmp(name, "Im")  ? y
                  : !strcmp(name, "Mod") ? sqrt(x * x + y * y)
                                         : atan2(y, x);
    }
    return r;
  }
  (void)buf;
  return NULL;
}

const char *s2_signature(const char *name) {
  int i;
  for (i = 0; signatures[i].name; i++)
    if (!strcmp(name, signatures[i].name))
      return signatures[i].signature;
  return NULL;
}
