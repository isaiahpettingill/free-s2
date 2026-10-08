#include "s2.h"
static FILE *device;
static int kind, active, page;
static double width = 640, height = 480, xmin, xmax, ymin, ymax;
static char filename[256];
static const char *color(Value *v) {
  static const char *palette[] = {"white", "black", "red",     "green",
                                  "blue",  "cyan",  "magenta", "yellow"};
  int i;
  if (v->type == V_STR && v->length && v->str[0])
    return v->str[0];
  i = (int)s2_scalar(v);
  return palette[i >= 0 && i < 8 ? i : 1];
}
static void escaped(const char *s) {
  unsigned char c;
  if (!s)
    s2_fail("missing graphics label");
  while (*s) {
    c = (unsigned char)*s++;
    if (kind == 1) {
      if (c == '&')
        fputs("&amp;", device);
      else if (c == '<')
        fputs("&lt;", device);
      else if (c == '>')
        fputs("&gt;", device);
      else if (c == '"')
        fputs("&quot;", device);
      else if (c >= 32 || c == '\n')
        fputc(c, device);
    } else {
      if (c == '(' || c == ')' || c == '\\')
        fputc('\\', device);
      if (c < 32 || c > 126)
        fprintf(device, "\\%03o", (unsigned)c);
      else
        fputc(c, device);
    }
  }
}
static double px(double x) {
  return 65 + (x - xmin) / (xmax - xmin) * (width - 100);
}
static double py(double y) {
  return height - 55 - (y - ymin) / (ymax - ymin) * (height - 110);
}
static void label(double x, double y, const char *s, int center) {
  if (kind == 1) {
    fprintf(device,
            "<text x=\"%.6g\" y=\"%.6g\" font-family=\"sans-serif\" "
            "font-size=\"12\" text-anchor=\"%s\">",
            x, y, center ? "middle" : "start");
    escaped(s);
    fputs("</text>\n", device);
  } else {
    fprintf(device, "%.6g %.6g moveto (", x, height - y);
    escaped(s);
    fputs(center ? ") dup stringwidth pop -2 div 0 rmoveto show\n" : ") show\n",
          device);
  }
}
static void segment(double x1, double y1, double x2, double y2, const char *col,
                    int clip) {
  if (kind == 1) {
    fprintf(device,
            "<line x1=\"%.6g\" y1=\"%.6g\" x2=\"%.6g\" y2=\"%.6g\" stroke=\"",
            x1, y1, x2, y2);
    escaped(col);
    fprintf(device, "\"%s/>\n", clip ? " clip-path=\"url(#panel)\"" : "");
  } else {
    (void)col;
    if (clip)
      fprintf(device,
              "gsave newpath 65 55 moveto %.6g 0 rlineto 0 %.6g rlineto %.6g 0 "
              "rlineto closepath clip\n",
              width - 100, height - 110, 100 - width);
    fprintf(device, "newpath %.6g %.6g moveto %.6g %.6g lineto stroke\n", x1,
            height - y1, x2, height - y2);
    if (clip)
      fputs("grestore\n", device);
  }
}
static void point(double x, double y, const char *col) {
  if (kind == 1) {
    fprintf(device,
            "<circle cx=\"%.6g\" cy=\"%.6g\" r=\"3\" fill=\"none\" stroke=\"",
            px(x), py(y));
    escaped(col);
    fputs("\" clip-path=\"url(#panel)\"/>\n", device);
  } else
    fprintf(device, "newpath %.6g %.6g 3 0 360 arc stroke\n", px(x),
            height - py(y));
}
static void rectangle(double x1, double y1, double x2, double y2,
                      const char *col) {
  double a, b, c, d;
  a = px(x1);
  b = px(x2);
  c = py(y2);
  d = py(y1);
  if (kind == 1) {
    fprintf(
        device,
        "<rect x=\"%.6g\" y=\"%.6g\" width=\"%.6g\" height=\"%.6g\" fill=\"", a,
        c, b - a, d - c);
    escaped(col);
    fputs("\" stroke=\"black\" clip-path=\"url(#panel)\"/>\n", device);
  } else
    fprintf(device,
            "newpath %.6g %.6g moveto %.6g 0 rlineto 0 %.6g rlineto %.6g 0 "
            "rlineto closepath gsave .85 setgray fill grestore stroke\n",
            a, height - d, b - a, d - c, a - b);
}
void s2_graphics_close(void) {
  int failed;
  if (!device)
    return;
  if (kind == 1 && active)
    fputs("</svg>\n", device);
  if (kind == 2) {
    if (active)
      fputs("showpage\n", device);
    fprintf(device, "%%%%Trailer\n%%%%Pages: %d\n%%%%EOF\n", page);
  }
  failed = ferror(device);
  if (fclose(device))
    failed = 1;
  device = NULL;
  active = 0;
  if (failed)
    s2_fail("graphics write failed");
}
static void open_device(const char *file, int type, double w, double h) {
  if (strlen(file) > 255 || !*file)
    s2_fail("graphics filename required");
  if (w < 160 || h < 140 || w > 10000 || h > 10000)
    s2_fail("invalid graphics size");
  s2_graphics_close();
  strcpy(filename, file);
  kind = type;
  width = w;
  height = h;
  page = active = 0;
  device = fopen(file, "w");
  if (!device)
    s2_fail("cannot open graphics file");
  if (kind == 2)
    fprintf(device,
            "%%!PS-Adobe-3.0\n%%%%BoundingBox: 0 0 %.0f %.0f\n%%%%Pages: "
            "(atend)\n/Helvetica findfont 12 scalefont setfont\n",
            width, height);
}
static void panel(double xl, double xh, double yl, double yh,
                  const char *main) {
  int i;
  char buf[60];
  double x, y;
  if (!device)
    open_device("S2PLOT.SVG", 1, 640, 480);
  if (xl == xh) {
    xl -= .5;
    xh += .5;
  }
  if (yl == yh) {
    yl -= .5;
    yh += .5;
  }
  xmin = xl;
  xmax = xh;
  ymin = yl;
  ymax = yh;
  if (kind == 1) {
    if (active) {
      fclose(device);
      device = fopen(filename, "w");
      if (!device)
        s2_fail("cannot start new plot");
    }
    fprintf(device,
            "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%.0f\" "
            "height=\"%.0f\" viewBox=\"0 0 %.0f %.0f\">\n<rect width=\"100%%\" "
            "height=\"100%%\" fill=\"white\"/>\n<defs><clipPath "
            "id=\"panel\"><rect x=\"65\" y=\"55\" width=\"%.6g\" "
            "height=\"%.6g\"/></clipPath></defs>\n",
            width, height, width, height, width - 100, height - 110);
  } else {
    if (active)
      fputs("showpage\n", device);
    fprintf(device, "%%%%Page: %d %d\n", page + 1, page + 1);
  }
  active = 1;
  page++;
  segment(65, 55, width - 35, 55, "black", 0);
  segment(65, height - 55, width - 35, height - 55, "black", 0);
  segment(65, 55, 65, height - 55, "black", 0);
  segment(width - 35, 55, width - 35, height - 55, "black", 0);
  for (i = 0; i <= 4; i++) {
    x = xl + (xh - xl) * i / 4;
    y = yl + (yh - yl) * i / 4;
    segment(px(x), height - 55, px(x), height - 50, "black", 0);
    sprintf(buf, "%.4g", x);
    label(px(x), height - 32, buf, 1);
    segment(60, py(y), 65, py(y), "black", 0);
    sprintf(buf, "%.4g", y);
    label(5, py(y) + 4, buf, 0);
  }
  if (main && *main)
    label(width / 2, 25, main, 1);
}
static int valid(double x) { return !s2_missing(x) && fabs(x) <= DBL_MAX; }
static const char *title_value(Node *args, Env *env) {
  Value *v;
  v = s2_arg(args, -1, "main", env, s2_string(""));
  if (v->type != V_STR || v->length != 1)
    s2_fail("main must be string");
  return v->str[0] ? v->str[0] : "";
}
static void xy(Value *v, Value *w, Value **x, Value **y) {
  int i;
  if (!w && v->type == V_LIST && v->names) {
    for (i = 0; i < v->length; i++) {
      if (v->names[i] && !strcmp(v->names[i], "x"))
        *x = v->items[i];
      if (v->names[i] && !strcmp(v->names[i], "y"))
        *y = v->items[i];
    }
    if (*x && *y)
      return;
  }
  if (!w && v->rows && v->cols == 2) {
    *x = s2_value(V_NUM, v->rows);
    *y = s2_value(V_NUM, v->rows);
    for (i = 0; i < v->rows; i++) {
      (*x)->num[i] = v->num[i];
      (*y)->num[i] = v->num[v->rows + i];
    }
  } else if (!w) {
    *y = v;
    *x = s2_value(V_NUM, v->length);
    for (i = 0; i < v->length; i++)
      (*x)->num[i] = i + 1;
  } else {
    *x = v;
    *y = w;
  }
  if ((*x)->type != V_NUM || (*y)->type != V_NUM ||
      (*x)->length != (*y)->length)
    s2_fail("equal-length numeric coordinates required");
}
static void ranges(Value *x, Value *y, double *xl, double *xh, double *yl,
                   double *yh) {
  int i, found;
  found = 0;
  for (i = 0; i < x->length; i++)
    if (valid(x->num[i]) && valid(y->num[i])) {
      if (!found) {
        *xl = *xh = x->num[i];
        *yl = *yh = y->num[i];
        found = 1;
      } else {
        if (x->num[i] < *xl)
          *xl = x->num[i];
        if (x->num[i] > *xh)
          *xh = x->num[i];
        if (y->num[i] < *yl)
          *yl = y->num[i];
        if (y->num[i] > *yh)
          *yh = y->num[i];
      }
    }
  if (!found)
    s2_fail("no finite points to plot");
}
static void draw_xy(Value *x, Value *y, char type, const char *col) {
  int i;
  if (!strchr("plbonhsS", type))
    s2_fail("unsupported plot type");
  for (i = 0; i < x->length; i++)
    if (valid(x->num[i]) && valid(y->num[i])) {
      if (strchr("pbo", type))
        point(x->num[i], y->num[i], col);
      if (type == 'h')
        segment(px(x->num[i]), py(0), px(x->num[i]), py(y->num[i]), col, 1);
      if (i && strchr("lbosS", type) && valid(x->num[i - 1]) &&
          valid(y->num[i - 1])) {
        if (type == 's' || type == 'S') {
          double z;
          z = type == 's' ? y->num[i - 1] : y->num[i];
          segment(px(x->num[i - 1]), py(y->num[i - 1]), px(x->num[i]), py(z),
                  col, 1);
          segment(px(x->num[i]), py(z), px(x->num[i]), py(y->num[i]), col, 1);
        } else
          segment(px(x->num[i - 1]), py(y->num[i - 1]), px(x->num[i]),
                  py(y->num[i]), col, 1);
      }
    }
}
Value *s2_graphics(const char *name, Node *args, Env *env) {
  Value *v, *w, *x, *y, *r, *a, *b, *c, *counts, *breaks, *labels;
  Node *p;
  const char *col;
  double xl, xh, yl, yh, z, h;
  int i, j, n, m, plot;
  if (!strcmp(name, "svg") || !strcmp(name, "postscript")) {
    v = s2_arg(args, 0, "file", env,
               s2_string(!strcmp(name, "svg") ? "S2PLOT.SVG" : "S2PLOT.PS"));
    if (v->type != V_STR || v->length != 1 || !v->str[0])
      s2_fail("graphics filename required");
    xl = s2_scalar(s2_arg(args, 1, "width", env,
                          s2_number(!strcmp(name, "svg") ? 640 : 7)));
    yl = s2_scalar(s2_arg(args, 2, "height", env,
                          s2_number(!strcmp(name, "svg") ? 480 : 5)));
    open_device(v->str[0], !strcmp(name, "svg") ? 1 : 2,
                !strcmp(name, "svg") ? xl : xl * 72,
                !strcmp(name, "svg") ? yl : yl * 72);
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "dev.off") || !strcmp(name, "graphics.off")) {
    s2_graphics_close();
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "plot") || !strcmp(name, "lines") ||
      !strcmp(name, "points")) {
    v = s2_arg(args, 0, "x", env, NULL);
    w = s2_arg(args, 1, "y", env, NULL);
    if (!v)
      s2_fail("plot data required");
    x = y = NULL;
    xy(v, w, &x, &y);
    col = color(s2_arg(args, -1, "col", env, s2_number(1)));
    a = s2_arg(args, -1, "type", env,
               s2_string(!strcmp(name, "lines") ? "l" : "p"));
    if (a->type != V_STR || !a->length || !a->str[0])
      s2_fail("plot type string required");
    if (!strcmp(name, "plot")) {
      ranges(x, y, &xl, &xh, &yl, &yh);
      a = s2_arg(args, -1, "xlim", env, NULL);
      if (a) {
        if (a->type != V_NUM || a->length != 2 || !valid(a->num[0]) ||
            !valid(a->num[1]) || a->num[0] >= a->num[1])
          s2_fail("invalid xlim");
        xl = a->num[0];
        xh = a->num[1];
      }
      a = s2_arg(args, -1, "ylim", env, NULL);
      if (a) {
        if (a->type != V_NUM || a->length != 2 || !valid(a->num[0]) ||
            !valid(a->num[1]) || a->num[0] >= a->num[1])
          s2_fail("invalid ylim");
        yl = a->num[0];
        yh = a->num[1];
      }
      panel(xl, xh, yl, yh, title_value(args, env));
      a = s2_arg(args, -1, "xlab", env, NULL);
      if (a && a->type == V_STR)
        label(width / 2, height - 7, a->str[0], 1);
      a = s2_arg(args, -1, "ylab", env, NULL);
      if (a && a->type == V_STR)
        label(65, 43, a->str[0], 0);
    } else if (!active)
      s2_fail("plot must establish axes first");
    a = s2_arg(args, -1, "type", env,
               s2_string(!strcmp(name, "lines") ? "l" : "p"));
    if (a->type != V_STR || a->length != 1 || !a->str[0] || !a->str[0][0])
      s2_fail("plot type must be character scalar");
    draw_xy(x, y, a->str[0][0], col);
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "abline")) {
    if (!active)
      s2_fail("plot required before abline");
    col = color(s2_arg(args, -1, "col", env, s2_number(1)));
    a = s2_arg(args, 0, "a", env, NULL);
    b = s2_arg(args, 1, "b", env, NULL);
    if (a && a->type == V_LIST) {
      for (i = 0; i < a->length; i++)
        if (a->names && a->names[i] && !strcmp(a->names[i], "coef")) {
          a = a->items[i];
          break;
        }
    }
    if (a && a->type == V_NUM && a->length == 2 && !b) {
      b = s2_element(a, 1);
      a = s2_element(a, 0);
    }
    if (a && b) {
      yl = s2_scalar(a);
      yh = s2_scalar(b);
      segment(px(xmin), py(yl + yh * xmin), px(xmax), py(yl + yh * xmax), col,
              1);
    }
    a = s2_arg(args, -1, "h", env, NULL);
    if (a)
      for (i = 0; i < a->length; i++)
        segment(px(xmin), py(a->num[i]), px(xmax), py(a->num[i]), col, 1);
    a = s2_arg(args, -1, "v", env, NULL);
    if (a)
      for (i = 0; i < a->length; i++)
        segment(px(a->num[i]), py(ymin), px(a->num[i]), py(ymax), col, 1);
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "title")) {
    if (!active)
      s2_fail("plot required before title");
    label(width / 2, 25, title_value(args, env), 1);
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "text")) {
    if (!active)
      s2_fail("plot required before text");
    x = s2_arg(args, 0, "x", env, NULL);
    y = s2_arg(args, 1, "y", env, NULL);
    labels = s2_arg(args, 2, "labels", env, NULL);
    if (!x || !y || !labels || x->type != V_NUM || y->type != V_NUM ||
        x->length != y->length)
      s2_fail("text coordinates and labels required");
    labels = s2_coerce(labels, V_STR, 0);
    if (!labels->length)
      s2_fail("empty labels");
    for (i = 0; i < x->length; i++)
      if (valid(x->num[i]) && valid(y->num[i]))
        label(px(x->num[i]), py(y->num[i]),
              labels->str[i % labels->length] ? labels->str[i % labels->length]
                                              : "NA",
              1);
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "barplot")) {
    v = s2_arg(args, 0, "height", env, NULL);
    if (!v || v->type != V_NUM || !v->length)
      s2_fail("numeric bar heights required");
    yl = yh = 0;
    for (i = 0; i < v->length; i++) {
      if (!valid(v->num[i]))
        s2_fail("missing bar height");
      if (v->num[i] < yl)
        yl = v->num[i];
      if (v->num[i] > yh)
        yh = v->num[i];
    }
    panel(0, v->length, yl, yh, title_value(args, env));
    col = color(s2_arg(args, -1, "col", env, s2_number(4)));
    r = s2_value(V_NUM, v->length);
    for (i = 0; i < v->length; i++) {
      rectangle(i + .1, v->num[i] < 0 ? v->num[i] : 0, i + .9,
                v->num[i] < 0 ? 0 : v->num[i], col);
      r->num[i] = i + .5;
    }
    s2_visible = 0;
    return r;
  }
  if (!strcmp(name, "hist")) {
    v = s2_arg(args, 0, "x", env, NULL);
    if (!v || v->type != V_NUM || !v->length)
      s2_fail("numeric histogram data required");
    xl = xh = v->num[0];
    for (i = 0; i < v->length; i++) {
      if (!valid(v->num[i]))
        s2_fail("missing histogram data");
      if (v->num[i] < xl)
        xl = v->num[i];
      if (v->num[i] > xh)
        xh = v->num[i];
    }
    breaks = s2_arg(args, 2, "breaks", env, NULL);
    if (!breaks) {
      m = s2_count(
          s2_arg(args, 1, "nclass", env,
                 s2_number(ceil(log((double)v->length) / log(2.0)) + 1)));
      if (!m || m >= S2_MAX_VECTOR)
        s2_fail("invalid histogram classes");
      if (xh == xl) {
        xl -= .5;
        xh += .5;
      }
      h = (xh - xl) / m;
      xl -= h * 1e-7;
      breaks = s2_value(V_NUM, m + 1);
      for (i = 0; i <= m; i++)
        breaks->num[i] = xl + i * (xh - xl) / m;
    }
    if (breaks->type != V_NUM || breaks->length < 2)
      s2_fail("numeric histogram breaks required");
    m = breaks->length - 1;
    for (i = 0; i < m; i++)
      if (breaks->num[i] >= breaks->num[i + 1])
        s2_fail("breaks must increase");
    counts = s2_value(V_NUM, m);
    for (i = 0; i < v->length; i++)
      for (j = 0; j < m; j++)
        if (v->num[i] > breaks->num[j] && v->num[i] <= breaks->num[j + 1]) {
          counts->num[j]++;
          break;
        }
    plot = s2_scalar(s2_arg(args, 3, "plot", env, s2_number(1))) != 0;
    if (plot) {
      yh = 1;
      for (i = 0; i < m; i++)
        if (counts->num[i] > yh)
          yh = counts->num[i];
      panel(breaks->num[0], breaks->num[m], 0, yh, title_value(args, env));
      col = color(s2_arg(args, -1, "col", env, s2_number(4)));
      for (i = 0; i < m; i++)
        rectangle(breaks->num[i], 0, breaks->num[i + 1], counts->num[i], col);
      s2_visible = 0;
    }
    r = s2_value(V_LIST, 2);
    r->names = (char **)s2_alloc(2 * sizeof(char *));
    r->names[0] = s2_strdup("counts");
    r->items[0] = counts;
    r->names[1] = s2_strdup("breaks");
    r->items[1] = breaks;
    return r;
  }
  if (!strcmp(name, "qqplot") || !strcmp(name, "qqnorm")) {
    x = s2_arg(args, 0, !strcmp(name, "qqnorm") ? "y" : "x", env, NULL);
    y = s2_arg(args, 1, "y", env, NULL);
    if (!x || x->type != V_NUM || !x->length)
      s2_fail("numeric Q-Q data required");
    x = s2_copy(x);
    for (i = 1; i < x->length; i++)
      for (j = i; j > 0 && x->num[j] < x->num[j - 1]; j--) {
        z = x->num[j];
        x->num[j] = x->num[j - 1];
        x->num[j - 1] = z;
      }
    if (!strcmp(name, "qqnorm")) {
      y = x;
      x = s2_value(V_NUM, y->length);
      for (i = 0; i < x->length; i++) {
        Node arg, expr;
        memset(&arg, 0, sizeof(arg));
        memset(&expr, 0, sizeof(expr));
        expr.kind = N_VALUE;
        expr.value = s2_number((i + .5) / x->length);
        arg.a = &expr;
        x->num[i] = s2_builtin("qnorm", &arg, env)->num[0];
      }
    } else {
      if (!y || y->type != V_NUM || y->length != x->length)
        s2_fail("qqplot currently needs equal-length samples");
      y = s2_copy(y);
      for (i = 1; i < y->length; i++)
        for (j = i; j > 0 && y->num[j] < y->num[j - 1]; j--) {
          z = y->num[j];
          y->num[j] = y->num[j - 1];
          y->num[j - 1] = z;
        }
    }
    ranges(x, y, &xl, &xh, &yl, &yh);
    panel(xl, xh, yl, yh, title_value(args, env));
    draw_xy(x, y, 'p', "black");
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  if (!strcmp(name, "boxplot")) {
    Value *sets[S2_MAX_ARGS], *quarts[S2_MAX_ARGS];
    n = 0;
    yl = yh = 0;
    for (p = args; p; p = p->next)
      if (!p->text || strcmp(p->text, "main")) {
        Node arg, expr;
        if (n == S2_MAX_ARGS)
          s2_fail("too many boxes");
        v = s2_eval(p->a, p->scope ? p->scope : env);
        if (v->type != V_NUM || !v->length)
          s2_fail("numeric boxplot data required");
        sets[n] = v;
        memset(&arg, 0, sizeof(arg));
        memset(&expr, 0, sizeof(expr));
        expr.kind = N_VALUE;
        expr.value = v;
        arg.a = &expr;
        quarts[n] = s2_builtin("quantile", &arg, env);
        if (!n)
          yl = yh = v->num[0];
        for (i = 0; i < v->length; i++) {
          if (v->num[i] < yl)
            yl = v->num[i];
          if (v->num[i] > yh)
            yh = v->num[i];
        }
        n++;
      }
    if (!n)
      s2_fail("boxplot data required");
    panel(0, n, yl, yh, title_value(args, env));
    for (j = 0; j < n; j++) {
      v = quarts[j];
      h = 1.5 * (v->num[3] - v->num[1]);
      xl = v->num[1];
      xh = v->num[3];
      for (i = 0; i < sets[j]->length; i++) {
        z = sets[j]->num[i];
        if (z >= v->num[1] - h && z < xl)
          xl = z;
        if (z <= v->num[3] + h && z > xh)
          xh = z;
      }
      rectangle(j + .25, v->num[1], j + .75, v->num[3], "white");
      segment(px(j + .25), py(v->num[2]), px(j + .75), py(v->num[2]), "black",
              1);
      segment(px(j + .5), py(xl), px(j + .5), py(v->num[1]), "black", 1);
      segment(px(j + .5), py(v->num[3]), px(j + .5), py(xh), "black", 1);
      for (i = 0; i < sets[j]->length; i++)
        if (sets[j]->num[i] < xl || sets[j]->num[i] > xh)
          point(j + .5, sets[j]->num[i], "black");
    }
    s2_visible = 0;
    return s2_value(V_NULL, 0);
  }
  (void)a;
  (void)b;
  (void)c;
  return NULL;
}
