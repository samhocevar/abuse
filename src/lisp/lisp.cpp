/*
 *  Abuse — dark 2D side-scrolling platform game
 *  Copyright © 1995 Crack dot Com
 *  Copyright © 2005—2016 Sam Hocevar <sam@hocevar.net>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Crack dot Com, by
 *  Jonathan Clark, or by Sam Hocevar.
 */

#if HAVE_CONFIG_H
#   include "config.h"
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"

#define TYPE_CHECKING 1

#include "lisp/lisp.h"
#include "lisp/gc.h"
#include "lisp/symbols.h"

#ifdef NO_LIBS
#   include "fakelib.h"
#else
#   include "imlib/status.h"
#   include "imlib/specs.h"
#   include "imlib/dprint.h"
#   include "cache.h"
#   include "dev.h"
#endif

#if TYPE_CHECKING
#   define ASSERT_TYPE(obj, type, message) \
        if (item_type(obj) != type) \
        { \
            lisp::print(obj); \
            lbreak(": %s\n", message); \
            exit(0); \
        }
#else
#   define ASSERT_TYPE(obj, type, message)
#endif

/* select, digistr, load-file are not common lisp functions! */

struct func sys_funcs[] =
{
    { "print", 1, -1 }, /* 0 */
    { "car", 1, 1 }, /* 1 */
    { "cdr", 1, 1 }, /* 2 */
    { "length", 0, -1 }, /* 3 */
    { "list", 0, -1 }, /* 4 */
    { "cons", 2, 2 }, /* 5 */
    { "quote", 1, 1 }, /* 6 */
    { "eq", 2, 2 }, /* 7 */
    { "+", 0, -1 }, /* 8 */
    { "-", 1, -1 }, /* 9 */
    { "if", 2, 3 }, /* 10 */
    { "setf", 2, 2 }, /* 11 */
    { "symbol-list", 0, 0 }, /* 12 */
    { "assoc", 2, 2 }, /* 13 */
    { "null", 1, 1 }, /* 14 */
    { "acons", 2, 2 }, /* 15 */
    { "pairlis", 2, 2 }, /* 16 */
    { "let", 1, -1 }, /* 17 */
    { "defun", 2, -1 }, /* 18 */
    { "atom", 1, 1 }, /* 19 */
    { "not", 1, 1 }, /* 20 */
    { "and", -1, -1 }, /* 21 */
    { "or", -1, -1 }, /* 22 */
    { "progn", -1, -1 }, /* 23 */
    { "equal", 2, 2 }, /* 24 */
    { "concatenate", 1, -1 }, /* 25 */
    { "char-code", 1, 1 }, /* 26 */
    { "code-char", 1, 1 }, /* 27 */
    { "*", -1, -1 }, /* 28 */
    { "/", 1, -1 }, /* 29 */
    { "cond", -1, -1 }, /* 30 */
    { "select", 1, -1 }, /* 31 */
    { "function", 1, 1 }, /* 32 */
    { "mapcar", 2, -1 }, /* 33 */
    { "funcall", 1, -1 }, /* 34 */
    { ">", 2, 2 }, /* 35 */
    { "<", 2, 2 }, /* 36 */
    { "tmp-space", 0, 0 }, /* 37 */
    { "perm-space", 0, 0 }, /* 38 */
    { "symbol-name", 1, 1 }, /* 39 */
    { "trace", 0, -1 }, /* 40 */
    { "untrace", 0, -1 }, /* 41 */
    { "digstr", 2, 2 }, /* 42 */
    { "compile-file", 1, 1 }, /* 43 */
    { "abs", 1, 1 }, /* 44 */
    { "min", 2, 2 }, /* 45 */
    { "max", 2, 2 }, /* 46 */
    { ">=", 2, 2 }, /* 47 */
    { "<=", 2, 2 }, /* 48 */
    { "backquote", 1, 1 }, /* 49 */
    { "comma", 1, 1 }, /* 50 */
    { "nth", 2, 2 }, /* 51 */
    { "resize-tmp", 1, 1 }, /* 52 */
    { "resize-perm", 1, 1 }, /* 53 */
    { "cos", 1, 1 }, /* 54 */
    { "sin", 1, 1 }, /* 55 */
    { "atan2", 2, 2 }, /* 56 */
    { "enum", 1, -1 }, /* 57 */
    { "quit", 0, 0 }, /* 58 */
    { "eval", 1, 1 }, /* 59 */
    { "break", 0, 0 }, /* 60 */
    { "mod", 2, 2 }, /* 61 */
    { "write_profile", 1, 1 }, /* 62 */
    { "setq", 2, 2 }, /* 63 */
    { "for", 4, -1 }, /* 64 */
    { "open_file", 2, -1 }, /* 65 */
    { "load", 1, 1 }, /* 66 */
    { "bit-and", 1, -1 }, /* 67 */
    { "bit-or", 1, -1 }, /* 68 */
    { "bit-xor", 1, -1 }, /* 69 */
    { "make-array", 1, -1 }, /* 70 */
    { "aref", 2, 2 }, /* 71 */
    { "if-1progn", 2, 3 }, /* 72 */
    { "if-2progn", 2, 3 }, /* 73 */
    { "if-12progn", 2, 3 }, /* 74 */
    { "eq0", 1, 1 }, /* 75 */
    { "preport", 1, 1 }, /* 76 */
    { "search", 2, 2 }, /* 77 */
    { "elt", 2, 2 }, /* 78 */
    { "listp", 1, 1 }, /* 79 */
    { "numberp", 1, 1 }, /* 80 */
    { "do", 2, 3 }, /* 81 */
    { "gc", 0, 0 }, /* 82 */
    { "schar", 2, 2 }, /* 83 */
    { "symbolp", 1, 1 }, /* 84 */
    { "num2str", 1, 1 }, /* 85 */
    { "nconc", 2, -1 }, /* 86 */
    { "first", 1, 1 }, /* 87 */
    { "second", 1, 1 }, /* 88 */
    { "third", 1, 1 }, /* 89 */
    { "fourth", 1, 1 }, /* 90 */
    { "fifth", 1, 1 }, /* 91 */
    { "sixth", 1, 1 }, /* 92 */
    { "seventh", 1, 1 }, /* 93 */
    { "eighth", 1, 1 }, /* 94 */
    { "ninth", 1, 1 }, /* 95 */
    { "tenth", 1, 1 }, /* 96 */
    { "substr", 3, 3 }, /* 97 */
    { "local_load", 1, 1 }, /* 98 */
};

enum sys_func_index
{
    SYS_FUNC_PRINT = 0,
    SYS_FUNC_CAR = 1,
    SYS_FUNC_CDR = 2,
    SYS_FUNC_LENGTH = 3,
    SYS_FUNC_LIST = 4,
    SYS_FUNC_CONS = 5,
    SYS_FUNC_QUOTE = 6,
    SYS_FUNC_EQ = 7,
    SYS_FUNC_PLUS = 8,
    SYS_FUNC_MINUS = 9,
    SYS_FUNC_IF = 10,
    SYS_FUNC_SETF = 11,
    SYS_FUNC_SYMBOL_LIST = 12,
    SYS_FUNC_ASSOC = 13,
    SYS_FUNC_NULL = 14,
    SYS_FUNC_ACONS = 15,
    SYS_FUNC_PAIRLIS = 16,
    SYS_FUNC_LET = 17,
    SYS_FUNC_DEFUN = 18,
    SYS_FUNC_ATOM = 19,
    SYS_FUNC_NOT = 20,
    SYS_FUNC_AND = 21,
    SYS_FUNC_OR = 22,
    SYS_FUNC_PROGN = 23,
    SYS_FUNC_EQUAL = 24,
    SYS_FUNC_CONCATENATE = 25,
    SYS_FUNC_CHAR_CODE = 26,
    SYS_FUNC_CODE_CHAR = 27,
    SYS_FUNC_TIMES = 28,
    SYS_FUNC_SLASH = 29,
    SYS_FUNC_COND = 30,
    SYS_FUNC_SELECT = 31,
    SYS_FUNC_FUNCTION = 32,
    SYS_FUNC_MAPCAR = 33,
    SYS_FUNC_FUNCALL = 34,
    SYS_FUNC_GT = 35,
    SYS_FUNC_LT = 36,
    SYS_FUNC_TMP_SPACE = 37,
    SYS_FUNC_PERM_SPACE = 38,
    SYS_FUNC_SYMBOL_NAME = 39,
    SYS_FUNC_TRACE = 40,
    SYS_FUNC_UNTRACE = 41,
    SYS_FUNC_DIGSTR = 42,
    SYS_FUNC_COMPILE_FILE = 43,
    SYS_FUNC_ABS = 44,
    SYS_FUNC_MIN = 45,
    SYS_FUNC_MAX = 46,
    SYS_FUNC_GE = 47,
    SYS_FUNC_LE = 48,
    SYS_FUNC_BACKQUOTE = 49,
    SYS_FUNC_COMMA = 50,
    SYS_FUNC_NTH = 51,
    SYS_FUNC_RESIZE_TMP = 52,
    SYS_FUNC_RESIZE_PERM = 53,
    SYS_FUNC_COS = 54,
    SYS_FUNC_SIN = 55,
    SYS_FUNC_ATAN2 = 56,
    SYS_FUNC_ENUM = 57,
    SYS_FUNC_QUIT = 58,
    SYS_FUNC_EVAL = 59,
    SYS_FUNC_BREAK = 60,
    SYS_FUNC_MOD = 61,
    SYS_FUNC_WRITE_PROFILE = 62,
    SYS_FUNC_SETQ = 63,
    SYS_FUNC_FOR = 64,
    SYS_FUNC_OPEN_FILE = 65,
    SYS_FUNC_LOAD = 66,
    SYS_FUNC_BIT_AND = 67,
    SYS_FUNC_BIT_OR = 68,
    SYS_FUNC_BIT_XOR = 69,
    SYS_FUNC_MAKE_ARRAY = 70,
    SYS_FUNC_AREF = 71,
    SYS_FUNC_IF_1PROGN = 72,
    SYS_FUNC_IF_2PROGN = 73,
    SYS_FUNC_IF_12PROGN = 74,
    SYS_FUNC_EQ0 = 75,
    SYS_FUNC_PREPORT = 76,
    SYS_FUNC_SEARCH = 77,
    SYS_FUNC_ELT = 78,
    SYS_FUNC_LISTP = 79,
    SYS_FUNC_NUMBERP = 80,
    SYS_FUNC_DO = 81,
    SYS_FUNC_GC = 82,
    SYS_FUNC_SCHAR = 83,
    SYS_FUNC_SYMBOLP = 84,
    SYS_FUNC_NUM2STR = 85,
    SYS_FUNC_NCONC = 86,
    SYS_FUNC_FIRST = 87,
    SYS_FUNC_SECOND = 88,
    SYS_FUNC_THIRD = 89,
    SYS_FUNC_FOURTH = 90,
    SYS_FUNC_FIFTH = 91,
    SYS_FUNC_SIXTH = 92,
    SYS_FUNC_SEVENTH = 93,
    SYS_FUNC_EIGHTH = 94,
    SYS_FUNC_NINTH = 95,
    SYS_FUNC_TENTH = 96,
    SYS_FUNC_SUBSTR = 97,
    SYS_FUNC_LOCAL_LOAD = 98,
};
/* To bypass the whole garbage collection issue of lisp I am going to have
 * separate spaces where lisp objects can reside.  Compiled code and gloabal
 * variables will reside in permanant space.  Eveything else will reside in
 * tmp space which gets thrown away after completion of eval.  system
 * functions reside in permant space. */
LSpace LSpace::Tmp, LSpace::Perm, LSpace::Gc;

/* Normally set to Tmp, unless compiling or other needs. */
LSpace *LSpace::Current;

bFILE *current_print_file = NULL;

LSymbol *lisp::symbol_root = nullptr;
size_t lisp::symbol_count = 0;

LObject *lisp::null = nullptr;

int print_level = 0, trace_level = 0, trace_print_level = 1000;
int total_user_functions;
static int evaldepth = 0, maxevaldepth = 0;

int break_level=0;

void l1print(void *block)
{
    if(!block || item_type(block) != L_CONS_CELL)
    {
        lisp::print((LObject *)block);
        return;
    }

    dprintf("(");
    for( ; block && item_type(block) == L_CONS_CELL; block = lisp::cdr(block))
    {
        void *a = lisp::car(block);
        if(item_type(a) == L_CONS_CELL)
            dprintf("[...]");
        else
            lisp::print((LObject *)a);
    }
    if (block)
    {
        dprintf(" . ");
        lisp::print((LObject *)block);
    }
    dprintf(")");
}

