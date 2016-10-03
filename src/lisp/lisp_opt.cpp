/*
 *  Abuse - dark 2D side-scrolling platform game
 *  Copyright (c) 1995 Crack dot Com
 *  Copyright (c) 2005-2013 Sam Hocevar <sam@hocevar.net>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Crack dot Com, by
 *  Jonathan Clark, or by Sam Hocevar.
 */

#if HAVE_CONFIG_H
#   include "config.h"
#endif

#ifdef NO_LIBS
#   include "fakelib.h"
#endif

#include "lisp/lisp.h"
#include "lisp/lisp_gc.h"

LObject *l_undefined;
LSymbol *true_symbol = nullptr, *list_symbol, *string_symbol, *quote_symbol,
     *backquote_symbol, *comma_symbol, *do_symbol, *in_symbol, *aref_symbol,
     *if_symbol, *progn_symbol, *car_symbol, *cdr_symbol;

void *colon_initial_contents, *colon_initial_element,
     *eq_symbol, *zero_symbol, *eq0_symbol, *load_warning;

void *if_1progn,*if_2progn,*if_12progn,*not_symbol;

void *comp_optimize(void *list)
{
  void *return_val=list;
  PtrRef r1(list);
  if (list)
  {
    if (lisp::car(list)==if_symbol)
    {
      void *eval1 = lisp::caddr(list);
      PtrRef r2(eval1);
      void *eval2 = lisp::cadddr(list);
      PtrRef r3(eval2);

      void *ret=nullptr;
      PtrRef r4(ret);
      if (lisp::car(list)==eq_symbol && (lisp::cadr(list))==zero_symbol)  //  simplify (eq 0 x) -> (eq0 x)
      {
    push_onto_list(lisp::caddr(list),ret);
    push_onto_list(eq0_symbol,ret);
    return_val=comp_optimize(ret);
      } else if (lisp::car(list)==eq_symbol &&
         (lisp::caddr(list)==zero_symbol)) //simplify (eq x 0)-> (eq0 x)
      {
    push_onto_list(lisp::cadr(list),ret);
    push_onto_list(eq0_symbol,ret);
    return_val=comp_optimize(ret);
      } else if (lisp::caadr(list)==not_symbol)  // simplify (if (not y) x z) -> (if y z x)
      {
    push_onto_list(lisp::caddr(list),ret);
    push_onto_list(lisp::cadddr(list),ret);
    push_onto_list(lisp::cadadr(list),ret);
    push_onto_list(if_symbol,ret);
    return_val=comp_optimize(ret);
      }
      else if (lisp::car(eval1)==progn_symbol && (eval2==nullptr ||
                         item_type(eval2)!=L_CONS_CELL))
      {
    push_onto_list(eval2,ret);
    push_onto_list(lisp::cdr(eval1),ret);
    push_onto_list(lisp::cadr(list),ret);
    push_onto_list(if_1progn,ret);
    return_val=comp_optimize(ret);
      } else if (lisp::car(eval1)==progn_symbol && lisp::car(eval2)==progn_symbol)
      {
    push_onto_list(lisp::cdr(eval2),ret);
    push_onto_list(lisp::cdr(eval1),ret);
    push_onto_list(lisp::cadr(list),ret);
    push_onto_list(if_12progn,ret);
    return_val=comp_optimize(ret);
      } else if (lisp::car(eval2)==progn_symbol)
      {
    push_onto_list(lisp::cdr(eval2),ret);
    push_onto_list(eval1,ret);
    push_onto_list(lisp::cadr(list),ret);
    push_onto_list(if_2progn,ret);
    return_val=comp_optimize(ret);
      }

    }
  }
  return return_val;
}

void lisp::init_constants()
{
    // This needs to be defined first
    LSymbol *tmp = lisp::make_sym(":UNDEFINED");
    l_undefined = tmp;
    // Collection problems result if we don't do this
    tmp->m_function = nullptr;
    tmp->m_value = nullptr;

    true_symbol = lisp::make_sym("T");

    list_symbol = lisp::make_sym("list");
    string_symbol = lisp::make_sym("string");
    quote_symbol = lisp::make_sym("quote");
    backquote_symbol = lisp::make_sym("backquote");
    comma_symbol = lisp::make_sym("comma");
    in_symbol = lisp::make_sym("in");
    do_symbol = lisp::make_sym("do");
    aref_symbol = lisp::make_sym("aref");
    colon_initial_contents = lisp::make_sym(":initial-contents");
    colon_initial_element = lisp::make_sym(":initial-element");

    if_1progn = lisp::make_sym("if-1progn");
    if_2progn = lisp::make_sym("if-2progn");
    if_12progn = lisp::make_sym("if-12progn");
    if_symbol = lisp::make_sym("if");
    progn_symbol = lisp::make_sym("progn");
    not_symbol = lisp::make_sym("not");
    eq_symbol = lisp::make_sym("eq");
    zero_symbol = lisp::make_sym("0");
    eq0_symbol = lisp::make_sym("eq0");
    car_symbol = lisp::make_sym("car");
    cdr_symbol = lisp::make_sym("cdr");
    load_warning = lisp::make_sym("load_warning");
}