void where_print(int max_lev = -1)
{
    dprintf("Main program\n");
    if (max_lev == -1)
        max_lev = PtrRef::stack.m_size;
    else if (max_lev >= (int)PtrRef::stack.m_size)
        max_lev = PtrRef::stack.m_size - 1;

    for (int i = 0; i < max_lev; i++)
    {
        dprintf("%d> ", i);
        lisp::print((LObject *)*PtrRef::stack.sdata[i]);
    }
}

void print_trace_stack(int max_levels)
{
    where_print(max_levels);
}

void lbreak(char const *format, ...)
{
  break_level++;
  bFILE *old_file=current_print_file;
  current_print_file=NULL;
  char st[300];
  va_list ap;
  va_start(ap, format);
  vsprintf(st, format, ap);
  va_end(ap);
  dprintf("%s\n", st);
  int cont=0;
  do
  {
    dprintf("type q to quit\n");
    dprintf("%d. Break> ", break_level);
    dgets(st, 300);
    if (!strcmp(st, "c") || !strcmp(st, "cont") || !strcmp(st, "continue"))
      cont=1;
    else if (!strcmp(st, "w") || !strcmp(st, "where"))
      where_print();
    else if (!strcmp(st, "q") || !strcmp(st, "quit"))
      exit(1);
    else if (!strcmp(st, "e") || !strcmp(st, "env") || !strcmp(st, "environment"))
    {
      dprintf("Enviorment : \nnot supported right now\n");

    }
    else if (!strcmp(st, "h") || !strcmp(st, "help") || !strcmp(st, "?"))
    {
      dprintf("CLIVE Debugger\n");
      dprintf(" w, where : show calling parents\n"
          " e, env   : show environment\n"
          " c, cont  : continue if possible\n"
          " q, quit  : quits the program\n"
          " h, help  : this\n");
    }
    else
    {
      char const *s=st;
      do
      {
        LObject *prog = lisp::compile(s);
        PtrRef r1(prog);
        while (*s==' ' || *s=='\t' || *s=='\r' || *s=='\n')
            s++;
        lisp::print(lisp::eval(prog));
      } while (*s);
    }

  } while (!cont);
  current_print_file=old_file;
  break_level--;
}

void need_perm_space(char const *why)
{
  if (LSpace::Current != &LSpace::Perm && LSpace::Current != &LSpace::Gc)
  {
    lbreak("%s : action requires permanant space\n", why);
    exit(0);
  }
}

void *LSpace::Mark()
{
    return m_free;
}

void LSpace::Restore(void *val)
{
    m_free = (uint8_t *)val;
}

size_t LSpace::GetFree()
{
    size_t used = m_free - m_data;
    return m_size > used ? m_size - used : 0;
}

void *LSpace::Alloc(size_t size)
{
    // Align allocation
    size = (size + sizeof(intptr_t) - 1) & ~(sizeof(intptr_t) - 1);

    // Collect garbage if necessary
    if (size > GetFree())
    {
        if (this == &LSpace::Perm || this == &LSpace::Tmp)
            lisp::collect_space(this, 0);

        if (size > GetFree())
            lisp::collect_space(this, 1);

        if (size > GetFree())
        {
            lbreak("lisp: cannot find %d bytes in %s\n", size, m_name);
            exit(0);
        }
    }

    void *ret = m_free;
    m_free += size;
    return ret;
}

void *eval_block(void *list)
{
  PtrRef r1(list);
  void *ret=NULL;
  while (list)
  {
    ret = lisp::eval(lisp::car(list));
    list = lisp::cdr(list);
  }
  return ret;
}

LArray *LArray::Create(size_t len, void *rest)
{
    PtrRef r11(rest);
    size_t size = sizeof(LArray) + (len - 1) * sizeof(LObject *);
    if (size < sizeof(LRedirect))
        size = sizeof(LRedirect);

    LArray *p = (LArray *)LSpace::Current->Alloc(size);
    p->m_type = L_1D_ARRAY;
    p->m_len = len;
    LObject **data = p->GetData();
    memset(data, 0, len * sizeof(LObject *));
    PtrRef r1(p);

    if (rest)
    {
        LObject *x = lisp::eval(lisp::car(rest));
        if (x == colon_initial_contents)
        {
            x = lisp::eval(lisp::cadr(rest));
            data = p->GetData();
            for (size_t i = 0; i < len; i++, x = lisp::cdr(x))
            {
                if (!x)
                {
                    lisp::print((LObject *)rest);
                    lbreak("(make-array) incorrect list length\n");
                    exit(0);
                }
                data[i] = (LObject *)lisp::car(x);
            }
            if (x)
            {
                lisp::print((LObject *)rest);
                lbreak("(make-array) incorrect list length\n");
                exit(0);
            }
        }
        else if (x == colon_initial_element)
        {
            x = lisp::eval(lisp::cadr(rest));
            data = p->GetData();
            for (size_t i = 0; i < len; i++)
                data[i] = (LObject *)x;
        }
        else
        {
            lisp::print((LObject *)x);
            lbreak("Bad option argument to make-array\n");
            exit(0);
        }
    }

    return p;
}

LFixedPoint *LFixedPoint::Create(int32_t x)
{
    size_t size = lol::max(sizeof(LFixedPoint), sizeof(LRedirect));

    LFixedPoint *p = (LFixedPoint *)LSpace::Current->Alloc(size);
    p->m_type = L_FIXED_POINT;
    p->m_fixed = x;
    return p;
}

LObjectVar *LObjectVar::Create(int index)
{
    size_t size = lol::max(sizeof(LObjectVar), sizeof(LRedirect));

    LObjectVar *p = (LObjectVar *)LSpace::Current->Alloc(size);
    p->m_type = L_OBJECT_VAR;
    p->m_index = index;
    return p;
}

LPointer *LPointer::Create(void *addr)
{
    if (addr == NULL)
        return NULL;
    size_t size = lol::max(sizeof(LPointer), sizeof(LRedirect));

    LPointer *p = (LPointer *)LSpace::Current->Alloc(size);
    p->m_type = L_POINTER;
    p->m_addr = addr;
    return p;
}

LChar *LChar::Create(uint16_t ch)
{
    size_t size = lol::max(sizeof(LChar), sizeof(LRedirect));

    LChar *c = (LChar *)LSpace::Current->Alloc(size);
    c->m_type = L_CHARACTER;
    c->m_ch = ch;
    return c;
}

struct LString *LString::Create(char const *string)
{
    LString *s = Create(strlen(string) + 1);
    strcpy(s->m_str, string);
    return s;
}

struct LString *LString::Create(char const *string, int length)
{
    LString *s = Create(length + 1);
    memcpy(s->m_str, string, length);
    s->m_str[length] = 0;
    return s;
}

struct LString *LString::Create(int length)
{
    size_t size = lol::max(sizeof(LString) + length - 1, sizeof(LRedirect));

    LString *s = (LString *)LSpace::Current->Alloc(size);
    s->m_type = L_STRING;
    s->m_str[0] = '\0';
    return s;
}

LUserFunction *new_lisp_user_function(LList *arg_list, LList *block_list)
{
    PtrRef r1(arg_list), r2(block_list);

    size_t size = lol::max(sizeof(LUserFunction), sizeof(LRedirect));

    LUserFunction *lu = (LUserFunction *)LSpace::Current->Alloc(size);
    lu->m_type = L_USER_FUNCTION;
    lu->arg_list = arg_list;
    lu->block_list = block_list;
    return lu;
}

LSysFunction *new_lisp_sys_function(int min_args, int max_args, int fun_number)
{
    size_t size = lol::max(sizeof(LSysFunction), sizeof(LRedirect));

    // System functions should reside in permanant space
    LSysFunction *ls = LSpace::Current == &LSpace::Gc
                     ? (LSysFunction *)LSpace::Gc.Alloc(size)
                     : (LSysFunction *)LSpace::Perm.Alloc(size);
    ls->m_type = L_SYS_FUNCTION;
    ls->min_args = min_args;
    ls->max_args = max_args;
    ls->fun_number = fun_number;
    return ls;
}

LSysFunction *new_lisp_c_function(int min_args, int max_args, int fun_number)
{
    LSysFunction *ls = new_lisp_sys_function(min_args, max_args, fun_number);
    ls->m_type = L_C_FUNCTION;
    return ls;
}

LSysFunction *new_lisp_c_bool(int min_args, int max_args, int fun_number)
{
    LSysFunction *ls = new_lisp_sys_function(min_args, max_args, fun_number);
    ls->m_type = L_C_BOOL;
    return ls;
}

LSysFunction *new_user_lisp_function(int min_args, int max_args, int fun_number)
{
    LSysFunction *ls = new_lisp_sys_function(min_args, max_args, fun_number);
    ls->m_type = L_L_FUNCTION;
    return ls;
}

LSymbol *new_lisp_symbol(char *name)
{
    size_t size = lol::max(sizeof(LSymbol), sizeof(LRedirect));

    LSymbol *s = (LSymbol *)LSpace::Current->Alloc(size);
    PtrRef ref(s);

    s->m_type = L_SYMBOL;
    s->m_name = LString::Create(name);
    s->m_value = lisp::obj::undefined;
    s->m_function = lisp::obj::undefined;
#ifdef L_PROFILE
    s->time_taken = 0;
#endif
    return s;
}

LNumber *LNumber::Create(long num)
{
    size_t size = lol::max(sizeof(LNumber), sizeof(LRedirect));

    LNumber *n = (LNumber *)LSpace::Current->Alloc(size);
    n->m_type = L_NUMBER;
    n->m_num = num;
    return n;
}

LList *LList::Create()
{
    size_t size = lol::max(sizeof(LList), sizeof(LRedirect));

    LList *c = (LList *)LSpace::Current->Alloc(size);
    c->m_type = L_CONS_CELL;
    c->m_car = NULL;
    c->m_cdr = NULL;
    return c;
}

char *lerror(char const *loc, char const *cause)
{
  int lines;
  if (loc)
  {
    for (lines=0; *loc && lines<10; loc++)
    {
      if (*loc=='\n') lines++;
      dprintf("%c", *loc);
    }
    dprintf("\nPROGRAM LOCATION : \n");
  }
  if (cause)
    dprintf("ERROR MESSAGE : %s\n", cause);
  lbreak("");
  exit(0);
  return NULL;
}

LObject *lisp::nth(int num, void *list)
{
    if (num < 0)
    {
        lbreak("NTH: %d is not a nonnegative fixnum and therefore not a valid index\n", num);
        exit(1);
    }

    while (list && num--)
        list = lisp::cdr(list);

    return list ? lisp::car(list) : nullptr;
}

void *lpointer_value(void *lpointer)
{
    if (!lpointer)
        return nullptr;
    ASSERT_TYPE((LPointer *)lpointer, L_POINTER, "not a pointer");
    return ((LPointer const *)lpointer)->m_addr;
}

int32_t lnumber_value(void *lnumber)
{
    switch (item_type(lnumber))
    {
    case L_NUMBER:
        return ((LNumber const *)lnumber)->m_num;
    case L_FIXED_POINT:
        return ((LFixedPoint const *)lnumber)->m_fixed >> 16;
    case L_STRING:
        return (uint8_t)*lstring_value(lnumber);
    case L_CHARACTER:
        return ((LChar const *)lnumber)->m_ch;
    default:
        lisp::print((LObject *)lnumber);
        lbreak(" is not a number\n");
        exit(0);
    }
    return 0;
}

char *LString::GetString()
{
    ASSERT_TYPE(this, L_STRING, "not a string");
    return m_str;
}

void *lisp_atom(void *i)
{
  if (item_type(i)==(ltype)L_CONS_CELL)
    return NULL;
  else return lisp::sym::true_;
}

uint16_t LChar::GetValue()
{
    ASSERT_TYPE(this, L_CHARACTER, "not a character");
    return m_ch;
}

long lfixed_point_value(void *c)
{
    switch (item_type(c))
    {
        case L_NUMBER:
            return ((LNumber const *)c)->m_num<<16; break;
        case L_FIXED_POINT:
            return (((LFixedPoint const *)c)->m_fixed); break;
        default:
        {
            lisp::print((LObject *)c);
            lbreak(" is not a number\n");
            exit(0);
        }
    }
    return 0;
}

void *lisp_eq(void *n1, void *n2)
{
  if (!n1 && !n2) return lisp::sym::true_;
  else if ((n1 && !n2) || (n2 && !n1)) return NULL;
  {
    int t1=*((ltype *)n1), t2=*((ltype *)n2);
    if (t1!=t2) return NULL;
    else if (t1==L_NUMBER)
    { if (((LNumber *)n1)->m_num==((LNumber *)n2)->m_num)
        return lisp::sym::true_;
      else return NULL;
    } else if (t1==L_CHARACTER)
    {
      if (((LChar *)n1)->m_ch==((LChar *)n2)->m_ch)
        return lisp::sym::true_;
      else return NULL;
    }
    else if (n1==n2)
      return lisp::sym::true_;
    else if (t1==L_POINTER)
      if (n1==n2) return lisp::sym::true_;
  }
  return NULL;
}

LObject *LArray::Get(int x)
{
    ASSERT_TYPE(this, L_1D_ARRAY, "not an array");
    if (x >= (int)m_len || x < 0)
    {
        lbreak("array reference out of bounds (%d)\n", x);
        exit(0);
    }
    return m_data[x];
}

void *lisp_equal(void *n1, void *n2)
{
    if(!n1 && !n2) // if both nil, then equal
        return lisp::sym::true_;

    if(!n1 || !n2) // one nil, nope
        return NULL;

    int t1 = item_type(n1), t2 = item_type(n2);
    if(t1 != t2)
        return NULL;

    switch (t1)
    {
    case L_STRING :
        if (!strcmp(lstring_value(n1), lstring_value(n2)))
            return lisp::sym::true_;
        return NULL;
    case L_CONS_CELL :
        while (n1 && n2) // loop through the list and compare each element
        {
          if (!lisp_equal(lisp::car(n1), lisp::car(n2)))
            return NULL;
          n1=lisp::cdr(n1);
          n2=lisp::cdr(n2);
          if (n1 && *((ltype *)n1)!=L_CONS_CELL)
            return lisp_equal(n1, n2);
        }
        if (n1 || n2)
            return NULL;   // if one is longer than the other
        return lisp::sym::true_;
    default :
        return lisp_eq(n1, n2);
    }
}

int32_t lisp::cos(int32_t x)
{
    return int32_t(65535.9f * lol::cos(x * (lol::F_PI / 180)));
}

int32_t lisp::sin(int32_t x)
{
    return int32_t(65535.9f * lol::sin(x * (lol::F_PI / 180)));
}

int32_t lisp::atan2(int32_t dy, int32_t dx)
{
    // We want 0—360 but atan2 provides -180—180 so we call atan2(-dy,-dx)
    return 180 + int32_t((180 / lol::F_PI) * lol::atan2(-dy / 65535.f, -dx / 65535.f));
}

/*
LSymbol *find_symbol(char const *name)
{
  LList *cs;
  for (cs=(LList *)symbol_list; cs; cs=(LList *)lisp::cdr(cs))
  {
    if (!strcmp( ((char *)((LSymbol *)cs->m_car)->m_name)+sizeof(LString), name))
      return (LSymbol *)(cs->m_car);
  }
  return NULL;
}


LSymbol *make_find_symbol(char const *name)    // find a symbol, if it doesn't exists it is created
{
  LSymbol *s=find_symbol(name);
  if (s) return s;
  else
  {
    LSpace *sp = LSpace::Current;
    if (LSpace::Current != &LSpace::Gc)
      LSpace::Current = &LSpace::Perm;       // make sure all symbols get defined in permanant space
    LList *cs;
    cs=LList::Create();
    s=new_lisp_symbol(name);
    cs->m_car=s;
    cs->m_cdr=symbol_list;
    symbol_list=cs;
    LSpace::Current = sp;
  }
  return s;
}

*/

LSymbol *lisp::make_sym(char const *name)
{
    return find_sym(name, true);
}

LSymbol *lisp::find_sym(char const *name, bool create)
{
    LSymbol *p = symbol_root;
    LSymbol **parent = &symbol_root;
    while (p)
    {
        int cmp = strcmp(name, p->m_name->GetString());
        if (cmp == 0)
            return p;
        parent = (cmp < 0) ? &p->m_left : &p->m_right;
        p = *parent;
    }

    if (!create)
        return nullptr;

    // Make sure all symbols get defined in permanant space
    LSpace *sp = LSpace::Current;
    if (LSpace::Current != &LSpace::Gc)
        LSpace::Current = &LSpace::Perm;

    // These permanent objects cannot be GCed, so malloc() them
    p = (LSymbol *)malloc(sizeof(LSymbol));
    p->m_type = L_SYMBOL;
    p->m_name = LString::Create(name);

    // If constant, set the value to ourself
    p->m_value = (name[0] == ':') ? p : lisp::obj::undefined;
    p->m_function = lisp::obj::undefined;
#ifdef L_PROFILE
    p->time_taken = 0;
#endif
    p->m_left = p->m_right = NULL;
    *parent = p;
    symbol_count++;

    LSpace::Current = sp;
    return p;
}

static void DeleteAllSymbols(LSymbol *root)
{
    if (root)
    {
        DeleteAllSymbols(root->m_left);
        DeleteAllSymbols(root->m_right);
        free(root);
    }
}

LList *LList::Assoc(LObject *item)
{
    LList *list = this;
    while (list && item_type(list) == L_CONS_CELL
                && item_type(lisp::car(list)) == L_CONS_CELL)
    {
        if (lisp_eq(lisp::caar(list), item))
            return (LList *)lisp::car(list);
        list = (LList *)lisp::cdr(list);
    }

    return NULL;
}

size_t LList::GetLength()
{
    size_t ret = 0;

    ASSERT_TYPE(this, L_CONS_CELL, "not a sequence");

    for (LObject *p = this; p; p = lisp::cdr(p))
        ret++;
    return ret;
}

void *pairlis(void *list1, void *list2, void *list3)
{
  if (item_type(list1)!=(ltype)L_CONS_CELL || item_type(list1)!=item_type(list2))
    return NULL;

  void *ret=NULL;
  size_t l1 = ((LList *)list1)->GetLength();
  size_t l2 = ((LList *)list2)->GetLength();

  if (l1!=l2)
  {
    lisp::print((LObject *)list1);
    lisp::print((LObject *)list2);
    lbreak("... are not the same length (pairlis)\n");
    exit(0);
  }
  if (l1!=0)
  {
    LList *first = NULL, *last = NULL, *cur = NULL;
    LObject *tmp;
    PtrRef r1(first), r2(last), r3(cur);
    while (list1)
    {
      cur = LList::Create();
      if (!first)
        first = cur;
      if (last)
        last->m_cdr = cur;
      last = cur;

      LList *cell = LList::Create();
      tmp = (LObject *)lisp::car(list1);
      cell->m_car = tmp;
      tmp = (LObject *)lisp::car(list2);
      cell->m_cdr = tmp;
      cur->m_car = cell;

      list1 = ((LList *)list1)->m_cdr;
      list2 = ((LList *)list2)->m_cdr;
    }
    cur->m_cdr = (LObject *)list3;
    ret=first;
  } else ret=NULL;
  return ret;
}

void LSymbol::SetFunction(LObject *function)
{
    m_function = function;
}

LSymbol *add_sys_function(char const *name, short min_args, short max_args, short number)
{
  need_perm_space("add_sys_function");
  LSymbol *s = lisp::make_sym(name);
  if (s->m_function!=lisp::obj::undefined)
  {
    lbreak("add_sys_fucntion -> symbol %s already has a function\n", name);
    exit(0);
  }
  else s->m_function=new_lisp_sys_function(min_args, max_args, number);
  return s;
}

LSymbol *add_c_object(void *symbol, int index)
{
  need_perm_space("add_c_object");
  LSymbol *s=(LSymbol *)symbol;
  if (s->m_value!=lisp::obj::undefined)
  {
    lbreak("add_c_object -> symbol %s already has a value\n", lstring_value(s->GetName()));
    exit(0);
  }
  else s->m_value=LObjectVar::Create(index);
  return NULL;
}

LSymbol *add_c_function(char const *name, short min_args, short max_args, short number)
{
  total_user_functions++;
  need_perm_space("add_c_function");
  LSymbol *s = lisp::make_sym(name);
  if (s->m_function!=lisp::obj::undefined)
  {
    lbreak("add_sys_fucntion -> symbol %s already has a function\n", name);
    exit(0);
  }
  else s->m_function=new_lisp_c_function(min_args, max_args, number);
  return s;
}

LSymbol *add_c_bool_fun(char const *name, short min_args, short max_args, short number)
{
  total_user_functions++;
  need_perm_space("add_c_bool_fun");
  LSymbol *s = lisp::make_sym(name);
  if (s->m_function!=lisp::obj::undefined)
  {
    lbreak("add_sys_fucntion -> symbol %s already has a function\n", name);
    exit(0);
  }
  else s->m_function=new_lisp_c_bool(min_args, max_args, number);
  return s;
}


LSymbol *add_lisp_function(char const *name, short min_args, short max_args, short number)
{
  total_user_functions++;
  need_perm_space("add_c_bool_fun");
  LSymbol *s = lisp::make_sym(name);
  if (s->m_function!=lisp::obj::undefined)
  {
    lbreak("add_sys_fucntion -> symbol %s already has a function\n", name);
    exit(0);
  }
  else s->m_function=new_user_lisp_function(min_args, max_args, number);
  return s;
}

void skip_c_comment(char const *&s)
{
  s+=2;
  while (*s && (*s!='*' || *(s+1)!='/'))
  {
    if (*s=='/' && *(s+1)=='*')
      skip_c_comment(s);
    else s++;
  }
  if (*s) s+=2;
}

long str_token_len(char const *st)
{
  long x=1;
  while (*st && (*st!='"' || st[1]=='"'))
  {
    if (*st=='\\' || *st=='"') st++;
    st++; x++;
  }
  return x;
}

int read_ltoken(char const *&s, char *buffer)
{
  // skip space
  while (*s==' ' || *s=='\t' || *s=='\n' || *s=='\r' || *s==26) s++;
  if (*s==';')  // comment
  {
    while (*s && *s!='\n' && *s!='\r' && *s!=26) s++;
    return read_ltoken(s, buffer);
  } else if  (*s=='/' && *(s+1)=='*')   // c style comment
  {
    skip_c_comment(s);
    return read_ltoken(s, buffer);
  }
  else if (*s==0)
    return 0;
  else if (*s==')' || *s=='(' || *s=='\'' || *s=='`' || *s==',' || *s==26)
  {
    *(buffer++)=*(s++);
    *buffer=0;
  } else if (*s=='"')    // string
  {
    *(buffer++)=*(s++);          // don't read off the string because it
                                 // may be to long to fit in the token buffer
                                 // so just read the '"' so the compiler knows to scan the rest.
    *buffer=0;
  } else if (*s=='#')
  {
    *(buffer++)=*(s++);
    if (*s!='\'')
      *(buffer++)=*(s++);
    *buffer=0;
  } else
  {
    while (*s && *s!=')' && *s!='(' && *s!=' ' && *s!='\n' && *s!='\r' && *s!='\t' && *s!=';' && *s!=26)
      *(buffer++)=*(s++);
    *buffer=0;
  }
  return 1;
}


static char token_buffer[MAX_LISP_TOKEN_LEN];  // assume all tokens will be < 200 characters

int end_of_program(char const *s)
{
  return !read_ltoken(s, token_buffer);
}


void push_onto_list(void *object, void *&list)
{
  PtrRef r1(object), r2(list);
  LList *c = LList::Create();
  c->m_car = (LObject *)object;
  c->m_cdr = (LObject *)list;
  list=c;
}

void *comp_optimize(void *list);

LObject *lisp::compile(char const *&code)
{
    LObject *ret = NULL;

    if (!read_ltoken(code, token_buffer))
        lerror(NULL, "unexpected end of program");

  if (!strcmp(token_buffer, "nil"))
    return NULL;
  else if (toupper(token_buffer[0])=='T' && !token_buffer[1])
    return lisp::sym::true_;
  else if (token_buffer[0]=='\'')                    // short hand for quote function
  {
    LObject *cs = LList::Create(), *c2=NULL, *tmp;
    PtrRef r1(cs), r2(c2);

    ((LList *)cs)->m_car=lisp::sym::quote;
    c2 = LList::Create();
    tmp = compile(code);
    ((LList *)c2)->m_car = (LObject *)tmp;
    ((LList *)c2)->m_cdr=NULL;
    ((LList *)cs)->m_cdr = (LObject *)c2;
    ret=cs;
  }
  else if (token_buffer[0]=='`')                    // short hand for backquote function
  {
    LObject *cs = LList::Create(), *c2=NULL, *tmp;
    PtrRef r1(cs), r2(c2);

    ((LList *)cs)->m_car=lisp::sym::backquote;
    c2 = LList::Create();
    tmp = compile(code);
    ((LList *)c2)->m_car = (LObject *)tmp;
    ((LList *)c2)->m_cdr=NULL;
    ((LList *)cs)->m_cdr = (LObject *)c2;
    ret=cs;
  }  else if (token_buffer[0]==',')              // short hand for comma function
  {
    LObject *cs = LList::Create(), *c2=NULL, *tmp;
    PtrRef r1(cs), r2(c2);

    ((LList *)cs)->m_car=lisp::sym::comma;
    c2 = LList::Create();
    tmp = compile(code);
    ((LList *)c2)->m_car = (LObject *)tmp;
    ((LList *)c2)->m_cdr=NULL;
    ((LList *)cs)->m_cdr = (LObject *)c2;
    ret=cs;
  }
  else if (token_buffer[0]=='(')                     // make a list of everything in ()
  {
    void *first=NULL, *cur=NULL, *last=NULL;
    PtrRef r1(first), r2(cur), r3(last);
    int done=0;
    do
    {
      char const *tmp=code;
      if (!read_ltoken(tmp, token_buffer))           // check for the end of the list
        lerror(NULL, "unexpected end of program");
      if (token_buffer[0]==')')
      {
                done=1;
                read_ltoken(code, token_buffer);                // read off the ')'
      }
      else
      {
                if (token_buffer[0]=='.' && !token_buffer[1])
                {
                  if (!first)
                    lerror(code, "token '.' not allowed here\n");
                  else
                  {
                    void *tmp;
                    read_ltoken(code, token_buffer);              // skip the '.'
                    tmp = lisp::compile(code);
                    ((LList *)last)->m_cdr = (LObject *)tmp;          // link the last cdr to
                    last=NULL;
                  }
                } else if (!last && first)
                  lerror(code, "illegal end of dotted list\n");
                else
                {
                  void *tmp;
                  cur = LList::Create();
                  PtrRef r4(cur);
                  if (!first) first=cur;
                  tmp = lisp::compile(code);
                  ((LList *)cur)->m_car = (LObject *)tmp;
                  if (last)
                    ((LList *)last)->m_cdr = (LObject *)cur;
                  last=cur;
                }
      }
    } while (!done);
    ret=(LObject *)comp_optimize(first);

  } else if (token_buffer[0]==')')
    lerror(code, "mismatched )");
  else if (isdigit(token_buffer[0]) || (token_buffer[0]=='-' && isdigit(token_buffer[1])))
  {
    LNumber *num = LNumber::Create(0);
    sscanf(token_buffer, "%ld", &num->m_num);
    ret=num;
  } else if (token_buffer[0]=='"')
  {
    ret = LString::Create(str_token_len(code));
    char *start=lstring_value(ret);
    for (; *code && (*code!='"' || code[1]=='"'); code++, start++)
    {
      if (*code=='\\')
      {
                code++;
                if (*code=='n') *start='\n';
                if (*code=='r') *start='\r';
                if (*code=='t') *start='\t';
                if (*code=='\\') *start='\\';
      } else *start=*code;
      if (*code=='"') code++;
    }
    *start=0;
    code++;
  } else if (token_buffer[0]=='#')
  {
    if (token_buffer[1]=='\\')
    {
      read_ltoken(code, token_buffer);                   // read character name
      if (!strcmp(token_buffer, "newline"))
        ret = LChar::Create('\n');
      else if (!strcmp(token_buffer, "space"))
        ret = LChar::Create(' ');
      else
        ret = LChar::Create(token_buffer[0]);
    }
    else if (token_buffer[1]==0)                           // short hand for function
    {
      LObject *cs = LList::Create(), *c2=NULL, *tmp;
      PtrRef r4(cs), r5(c2);
      tmp = lisp::make_sym("function");
      ((LList *)cs)->m_car = (LObject *)tmp;
      c2 = LList::Create();
      tmp = lisp::compile(code);
      ((LList *)c2)->m_car = (LObject *)tmp;
      ((LList *)cs)->m_cdr = (LObject *)c2;
      ret=cs;
    }
    else
    {
      lbreak("Unknown #\\ notation : %s\n", token_buffer);
      exit(0);
    }
  } else {
    ret = lisp::make_sym(token_buffer);
  }
  return ret;
}


static void lprint_string(char const *st)
{
  if (current_print_file)
  {
    for (char const *s=st; *s; s++)
    {
/*      if (*s=='\\')
      {
    s++;
    if (*s=='n')
      current_print_file->write_uint8('\n');
    else if (*s=='r')
      current_print_file->write_uint8('\r');
    else if (*s=='t')
      current_print_file->write_uint8('\t');
    else if (*s=='\\')
      current_print_file->write_uint8('\\');
      }
      else*/
        current_print_file->write_uint8(*s);
    }
  }
  else
    dprintf(st);
}

void lisp::print(LObject *o)
{
    char buf[32];

    print_level++;

    switch (item_type(o))
    {
    case L_CONS_CELL:
        if (!o)
        {
            lprint_string("nil");
        }
        else
        {
            LList *cs = (LList *)o;
            lprint_string("(");
            for (; cs; cs = (LList *)lisp::cdr(cs))
            {
                if (item_type(cs) == (ltype)L_CONS_CELL)
                {
                    print(cs->m_car);
                    if (cs->m_cdr)
                        lprint_string(" ");
                }
                else
                {
                    lprint_string(". ");
                    print(cs);
                    cs = NULL;
                }
            }
            lprint_string(")");
        }
        break;
    case L_NUMBER:
        sprintf(buf, "%ld", ((LNumber const *)o)->m_num);
        lprint_string(buf);
        break;
    case L_SYMBOL:
        lprint_string(((LSymbol const *)o)->m_name->GetString());
        break;
    case L_USER_FUNCTION:
    case L_SYS_FUNCTION:
        lprint_string("err... function?");
        break;
    case L_C_FUNCTION:
        lprint_string("C function, returns number\n");
        break;
    case L_C_BOOL:
        lprint_string("C boolean function\n");
        break;
    case L_L_FUNCTION:
        lprint_string("External lisp function\n");
        break;
    case L_STRING:
        if (current_print_file)
            lprint_string(lstring_value(o));
        else
            dprintf("\"%s\"", lstring_value(o));
        break;
    case L_POINTER:
        sprintf(buf, "%p", lpointer_value(o));
        lprint_string(buf);
        break;
    case L_FIXED_POINT:
        sprintf(buf, "%g", (lfixed_point_value(o) >> 16) +
                ((lfixed_point_value(o) & 0xffff)) / (double)0x10000);
        lprint_string(buf);
        break;
    case L_CHARACTER:
        if (current_print_file)
        {
            uint8_t ch = ((LChar const *)o)->m_ch;
            current_print_file->write(&ch, 1);
        }
        else
        {
            uint16_t ch = ((LChar const *)o)->m_ch;
            dprintf("#\\");
            switch (ch)
            {
            case '\n':
                dprintf("newline"); break;
            case ' ':
                dprintf("space"); break;
            default:
                dprintf("%c", ch); break;
            }
        }
        break;
    case L_OBJECT_VAR:
        l_obj_print(((LObjectVar const *)o)->m_index);
        break;
    case L_1D_ARRAY:
        {
            LArray *a = (LArray *)o;
            LObject **data = a->GetData();
            dprintf("#(");
            for (size_t j = 0; j < a->m_len; j++)
            {
                lisp::print(data[j]);
                if (j != a->m_len - 1)
                    dprintf(" ");
            }
            dprintf(")");
        }
        break;
    case L_COLLECTED_OBJECT:
        lprint_string("GC_reference->");
        lisp::print(((LRedirect *)o)->m_ref);
        break;
    default:
        dprintf("Shouldn't happen\n");
    }

    print_level--;
    if (!print_level && !current_print_file)
        dprintf("\n");
}

/* PtrRef check: OK */
LObject *LSymbol::EvalFunction(void *arg_list)
{
    ASSERT_TYPE(this, L_SYMBOL, "EVAL: not a function name (not symbol either)");

    LObject *fun = m_function;
    PtrRef ref2(fun);
    PtrRef ref3(arg_list);

    // make sure the arguments given to the function are the correct number
    ltype t = item_type(fun);

#ifdef TYPE_CHECKING
    int args, req_min, req_max;

    switch (t)
    {
    case L_SYS_FUNCTION:
    case L_C_FUNCTION:
    case L_C_BOOL:
    case L_L_FUNCTION:
        req_min = ((LSysFunction *)fun)->min_args;
        req_max = ((LSysFunction *)fun)->max_args;
        break;
    case L_USER_FUNCTION:
        return EvalUserFunction((LList *)arg_list);
    default:
        lisp::print(this);
        lbreak(" is not a function name");
        exit(0);
        break;
    }

    if (req_min != -1)
    {
        void *a = arg_list;
        for (args = 0; a; a = lisp::cdr(a))
            args++; // count number of parameters

        if (args < req_min)
        {
            lisp::print((LObject *)arg_list);
            lisp::print(m_name);
            lbreak("\nToo few parameters to function\n");
            exit(0);
        }
        else if (req_max != -1 && args > req_max)
        {
            lisp::print((LObject *)arg_list);
            lisp::print(m_name);
            lbreak("\nToo many parameters to function\n");
            exit(0);
        }
    }
#endif

#ifdef L_PROFILE
    Timer t;
#endif

    LObject *ret = NULL;

    switch (t)
    {
    case L_SYS_FUNCTION:
        ret = ((LSysFunction *)fun)->EvalFunction((LList *)arg_list);
        break;
    case L_L_FUNCTION:
        ret = (LObject *)l_caller(((LSysFunction *)fun)->fun_number, arg_list);
        break;
    case L_USER_FUNCTION:
        return EvalUserFunction((LList *)arg_list);
    case L_C_FUNCTION:
    case L_C_BOOL:
    {
        LList *first = NULL, *cur = NULL;
        PtrRef r1(first), r2(cur), r3(arg_list);
        while (arg_list)
        {
            LList *tmp = LList::Create();
            if (first)
                cur->m_cdr = tmp;
            else
                first = tmp;
            cur = tmp;

            LObject *val = lisp::eval(lisp::car(arg_list));
            ((LList *)cur)->m_car = val;
            arg_list = lisp::cdr(arg_list);
        }
        if (t == L_C_FUNCTION)
            ret = LNumber::Create(c_caller(((LSysFunction *)fun)->fun_number, first));
        else if (c_caller(((LSysFunction *)fun)->fun_number, first))
            ret = lisp::sym::true_;
        else
            ret = NULL;
        break;
    }
    default:
        fprintf(stderr, "not a fun, shouldn't happen\n");
    }

#ifdef L_PROFILE
    time_taken += t.Get();
#endif

    return ret;
}

#ifdef L_PROFILE
void pro_print(bFILE *out, LSymbol *p)
{
  if (p)
  {
    pro_print(out, p->m_right);
    {
      char st[100];
      sprintf(st, "%20s %f\n", lstring_value(p->GetName()), p->time_taken);
      out->write(st, strlen(st));
    }
    pro_print(out, p->m_left);
  }
}

void preport(char *fn)
{
  bFILE *fp=open_file("preport.out", "wb");
  pro_print(fp, LSymbol::root);
  delete fp;
}
#endif

void *mapcar(void *arg_list)
{
  PtrRef ref1(arg_list);
  LObject *sym = lisp::eval(lisp::car(arg_list));
  switch ((short)item_type(sym))
  {
    case L_SYS_FUNCTION:
    case L_USER_FUNCTION:
    case L_SYMBOL:
      break;
    default:
    {
      lisp::print(sym);
      lbreak(" is not a function\n");
      exit(0);
    }
  }
  int i, stop = 0, num_args = ((LList *)lisp::cdr(arg_list))->GetLength();
  if (!num_args) return 0;

  void **arg_on=(void **)malloc(sizeof(void *)*num_args);
  LList *list_on=(LList *)lisp::cdr(arg_list);
  long old_ptr_son=PtrRef::stack.m_size;

  for (i=0; i<num_args; i++)
  {
    arg_on[i] = (LList *)lisp::eval(lisp::car(list_on));
    PtrRef::stack.push(&arg_on[i]);

    list_on=(LList *)lisp::cdr(list_on);
    if (!arg_on[i]) stop=1;
  }

  if (stop)
  {
    free(arg_on);
    return NULL;
  }

  LList *na_list=NULL, *return_list=NULL, *last_return=NULL;

  do
  {
    na_list=NULL;          // create a cons list with all of the parameters for the function

    LList *first=NULL;                       // save the start of the list
    for (i=0; !stop &&i<num_args; i++)
    {
      if (!na_list)
        first=na_list = LList::Create();
      else
      {
        na_list->m_cdr = (LObject *)LList::Create();
                na_list=(LList *)lisp::cdr(na_list);
      }


      if (arg_on[i])
      {
                na_list->m_car = (LObject *)lisp::car(arg_on[i]);
                arg_on[i]=(LList *)lisp::cdr(arg_on[i]);
      }
      else stop=1;
    }
    if (!stop)
    {
      LList *c = LList::Create();
      c->m_car = ((LSymbol *)sym)->EvalFunction(first);
      if (return_list)
        last_return->m_cdr=c;
      else
        return_list=c;
      last_return=c;
    }
  }
  while (!stop);
  PtrRef::stack.m_size=old_ptr_son;

  free(arg_on);
  return return_list;
}

void *concatenate(void *prog_list)
{
  void *el_list=lisp::cdr(prog_list);
  PtrRef ref1(prog_list), ref2(el_list);
  void *ret=NULL;
  void *rtype = lisp::eval(lisp::car(prog_list));

  long len=0;                                // determin the length of the resulting string
  if (rtype==lisp::sym::string)
  {
    int elements = ((LList *)el_list)->GetLength(); // see how many things we need to concat
    if (!elements) ret = LString::Create("");
    else
    {
      void **str_eval=(void **)malloc(elements*sizeof(void *));
      int i, old_ptr_stack_start=PtrRef::stack.m_size;

      // evalaute all the strings and count their lengths
      for (i=0; i<elements; i++, el_list=lisp::cdr(el_list))
      {
        str_eval[i] = lisp::eval(lisp::car(el_list));
    PtrRef::stack.push(&str_eval[i]);

    switch ((short)item_type(str_eval[i]))
    {
      case L_CONS_CELL :
      {
        LList *char_list=(LList *)str_eval[i];
        while (char_list)
        {
          if (item_type(lisp::car(char_list))==(ltype)L_CHARACTER)
            len++;
          else
          {
        lisp::print((LObject *)str_eval[i]);
        lbreak(" is not a character\n");
        exit(0);
          }
          char_list=(LList *)lisp::cdr(char_list);
        }
      } break;
      case L_STRING : len+=strlen(lstring_value(str_eval[i])); break;
      default :
        lisp::print((LObject *)prog_list);
        lbreak("type not supported\n");
        exit(0);
      break;

    }
      }
      LString *st = LString::Create(len+1);
      char *s=lstring_value(st);

      // now add the string up into the new string
      for (i=0; i<elements; i++)
      {
    switch ((short)item_type(str_eval[i]))
    {
      case L_CONS_CELL :
      {
        LList *char_list=(LList *)str_eval[i];
        while (char_list)
        {
          if (item_type(lisp::car(char_list))==L_CHARACTER)
            *(s++)=((LChar *)lisp::car(char_list))->m_ch;
          char_list=(LList *)lisp::cdr(char_list);
        }
      } break;
      case L_STRING :
      {
        memcpy(s, lstring_value(str_eval[i]), strlen(lstring_value(str_eval[i])));
        s+=strlen(lstring_value(str_eval[i]));
      } break;
      default : ;     // already checked for, but make compiler happy
    }
      }
      free(str_eval);
      PtrRef::stack.m_size=old_ptr_stack_start;   // restore pointer GC stack
      *s=0;
      ret=st;
    }
  }
  else
  {
    lisp::print((LObject *)prog_list);
    lbreak("concat operation not supported, try 'string\n");
    exit(0);
  }
  return ret;
}


void *backquote_eval(void *args)
{
  if (item_type(args)!=L_CONS_CELL)
    return args;
  else if (args==NULL)
    return NULL;
  else if ((LSymbol *) (((LList *)args)->m_car)==lisp::sym::comma)
    return lisp::eval(lisp::cadr(args));
  else
  {
    void *first=NULL, *last=NULL, *cur=NULL, *tmp;
    PtrRef ref1(first), ref2(last), ref3(cur), ref4(args);
    while (args)
    {
      if (item_type(args)==L_CONS_CELL)
      {
    if (lisp::car(args)==lisp::sym::comma)               // dot list with a comma?
    {
      tmp = lisp::eval(lisp::cadr(args));
      ((LList *)last)->m_cdr = (LObject *)tmp;
      args=NULL;
    }
    else
    {
      cur = LList::Create();
      if (first)
        ((LList *)last)->m_cdr = (LObject *)cur;
      else
            first=cur;
      last=cur;
          tmp=backquote_eval(lisp::car(args));
          ((LList *)cur)->m_car = (LObject *)tmp;
       args=lisp::cdr(args);
    }
      } else
      {
    tmp=backquote_eval(args);
    ((LList *)last)->m_cdr = (LObject *)tmp;
    args=NULL;
      }

    }
    return (void *)first;
  }
  return NULL;       // for stupid compiler messages
}

/* PtrRef check: OK */
LObject *LSysFunction::EvalFunction(LList *arg_list)
{
    LObject *ret = NULL;

    PtrRef ref1(arg_list);

    switch (fun_number)
    {
    case SYS_FUNC_PRINT:
        while (arg_list)
        {
            ret = lisp::eval(lisp::car(arg_list));
            arg_list = (LList *)lisp::cdr(arg_list);
            lisp::print(ret);
        }
        break;
    case SYS_FUNC_CAR:
        ret = lisp::car(lisp::eval(lisp::car(arg_list)));
        break;
    case SYS_FUNC_CDR:
        ret = lisp::cdr(lisp::eval(lisp::car(arg_list)));
        break;
    case SYS_FUNC_LENGTH:
    {
        LObject *v = lisp::eval(lisp::car(arg_list));
        switch (item_type(v))
        {
        case L_STRING:
            ret = LNumber::Create(strlen(lstring_value(v)));
            break;
        case L_CONS_CELL:
            ret = LNumber::Create(((LList *)v)->GetLength());
            break;
        default:
            lisp::print(v);
            lbreak("length : type not supported\n");
            break;
        }
        break;
    }
    case SYS_FUNC_LIST:
    {
        LList *cur = NULL, *last = NULL, *first = NULL;
        PtrRef r1(cur), r2(first), r3(last);
        while (arg_list)
        {
            cur = LList::Create();
            LObject *val = lisp::eval(lisp::car(arg_list));
            cur->m_car = val;
            if (last)
                last->m_cdr = cur;
            else
                first = cur;
            last = cur;
            arg_list = (LList *)lisp::cdr(arg_list);
        }
        ret = first;
        break;
    }
    case SYS_FUNC_CONS:
    {
        LList *c = LList::Create();
        PtrRef r1(c);
        LObject *val = lisp::eval(lisp::car(arg_list));
        c->m_car = val;
        val = lisp::eval(lisp::cadr(arg_list));
        c->m_cdr = val;
        ret = c;
        break;
    }
    case SYS_FUNC_QUOTE:
        ret = lisp::car(arg_list);
        break;
    case SYS_FUNC_EQ:
        l_user_stack.push(lisp::eval(lisp::car(arg_list)));
        l_user_stack.push(lisp::eval(lisp::cadr(arg_list)));
        ret = (LObject *)lisp_eq(l_user_stack.pop(1), l_user_stack.pop(1));
        break;
    case SYS_FUNC_EQUAL:
        l_user_stack.push(lisp::eval(lisp::car(arg_list)));
        l_user_stack.push(lisp::eval(lisp::cadr(arg_list)));
        ret = (LObject *)lisp_equal(l_user_stack.pop(1), l_user_stack.pop(1));
        break;
    case SYS_FUNC_PLUS:
    {
        int32_t sum = 0;
        while (arg_list)
        {
            sum += lnumber_value(lisp::eval(lisp::car(arg_list)));
            arg_list = (LList *)lisp::cdr(arg_list);
        }
        ret = LNumber::Create(sum);
        break;
    }
    case SYS_FUNC_TIMES:
    {
        int32_t prod;
        LObject *first = lisp::eval(lisp::car(arg_list));
        PtrRef r1(first);
        if (arg_list && item_type(first) == L_FIXED_POINT)
        {
            prod = 1 << 16;
            do
            {
                prod = (prod >> 8) * (lfixed_point_value(first) >> 8);
                arg_list = (LList *)lisp::cdr(arg_list);
                if (arg_list)
                    first = lisp::eval(lisp::car(arg_list));
            } while (arg_list);
            ret = LFixedPoint::Create(prod);
        }
        else
        {
            prod = 1;
            do
            {
                prod *= lnumber_value(lisp::eval(lisp::car(arg_list)));
                arg_list = (LList *)lisp::cdr(arg_list);
                if (arg_list)
                    first = lisp::eval(lisp::car(arg_list));
            } while (arg_list);
            ret = LNumber::Create(prod);
        }
        break;
    }
    case SYS_FUNC_SLASH:
    {
        int32_t quot = 0, first = 1;
        while (arg_list)
        {
            LObject *i = lisp::eval(lisp::car(arg_list));
            if (item_type(i) != L_NUMBER)
            {
                lisp::print(i);
                lbreak("/ only defined for numbers, cannot divide ");
                exit(0);
            }
            else if (first)
            {
                quot = ((LNumber *)i)->m_num;
                first = 0;
            }
            else
                quot /= ((LNumber *)i)->m_num;
            arg_list = (LList *)lisp::cdr(arg_list);
        }
        ret = LNumber::Create(quot);
        break;
    }
    case SYS_FUNC_MINUS:
    {
        int32_t sub = lnumber_value(lisp::eval(lisp::car(arg_list)));
        arg_list = (LList *)lisp::cdr(arg_list);
        while (arg_list)
        {
            sub -= lnumber_value(lisp::eval(lisp::car(arg_list)));
            arg_list = (LList *)lisp::cdr(arg_list);
        }
        ret = LNumber::Create(sub);
        break;
    }
    case SYS_FUNC_IF:
        if (lisp::eval(lisp::car(arg_list)))
            ret = lisp::eval(lisp::cadr(arg_list));
        else
        {
            arg_list = (LList *)lisp::cddr(arg_list); // check for a else part
            if (arg_list)
                ret = lisp::eval(lisp::car(arg_list));
            else
                ret = NULL;
        }
        break;
    case SYS_FUNC_SETQ:
    case SYS_FUNC_SETF:
    {
        LObject *set_to = lisp::eval(lisp::cadr(arg_list)), *i = NULL;
        PtrRef r1(set_to), r2(i);
        i = lisp::car(arg_list);

        ltype x = item_type(set_to);
        switch (item_type(i))
        {
        case L_SYMBOL:
            switch (item_type(((LSymbol *)i)->m_value))
            {
            case L_NUMBER:
                if (x == L_NUMBER && ((LSymbol *)i)->m_value != lisp::obj::undefined)
                    ((LSymbol *)i)->SetNumber(lnumber_value(set_to));
                else
                    ((LSymbol *)i)->SetValue((LNumber *)set_to);
                break;
            case L_OBJECT_VAR:
                l_obj_set(((LObjectVar *)(((LSymbol *)i)->m_value))->m_index, set_to);
                break;
            default:
                ((LSymbol *)i)->SetValue((LObject *)set_to);
            }
            ret = ((LSymbol *)i)->m_value;
            break;
        case L_CONS_CELL:   // this better be an 'aref'
        {
#ifdef TYPE_CHECKING
            LObject *car = ((LList *)i)->m_car;
            if (car == lisp::sym::car)
            {
                car = lisp::eval(lisp::cadr(i));
                if (!car || item_type(car) != L_CONS_CELL)
                {
                    lisp::print(car);
                    lbreak("setq car : evaled object is not a cons cell\n");
                    exit(0);
                }
                ((LList *)car)->m_car = set_to;
            }
            else if (car == lisp::sym::cdr)
            {
                car = lisp::eval(lisp::cadr(i));
                if (!car || item_type(car) != L_CONS_CELL)
                {
                    lisp::print(car);
                    lbreak("setq cdr : evaled object is not a cons cell\n");
                    exit(0);
                }
                ((LList *)car)->m_cdr = set_to;
            }
            else if (car != lisp::sym::aref)
            {
                lbreak("expected (aref, car, cdr, or symbol) in setq\n");
                exit(0);
            }
            else
            {
#endif
                LArray *a = (LArray *)lisp::eval(lisp::cadr(i));
                PtrRef r3(a);
                ASSERT_TYPE(a, L_1D_ARRAY, "not an array (aref)");
                int num = lnumber_value(lisp::eval(lisp::caddr(i)));
#ifdef TYPE_CHECKING
                if (num >= (int)a->m_len || num < 0)
                {
                    lbreak("aref : value of bounds (%d)\n", num);
                    exit(0);
                }
#endif
                a->GetData()[num] = set_to;
#ifdef TYPE_CHECKING
            }
#endif
            ret = set_to;
            break;
        }
        default:
            lisp::print(i);
            lbreak("setq/setf only defined for symbols and arrays now..\n");
            exit(0);
            break;
        }
        break;
    }
    case SYS_FUNC_SYMBOL_LIST:
        ret = NULL;
        break;
    case SYS_FUNC_ASSOC:
    {
        LObject *item = lisp::eval(lisp::car(arg_list));
        PtrRef r1(item);
        LList *list = (LList *)lisp::eval(lisp::cadr(arg_list));
        PtrRef r2(list);
        ret = list->Assoc(item);
        break;
    }
    case SYS_FUNC_NOT:
    case SYS_FUNC_NULL:
        if (lisp::eval(lisp::car(arg_list)) == NULL)
            ret = lisp::sym::true_;
        else
            ret = NULL;
        break;
    case SYS_FUNC_ACONS:
    {
        LObject *i1 = lisp::eval(lisp::car(arg_list));
        PtrRef r1(i1);
        LObject *i2 = lisp::eval(lisp::cadr(arg_list));
        PtrRef r2(i2);
        LList *cs = LList::Create();
        cs->m_car = i1;
        cs->m_cdr = i2;
        ret = cs;
        break;
    }
    case SYS_FUNC_PAIRLIS:
    {
        l_user_stack.push(lisp::eval(lisp::car(arg_list)));
        arg_list = (LList *)lisp::cdr(arg_list);
        l_user_stack.push(lisp::eval(lisp::car(arg_list)));
        arg_list = (LList *)lisp::cdr(arg_list);
        LObject *n3 = lisp::eval(lisp::car(arg_list));
        LObject *n2 = (LObject *)l_user_stack.pop(1);
        LObject *n1 = (LObject *)l_user_stack.pop(1);
        ret = (LObject *)pairlis(n1, n2, n3);
        break;
    }
    case SYS_FUNC_LET:
    {
        // make an a-list of new variable names and new values
        LObject *var_list = lisp::car(arg_list);
        LObject *block_list = lisp::cdr(arg_list);
        PtrRef r1(block_list), r2(var_list);
        long stack_start = l_user_stack.m_size;

        while (var_list)
        {
            LObject *var_name = lisp::caar(var_list), *tmp;
            ASSERT_TYPE(var_name, L_SYMBOL, "should be a symbol (let)");

            l_user_stack.push(((LSymbol *)var_name)->m_value);
            tmp = lisp::eval(lisp::cadar(var_list));
            ((LSymbol *)var_name)->SetValue(tmp);
            var_list = lisp::cdr(var_list);
        }

        // now evaluate each of the blocks with the new environment and
        // return value from the last block
        while (block_list)
        {
            ret = lisp::eval(lisp::car(block_list));
            block_list = lisp::cdr(block_list);
        }

        long cur_stack = stack_start;
        var_list = lisp::car(arg_list); // now restore the old symbol values
        while (var_list)
        {
            LObject *var_name = lisp::caar(var_list);
            ((LSymbol *)var_name)->SetValue((LObject *)l_user_stack.sdata[cur_stack++]);
            var_list = lisp::cdr(var_list);
        }
        l_user_stack.m_size = stack_start; // restore the stack
        break;
    }
    case SYS_FUNC_DEFUN:
    {
        LSymbol *symbol = (LSymbol *)lisp::car(arg_list);
        PtrRef r1(symbol);

        ASSERT_TYPE(symbol, L_SYMBOL, "not a symbol (DEFUN)");
        ASSERT_TYPE(arg_list, L_CONS_CELL, "not a lambda list (DEFUN)");

        LObject *block_list = lisp::cddr(arg_list);

        LUserFunction *ufun = new_lisp_user_function((LList *)lisp::cadr(arg_list), (LList *)block_list);
        symbol->SetFunction(ufun);
        ret = symbol;
        break;
    }
    case SYS_FUNC_ATOM:
        ret = (LObject *)lisp_atom(lisp::eval(lisp::car(arg_list)));
        break;
    case SYS_FUNC_AND:
    {
        LObject *l = arg_list;
        PtrRef r1(l);
        ret = lisp::sym::true_;
        while (l)
        {
            if (!lisp::eval(lisp::car(l)))
            {
                ret = NULL;
                l = NULL; // short-circuit
            }
            else
                l = lisp::cdr(l);
        }
        break;
    }
    case SYS_FUNC_OR:
    {
        LObject *l = arg_list;
        PtrRef r1(l);
        ret = NULL;
        while (l)
        {
            if (lisp::eval(lisp::car(l)))
            {
                ret = lisp::sym::true_;
                l = NULL; // short-circuit
            }
            else
                l = lisp::cdr(l);
        }
        break;
    }
    case SYS_FUNC_PROGN:
        ret = (LObject *)eval_block(arg_list);
        break;
    case SYS_FUNC_CONCATENATE:
        ret = (LObject *)concatenate(arg_list);
        break;
    case SYS_FUNC_CHAR_CODE:
    {
        LObject *i = lisp::eval(lisp::car(arg_list));
        PtrRef r1(i);
        ret = NULL;
        switch (item_type(i))
        {
        case L_CHARACTER:
            ret = LNumber::Create(((LChar *)i)->m_ch);
            break;
        case L_STRING:
            ret = LNumber::Create(*lstring_value(i));
            break;
        default:
            lisp::print(i);
            lbreak(" is not character type\n");
            exit(0);
            break;
        }
        break;
    }
    case SYS_FUNC_CODE_CHAR:
    {
        LObject *i = lisp::eval(lisp::car(arg_list));
        PtrRef r1(i);
        ASSERT_TYPE(i, L_NUMBER, "not number type");
        ret = LChar::Create(((LNumber *)i)->m_num);
        break;
    }
    case SYS_FUNC_COND:
    {
        LList *block_list = (LList *)lisp::car(arg_list);
        PtrRef r1(block_list);
        ret = NULL;
        PtrRef r2(ret); // Required to protect from the last eval call
        while (block_list)
        {
            if (lisp::eval(lisp::caar(block_list)))
                ret = lisp::eval(lisp::cadar(block_list));
            block_list = (LList *)lisp::cdr(block_list);
        }
        break;
    }
    case SYS_FUNC_SELECT:
    {
        LObject *selector = lisp::eval(lisp::car(arg_list));
        LObject *sel = lisp::cdr(arg_list);
        PtrRef r1(selector), r2(sel);
        ret = NULL;
        PtrRef r3(ret); // Required to protect from the last eval call
        while (sel)
        {
            if (lisp_equal(selector, lisp::eval(lisp::caar(sel))))
            {
                sel = lisp::cdar(sel);
                while (sel)
                {
                    ret = lisp::eval(lisp::car(sel));
                    sel = lisp::cdr(sel);
                }
            }
            else
                sel = lisp::cdr(sel);
        }
        break;
    }
    case SYS_FUNC_FUNCTION:
        ret = ((LSymbol *)lisp::eval(lisp::car(arg_list)))->GetFunction();
        break;
    case SYS_FUNC_MAPCAR:
        ret = (LObject *)mapcar(arg_list);
        break;
    case SYS_FUNC_FUNCALL:
    {
        LSymbol *n1 = (LSymbol *)lisp::eval(lisp::car(arg_list));
        ret = n1->EvalFunction(lisp::cdr(arg_list));
        break;
    }
    case SYS_FUNC_GT:
    case SYS_FUNC_LT:
    case SYS_FUNC_GE:
    case SYS_FUNC_LE:
    {
        int32_t n1 = lnumber_value(lisp::eval(lisp::car(arg_list)));
        int32_t n2 = lnumber_value(lisp::eval(lisp::cadr(arg_list)));
        bool test = fun_number == SYS_FUNC_GT ? n1 > n2 :
                    fun_number == SYS_FUNC_LT ? n1 < n2 :
                    fun_number == SYS_FUNC_GE ? n1 >= n2 :
                    fun_number == SYS_FUNC_LE ? n1 <= n2 : false;
        ret = test ? lisp::sym::true_ : nullptr;
        break;
    }
    case SYS_FUNC_TMP_SPACE:
        tmp_space();
        ret = lisp::sym::true_;
        break;
    case SYS_FUNC_PERM_SPACE:
        perm_space();
        ret = lisp::sym::true_;
        break;
    case SYS_FUNC_SYMBOL_NAME:
    {
        LSymbol *symb = (LSymbol *)lisp::eval(lisp::car(arg_list));
        ASSERT_TYPE(symb, L_SYMBOL, "not a symbol (symbol-name)");
        ret = symb->m_name;
        break;
    }
    case SYS_FUNC_TRACE:
        trace_level++;
        if (arg_list)
            trace_print_level = lnumber_value(lisp::eval(lisp::car(arg_list)));
        ret = lisp::sym::true_;
        break;
    case SYS_FUNC_UNTRACE:
        if (trace_level > 0)
        {
            trace_level--;
            ret = lisp::sym::true_;
        }
        else
            ret = NULL;
        break;
    case SYS_FUNC_DIGSTR:
    {
        char tmp[50], *tp;
        int32_t num = lnumber_value(lisp::eval(lisp::car(arg_list)));
        int32_t dig = lnumber_value(lisp::eval(lisp::cadr(arg_list)));
        tp = tmp + 49;
        *(tp--) = 0;
        while (num)
        {
            *(tp--) = '0' + (num % 10);
            num /= 10;
            dig--;
        }
        while (dig--)
            *(tp--) = '0';
        ret = LString::Create(tp + 1);
        break;
    }
    case SYS_FUNC_LOCAL_LOAD:
    case SYS_FUNC_LOAD:
    case SYS_FUNC_COMPILE_FILE:
    {
        LObject *fn = lisp::eval(lisp::car(arg_list));
        PtrRef r1(fn);
        char *st = lstring_value(fn);
        bFILE *fp;
        if (fun_number == SYS_FUNC_LOCAL_LOAD)
        {
            // A special test for gamma.lsp
            if (strcmp(st, "gamma.lsp") == 0)
            {
                char *gammapath;
                gammapath = (char *)malloc(strlen(get_save_filename_prefix()) + 9 + 1);
                sprintf(gammapath, "%sgamma.lsp", get_save_filename_prefix());
                fp = new jFILE(gammapath, "rb");
                free(gammapath);
            }
            else
                fp = new jFILE(st, "rb");
        }
        else
            fp = open_file(st, "rb");

        if (fp->open_failure())
        {
            delete fp;
            if (DEFINEDP(((LSymbol *)load_warning)->GetValue())
                 && ((LSymbol *)load_warning)->GetValue())
                dprintf("Warning : file %s does not exist\n", st);
            ret = NULL;
        }
        else
        {
            size_t l = fp->file_size();
            char *s = (char *)malloc(l + 1);
            if (!s)
            {
                printf("Malloc error in load_script\n");
                exit(0);
            }

            fp->read(s, l);
            s[l] = 0;
            delete fp;
            char const *cs = s;
#ifndef NO_LIBS
            char msg[100];
            sprintf(msg, "(load \"%s\")", st);
            if (stat_man)
                stat_man->push(msg, NULL);
            crc_manager.GetFileNumber(st); // make sure this file gets crc'ed
#endif
            LObject *compiled_form = NULL;
            PtrRef r11(compiled_form);
            while (!end_of_program(cs))  // see if there is anything left to compile and run
            {
#ifndef NO_LIBS
                if (stat_man)
                    stat_man->update((cs - s) * 100 / l);
#endif
                void *m = LSpace::Tmp.Mark();
                compiled_form = lisp::compile(cs);
                lisp::eval(compiled_form);
                compiled_form = NULL;
                LSpace::Tmp.Restore(m);
            }
#ifndef NO_LIBS
            if (stat_man)
            {
                stat_man->update(100);
                stat_man->pop();
            }
#endif
            free(s);
            ret = fn;
        }
        break;
    }
    case SYS_FUNC_ABS:
        ret = LNumber::Create(lol::abs(lnumber_value(lisp::eval(lisp::car(arg_list)))));
        break;
    case SYS_FUNC_MIN:
    {
        int32_t x = lnumber_value(lisp::eval(lisp::car(arg_list)));
        int32_t y = lnumber_value(lisp::eval(lisp::cadr(arg_list)));
        ret = LNumber::Create(x < y ? x : y);
        break;
    }
    case SYS_FUNC_MAX:
    {
        int32_t x = lnumber_value(lisp::eval(lisp::car(arg_list)));
        int32_t y = lnumber_value(lisp::eval(lisp::cadr(arg_list)));
        ret = LNumber::Create(x > y ? x : y);
        break;
    }
    case SYS_FUNC_BACKQUOTE:
        ret = (LObject *)backquote_eval(lisp::car(arg_list));
        break;
    case SYS_FUNC_COMMA:
        lisp::print(arg_list);
        lbreak("comma is illegal outside of backquote\n");
        exit(0);
        break;
    case SYS_FUNC_NTH:
    {
        int32_t x = lnumber_value(lisp::eval(lisp::car(arg_list)));
        ret = lisp::nth(x, lisp::eval(lisp::cadr(arg_list)));
        break;
    }
    case SYS_FUNC_RESIZE_TMP:
        // Deprecated and useless
        break;
    case SYS_FUNC_RESIZE_PERM:
        // Deprecated and useless
        break;
    case SYS_FUNC_COS:
        ret = LFixedPoint::Create(lisp::cos(lnumber_value(lisp::eval(lisp::car(arg_list)))));
        break;
    case SYS_FUNC_SIN:
        ret = LFixedPoint::Create(lisp::sin(lnumber_value(lisp::eval(lisp::car(arg_list)))));
        break;
    case SYS_FUNC_ATAN2:
    {
        int32_t y = (lnumber_value(lisp::eval(lisp::car(arg_list))));
        int32_t x = (lnumber_value(lisp::eval(lisp::cadr(arg_list))));
        ret = LNumber::Create(lisp::atan2(y, x));
        break;
    }
    case SYS_FUNC_ENUM:
    {
        LSpace *sp = LSpace::Current;
        LSpace::Current = &LSpace::Perm;
        int32_t x = 0;
        while (arg_list)
        {
            LObject *sym = lisp::eval(lisp::car(arg_list));
            PtrRef r1(sym);
            switch (item_type(sym))
            {
            case L_SYMBOL:
            {
                LObject *tmp = LNumber::Create(x);
                ((LSymbol *)sym)->m_value = tmp;
                break;
            }
            case L_CONS_CELL:
            {
                LObject *s = lisp::eval(lisp::car(sym));
                PtrRef r2(s);
#ifdef TYPE_CHECKING
                if (item_type(s) != L_SYMBOL)
                {
                    lisp::print(arg_list);
                    lbreak("expecting (symbol value) for enum\n");
                    exit(0);
                }
#endif
                x = lnumber_value(lisp::eval(lisp::cadr(sym)));
                LObject *tmp = LNumber::Create(x);
                ((LSymbol *)sym)->m_value = tmp;
                break;
            }
            default:
                lisp::print(arg_list);
                lbreak("expecting symbol or (symbol value) in enum\n");
                exit(0);
            }
            arg_list = (LList *)lisp::cdr(arg_list);
            x++;
        }
        LSpace::Current = sp;
        break;
    }
    case SYS_FUNC_QUIT:
        exit(0);
        break;
    case SYS_FUNC_EVAL:
        ret = lisp::eval(lisp::eval(lisp::car(arg_list)));
        break;
    case SYS_FUNC_BREAK:
        lbreak("User break");
        break;
    case SYS_FUNC_MOD:
    {
        int32_t x = lnumber_value(lisp::eval(lisp::car(arg_list)));
        int32_t y = lnumber_value(lisp::eval(lisp::cadr(arg_list)));
        if (y == 0)
        {
            lbreak("mod: division by zero\n");
            y = 1;
        }
        ret = LNumber::Create(x % y);
        break;
    }
#if 0
    case SYS_FUNC_WRITE_PROFILE:
    {
        char *fn = lstring_value(lisp::eval(lisp::car(arg_list)));
        FILE *fp = fopen(fn, "wb");
        if (!fp)
            lbreak("could not open %s for writing", fn);
        else
        {
            for (void *s = symbol_list; s; s = lisp::cdr(s))
                fprintf(fp, "%8d  %s\n", ((LSymbol *)(lisp::car(s)))->call_counter,
                        lstring_value(((LSymbol *)(lisp::car(s)))->m_name));
            fclose(fp);
        }
        break;
    }
#endif
    case SYS_FUNC_FOR:
    {
        LSymbol *bind_var = (LSymbol *)lisp::car(arg_list);
        PtrRef r1(bind_var);
        if (item_type(bind_var) != L_SYMBOL)
        {
            lbreak("expecting for iterator to be a symbol\n");
            exit(1);
        }
        arg_list = (LList *)lisp::cdr(arg_list);

        if (lisp::car(arg_list) != lisp::sym::in)
        {
            lbreak("expecting in after 'for iterator'\n");
            exit(1);
        }
        arg_list = (LList *)lisp::cdr(arg_list);

        LObject *ilist = lisp::eval(lisp::car(arg_list));
        PtrRef r2(ilist);
        arg_list = (LList *)lisp::cdr(arg_list);

        if (lisp::car(arg_list) != lisp::sym::do_)
        {
            lbreak("expecting do after 'for iterator in list'\n");
            exit(1);
        }
        arg_list = (LList *)lisp::cdr(arg_list);

        LObject *block = NULL;
        PtrRef r3(block);
        PtrRef r4(ret); // Required to protect from the last SetValue call
        l_user_stack.push(bind_var->GetValue());  // save old symbol value
        while (ilist)
        {
            bind_var->SetValue((LObject *)lisp::car(ilist));
            for (block = arg_list; block; block = lisp::cdr(block))
                ret = lisp::eval(lisp::car(block));
            ilist = lisp::cdr(ilist);
        }
        bind_var->SetValue((LObject *)l_user_stack.pop(1)); // restore value
        break;
    }
    case SYS_FUNC_OPEN_FILE:
    {
        LObject *str1 = lisp::eval(lisp::car(arg_list));
        PtrRef r1(str1);
        LObject *str2 = lisp::eval(lisp::cadr(arg_list));

        bFILE *old_file = current_print_file;
        current_print_file = open_file(lstring_value(str1),
                                       lstring_value(str2));

        if (!current_print_file->open_failure())
        {
            while (arg_list)
            {
                ret = lisp::eval(lisp::car(arg_list));
                arg_list = (LList *)lisp::cdr(arg_list);
            }
        }
        delete current_print_file;
        current_print_file = old_file;
        break;
    }
    case SYS_FUNC_BIT_AND:
    {
        int32_t first = lnumber_value(lisp::eval(lisp::car(arg_list)));
        arg_list = (LList *)lisp::cdr(arg_list);
        while (arg_list)
        {
            first &= lnumber_value(lisp::eval(lisp::car(arg_list)));
            arg_list = (LList *)lisp::cdr(arg_list);
        }
        ret = LNumber::Create(first);
        break;
    }
    case SYS_FUNC_BIT_OR:
    {
        int32_t first = lnumber_value(lisp::eval(lisp::car(arg_list)));
        arg_list = (LList *)lisp::cdr(arg_list);
        while (arg_list)
        {
            first |= lnumber_value(lisp::eval(lisp::car(arg_list)));
            arg_list = (LList *)lisp::cdr(arg_list);
        }
        ret = LNumber::Create(first);
        break;
    }
    case SYS_FUNC_BIT_XOR:
    {
        int32_t first = lnumber_value(lisp::eval(lisp::car(arg_list)));
        arg_list = (LList *)lisp::cdr(arg_list);
        while (arg_list)
        {
            first ^= lnumber_value(lisp::eval(lisp::car(arg_list)));
            arg_list = (LList *)lisp::cdr(arg_list);
        }
        ret = LNumber::Create(first);
        break;
    }
    case SYS_FUNC_MAKE_ARRAY:
    {
        int32_t l = lnumber_value(lisp::eval(lisp::car(arg_list)));
        if (l >= (2 << 16) || l <= 0)
        {
            lbreak("bad array size %d\n", l);
            exit(0);
        }
        ret = LArray::Create(l, lisp::cdr(arg_list));
        break;
    }
    case SYS_FUNC_AREF:
    {
        int32_t x = lnumber_value(lisp::eval(lisp::cadr(arg_list)));
        ret = ((LArray *)lisp::eval(lisp::car(arg_list)))->Get(x);
        break;
    }
    case SYS_FUNC_IF_1PROGN:
        if (lisp::eval(lisp::car(arg_list)))
            ret = (LObject *)eval_block(lisp::cadr(arg_list));
        else
            ret = lisp::eval(lisp::caddr(arg_list));
        break;
    case SYS_FUNC_IF_2PROGN:
        if (lisp::eval(lisp::car(arg_list)))
            ret = lisp::eval(lisp::cadr(arg_list));
        else
            ret = (LObject *)eval_block(lisp::caddr(arg_list));

        break;
    case SYS_FUNC_IF_12PROGN:
        if (lisp::eval(lisp::car(arg_list)))
            ret = (LObject *)eval_block(lisp::cadr(arg_list));
        else
            ret = (LObject *)eval_block(lisp::caddr(arg_list));
        break;
    case SYS_FUNC_EQ0:
    {
        LObject *v = lisp::eval(lisp::car(arg_list));
        if (item_type(v) != L_NUMBER || (((LNumber *)v)->m_num != 0))
            ret = NULL;
        else
            ret = lisp::sym::true_;
        break;
    }
    case SYS_FUNC_PREPORT:
    {
#ifdef L_PROFILE
        char *s = lstring_value(lisp::eval(lisp::car(arg_list)));
        preport(s);
#endif
        break;
    }
    case SYS_FUNC_SEARCH:
    {
        LObject *arg1 = lisp::eval(lisp::car(arg_list));
        PtrRef r1(arg1); // protect this reference
        arg_list = (LList *)lisp::cdr(arg_list);
        char *haystack = lstring_value(lisp::eval(lisp::car(arg_list)));
        char *needle = lstring_value(arg1);

        char *find = strstr(haystack, needle);
        ret = find ? LNumber::Create(find - haystack) : NULL;
        break;
    }
    case SYS_FUNC_ELT:
    {
        LObject *arg1 = lisp::eval(lisp::car(arg_list));
        PtrRef r1(arg1); // protect this reference
        arg_list = (LList *)lisp::cdr(arg_list);
        int32_t x = lnumber_value(lisp::eval(lisp::car(arg_list)));
        char *st = lstring_value(arg1);
        if (x < 0 || x >= (int32_t)strlen(st))
        {
            lbreak("elt: out of range of string\n");
            ret = NULL;
        }
        else
            ret = LChar::Create(st[x]);
        break;
    }
    case SYS_FUNC_LISTP:
    {
        LObject *tmp = lisp::eval(lisp::car(arg_list));
        ltype t = item_type(tmp);
        ret = (t == L_CONS_CELL) ? lisp::sym::true_ : NULL;
        break;
    }
    case SYS_FUNC_NUMBERP:
    {
        LObject *tmp = lisp::eval(lisp::car(arg_list));
        ltype t = item_type(tmp);
        ret = (t == L_NUMBER || t == L_FIXED_POINT) ? lisp::sym::true_ : NULL;
        break;
    }
    case SYS_FUNC_DO:
    {
        LObject *init_var = lisp::car(arg_list);
        PtrRef r1(init_var);
        int ustack_start = l_user_stack.m_size; // restore stack at end
        LSymbol *sym = NULL;
        PtrRef r2(sym);

        // check to make sure iter vars are symbol and push old values
        for (init_var = lisp::car(arg_list); init_var; init_var = lisp::cdr(init_var))
        {
            sym = (LSymbol *)lisp::caar(init_var);
            if (item_type(sym) != L_SYMBOL)
            {
                lbreak("expecting symbol name for iteration var\n");
                exit(0);
            }
            l_user_stack.push(sym->GetValue());
        }

        void **do_evaled = l_user_stack.sdata + l_user_stack.m_size;
        // push all of the init forms, so we can set the symbol
        for (init_var = lisp::car(arg_list); init_var; init_var = lisp::cdr(init_var))
            l_user_stack.push(lisp::eval(lisp::cadar((init_var))));

        // now set all the symbols
        for (init_var = lisp::car(arg_list); init_var; init_var = lisp::cdr(init_var))
        {
            sym = (LSymbol *)lisp::caar(init_var);
            sym->SetValue((LObject *)*do_evaled);
            do_evaled++;
        }

        for (int i = 0; !i; ) // set i to 1 when terminate conditions are met
        {
            i = lisp::eval(lisp::caadr(arg_list)) != NULL;
            if (!i)
            {
                eval_block(lisp::cddr(arg_list));
                for (init_var = lisp::car(arg_list); init_var; init_var = lisp::cdr(init_var))
                    lisp::eval(lisp::caddar(init_var));
            }
        }

        ret = lisp::eval(lisp::cadadr(arg_list));

        // restore old values for symbols
        do_evaled = l_user_stack.sdata + ustack_start;
        for (init_var = lisp::car(arg_list); init_var; init_var = lisp::cdr(init_var))
        {
            sym = (LSymbol *)lisp::caar(init_var);
            sym->SetValue((LObject *)*do_evaled);
            do_evaled++;
        }

        l_user_stack.m_size = ustack_start;
        break;
    }
    case SYS_FUNC_GC:
        lisp::collect_space(LSpace::Current, 0);
        break;
    case SYS_FUNC_SCHAR:
    {
        char *s = lstring_value(lisp::eval(lisp::car(arg_list)));
        arg_list = (LList *)lisp::cdr(arg_list);
        int32_t x = lnumber_value(lisp::eval(lisp::car(arg_list)));

        if (x < 0 || x >= (int32_t)strlen(s))
        {
            lbreak("SCHAR: index %d out of bounds\n", x);
            exit(0);
        }
        ret = LChar::Create(s[x]);
        break;
    }
    case SYS_FUNC_SYMBOLP:
    {
        LObject *tmp = lisp::eval(lisp::car(arg_list));
        ret = (item_type(tmp) == L_SYMBOL) ? lisp::sym::true_ : NULL;
        break;
    }
    case SYS_FUNC_NUM2STR:
    {
        char str[20];
        sprintf(str, "%ld", (long int)lnumber_value(lisp::eval(lisp::car(arg_list))));
        ret = LString::Create(str);
        break;
    }
    case SYS_FUNC_NCONC:
    {
        LObject *l1 = lisp::eval(lisp::car(arg_list));
        PtrRef r1(l1);
        arg_list = (LList *)lisp::cdr(arg_list);
        LObject *first = l1, *next;
        PtrRef r2(first);

        if (!l1)
        {
            l1 = first = lisp::eval(lisp::car(arg_list));
            arg_list = (LList *)lisp::cdr(arg_list);
        }

        if (item_type(l1) != L_CONS_CELL)
        {
            lisp::print(l1);
            lbreak("first arg should be a list\n");
        }

        do
        {
            next = l1;
            while (next)
            {
                l1 = next;
                next = lisp::cdr(next);
            }
            LObject *tmp = lisp::eval(lisp::car(arg_list));
            ((LList *)l1)->m_cdr = tmp;
            arg_list = (LList *)lisp::cdr(arg_list);
        } while (arg_list);
        ret = first;
        break;
    }
    case SYS_FUNC_FIRST:   ret = lisp::eval(lisp::nth(0, lisp::car(arg_list))); break;
    case SYS_FUNC_SECOND:  ret = lisp::eval(lisp::nth(1, lisp::car(arg_list))); break;
    case SYS_FUNC_THIRD:   ret = lisp::eval(lisp::nth(2, lisp::car(arg_list))); break;
    case SYS_FUNC_FOURTH:  ret = lisp::eval(lisp::nth(3, lisp::car(arg_list))); break;
    case SYS_FUNC_FIFTH:   ret = lisp::eval(lisp::nth(4, lisp::car(arg_list))); break;
    case SYS_FUNC_SIXTH:   ret = lisp::eval(lisp::nth(5, lisp::car(arg_list))); break;
    case SYS_FUNC_SEVENTH: ret = lisp::eval(lisp::nth(6, lisp::car(arg_list))); break;
    case SYS_FUNC_EIGHTH:  ret = lisp::eval(lisp::nth(7, lisp::car(arg_list))); break;
    case SYS_FUNC_NINTH:   ret = lisp::eval(lisp::nth(8, lisp::car(arg_list))); break;
    case SYS_FUNC_TENTH:   ret = lisp::eval(lisp::nth(9, lisp::car(arg_list))); break;
    case SYS_FUNC_SUBSTR:
    {
        int32_t x1 = lnumber_value(lisp::eval(lisp::car(arg_list)));
        int32_t x2 = lnumber_value(lisp::eval(lisp::cadr(arg_list)));
        LObject *st = lisp::eval(lisp::caadr(arg_list));
        PtrRef r1(st);

        if (x1 < 0 || x1 > x2 || x2 >= (int32_t)strlen(lstring_value(st)))
            lbreak("substr: bad x1 or x2 value");

        LString *s = LString::Create(x2 - x1 + 2);
        if (x2 - x1)
            memcpy(lstring_value(s), lstring_value(st) + x1, x2 - x1 + 1);

        lstring_value(s)[x2 - x1 + 1] = 0;
        ret = s;
        break;
    }
    case 99:
    {
        LObject *r = NULL, *rstart = NULL;
        PtrRef r1(r), r2(rstart);
        while (arg_list)
        {
            LObject *q = lisp::eval(lisp::car(arg_list));
            if (!rstart)
                rstart = q;
            while (r && lisp::cdr(r))
                r = lisp::cdr(r);
            lisp::cdr(r) = q;
            arg_list = (LList *)lisp::cdr(arg_list);
        }
        ret = rstart;
        break;
    }
    default:
        dprintf("Undefined system function number %d\n", fun_number);
        break;
    }

    return ret;
}

void tmp_space()
{
    LSpace::Current = &LSpace::Tmp;
}

void perm_space()
{
    LSpace::Current = &LSpace::Perm;
}

/* PtrRef check: OK */
LObject *LSymbol::EvalUserFunction(LList *arg_list)
{
    LObject *ret = NULL;
    PtrRef ref1(ret);

    ASSERT_TYPE(this, L_SYMBOL, "EVAL: not a function name (not symbol either)");
#ifdef L_PROFILE
    Timer t;
#endif

    LUserFunction *fun = (LUserFunction *)m_function;

#ifdef TYPE_CHECKING
    if (item_type(fun) != L_USER_FUNCTION)
    {
        lisp::print(this);
        lbreak("is not a user defined function\n");
    }
#endif

    LList *fun_arg_list = fun->arg_list;
    LList *block_list = fun->block_list;
    PtrRef r9(block_list), r10(fun_arg_list);

    // mark the start start, so we can restore when done
    long stack_start = l_user_stack.m_size;

    // first push all of the old symbol values
    LObject *f_arg = NULL;
    PtrRef r18(f_arg);
    PtrRef r19(arg_list);

    for (f_arg = fun_arg_list; f_arg; f_arg = lisp::cdr(f_arg))
    {
        LSymbol *s = (LSymbol *)lisp::car(f_arg);
        l_user_stack.push(s->m_value);
    }

    // open block so that local vars aren't saved on the stack
    {
        int new_start = l_user_stack.m_size;
        int i = new_start;
        // now push all the values we wish to gather
        for (f_arg = fun_arg_list; f_arg; f_arg = lisp::cdr(f_arg))
        {
            if (!arg_list)
            {
                lisp::print(this);
                lbreak("too few parameter to function\n");
                exit(0);
            }
            l_user_stack.push(lisp::eval(lisp::car(arg_list)));
            arg_list = (LList *)lisp::cdr(arg_list);
        }

        // now store all the values and put them into the symbols
        for (f_arg = fun_arg_list; f_arg; f_arg = lisp::cdr(f_arg))
            ((LSymbol *)lisp::car(f_arg))->SetValue((LObject *)l_user_stack.sdata[i++]);

        l_user_stack.m_size = new_start;
    }

    if (f_arg)
    {
        lisp::print(this);
        lbreak("too many parameter to function\n");
        exit(0);
    }

    // now evaluate the function block
    while (block_list)
    {
        ret = lisp::eval(lisp::car(block_list));
        block_list = (LList *)lisp::cdr(block_list);
    }

    long cur_stack = stack_start;
    for (f_arg = fun_arg_list; f_arg; f_arg = lisp::cdr(f_arg))
        ((LSymbol *)lisp::car(f_arg))->SetValue((LObject *)l_user_stack.sdata[cur_stack++]);

    l_user_stack.m_size = stack_start;

#ifdef L_PROFILE
    sym->time_taken += t.Get();
#endif

    return ret;
}

/* PtrRef check: OK */
LObject *lisp::eval(LObject *obj)
{
    PtrRef ref1(obj);

    maxevaldepth = lol::max(maxevaldepth, ++evaldepth);

    int tstart = trace_level;

    if (trace_level)
    {
        if (trace_level <= trace_print_level)
        {
            dprintf("%d (%d, %d, %d) TRACE : ", trace_level,
                    LSpace::Perm.GetFree(), LSpace::Tmp.GetFree(),
                    PtrRef::stack.m_size);
            lisp::print(obj);
            dprintf("\n");
        }
        trace_level++;
    }

    LObject *ret = NULL;

    if (obj)
    {
        switch (item_type(obj))
        {
        case L_BAD_CELL:
            lbreak("error: eval on a bad cell\n");
            exit(0);
            break;
        case L_CHARACTER:
        case L_STRING:
        case L_NUMBER:
        case L_POINTER:
        case L_FIXED_POINT:
            ret = obj;
            break;
        case L_SYMBOL:
            if (obj == lisp::sym::true_)
                ret = obj;
            else
            {
                ret = ((LSymbol *)obj)->GetValue();
                if (item_type(ret) == L_OBJECT_VAR)
                    ret = (LObject *)l_obj_get(((LObjectVar *)ret)->m_index);
            }
            break;
        case L_CONS_CELL:
            ret = ((LSymbol *)lisp::car(obj))->EvalFunction(lisp::cdr(obj));
            break;
        default :
            fprintf(stderr, "shouldn't happen\n");
            break;
        }
    }

    if (tstart)
    {
        trace_level--;
        if (trace_level <= trace_print_level)
            dprintf("%d (%d, %d, %d) TRACE ==> ", trace_level,
                    LSpace::Perm.GetFree(), LSpace::Tmp.GetFree(),
                    PtrRef::stack.m_size);
        lisp::print(ret);
        dprintf("\n");
    }

/*  l_user_stack.push(ret);
  lisp::collect_space(&LSpace::Perm);
  ret=l_user_stack.pop(1);  */
    --evaldepth;

    return ret;
}

void lisp::init()
{
    symbol_root = nullptr;
    total_user_functions = 0;

    LSpace::Tmp.m_free = LSpace::Tmp.m_data = (uint8_t *)malloc(0x1000);
    LSpace::Tmp.m_size = 0x1000;
    LSpace::Tmp.m_name = "temporary space";

    LSpace::Perm.m_free = LSpace::Perm.m_data = (uint8_t *)malloc(0x1000);
    LSpace::Perm.m_size = 0x1000;
    LSpace::Perm.m_name = "permanent space";

    LSpace::Gc.m_name = "garbage space";

    LSpace::Current = &LSpace::Perm;

    init_constants();

    for(size_t i = 0; i < sizeof(sys_funcs) / sizeof(*sys_funcs); i++)
        add_sys_function(sys_funcs[i].name,
                         sys_funcs[i].min_args, sys_funcs[i].max_args, i);
    clisp_init();
    LSpace::Current = &LSpace::Tmp;
    dprintf("Lisp: %d symbols defined, %d system functions, "
            "%d pre-compiled functions\n", symbol_count,
            sizeof(sys_funcs) / sizeof(*sys_funcs), total_user_functions);
}

void lisp::uninit()
{
    free(LSpace::Tmp.m_data);
    free(LSpace::Perm.m_data);

    DeleteAllSymbols(symbol_root);
    symbol_root = nullptr;
    symbol_count = 0;
}

void LSpace::Clear()
{
    m_free = m_data;
}

LString *LSymbol::GetName()
{
    ASSERT_TYPE(this, L_SYMBOL, "not a symbol");
    return m_name;
}

void LSymbol::SetNumber(long num)
{
    ASSERT_TYPE(this, L_SYMBOL, "not a symbol");
    if (m_value != lisp::obj::undefined && item_type(m_value) == L_NUMBER)
        ((LNumber *)m_value)->m_num = num;
    else
        m_value = LNumber::Create(num);
}

void LSymbol::SetValue(LObject *val)
{
    ASSERT_TYPE(this, L_SYMBOL, "not a symbol");
    m_value = val;
}

LObject *LSymbol::GetFunction()
{
    ASSERT_TYPE(this, L_SYMBOL, "not a symbol");
    return m_function;
}

LObject *LSymbol::GetValue()
{
    ASSERT_TYPE(this, L_SYMBOL, "not a symbol");
    return m_value;
}

