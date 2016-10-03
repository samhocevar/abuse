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

#ifdef NO_LIBS
#   include "fakelib.h"
#endif

#include "lisp/lisp.h"
#include "lisp/gc.h"
#include "lisp/symbols.h"

void *comp_optimize(void *list)
{
  void *return_val=list;
  PtrRef r1(list);
  if (list)
  {
    if (lisp::car(list)==lisp::sym::if_)
    {
      void *eval1 = lisp::caddr(list);
      PtrRef r2(eval1);
      void *eval2 = lisp::cadddr(list);
      PtrRef r3(eval2);

      void *ret=nullptr;
      PtrRef r4(ret);
      if (lisp::car(list)==lisp::sym::eq && (lisp::cadr(list))==lisp::sym::zero)  //  simplify (eq 0 x) -> (eq0 x)
      {
    push_onto_list(lisp::caddr(list),ret);
    push_onto_list(lisp::sym::eq0,ret);
    return_val=comp_optimize(ret);
      } else if (lisp::car(list)==lisp::sym::eq &&
         (lisp::caddr(list)==lisp::sym::zero)) //simplify (eq x 0)-> (eq0 x)
      {
    push_onto_list(lisp::cadr(list),ret);
    push_onto_list(lisp::sym::eq0,ret);
    return_val=comp_optimize(ret);
      } else if (lisp::caadr(list)==lisp::sym::not_)  // simplify (if (not y) x z) -> (if y z x)
      {
    push_onto_list(lisp::caddr(list),ret);
    push_onto_list(lisp::cadddr(list),ret);
    push_onto_list(lisp::cadadr(list),ret);
    push_onto_list(lisp::sym::if_,ret);
    return_val=comp_optimize(ret);
      }
      else if (lisp::car(eval1)==lisp::sym::progn && (eval2==nullptr ||
                         item_type(eval2)!=L_CONS_CELL))
      {
    push_onto_list(eval2,ret);
    push_onto_list(lisp::cdr(eval1),ret);
    push_onto_list(lisp::cadr(list),ret);
    push_onto_list(lisp::sym::if_1progn,ret);
    return_val=comp_optimize(ret);
      } else if (lisp::car(eval1)==lisp::sym::progn && lisp::car(eval2)==lisp::sym::progn)
      {
    push_onto_list(lisp::cdr(eval2),ret);
    push_onto_list(lisp::cdr(eval1),ret);
    push_onto_list(lisp::cadr(list),ret);
    push_onto_list(lisp::sym::if_12progn,ret);
    return_val=comp_optimize(ret);
      } else if (lisp::car(eval2)==lisp::sym::progn)
      {
    push_onto_list(lisp::cdr(eval2),ret);
    push_onto_list(eval1,ret);
    push_onto_list(lisp::cadr(list),ret);
    push_onto_list(lisp::sym::if_2progn,ret);
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
    lisp::obj::undefined = tmp;
    // Collection problems result if we don't do this
    tmp->m_function = nullptr;
    tmp->m_value = nullptr;

    sym::true_ = lisp::make_sym("T");

    sym::list = lisp::make_sym("list");
    sym::string = lisp::make_sym("string");
    sym::quote = lisp::make_sym("quote");
    sym::backquote = lisp::make_sym("backquote");
    sym::comma = lisp::make_sym("comma");
    sym::in = lisp::make_sym("in");
    sym::do_ = lisp::make_sym("do");
    sym::aref = lisp::make_sym("aref");
    colon_initial_contents = lisp::make_sym(":initial-contents");
    colon_initial_element = lisp::make_sym(":initial-element");

    sym::if_1progn = lisp::make_sym("if-1progn");
    sym::if_2progn = lisp::make_sym("if-2progn");
    sym::if_12progn = lisp::make_sym("if-12progn");
    sym::if_ = lisp::make_sym("if");
    sym::progn = lisp::make_sym("progn");
    sym::not_ = lisp::make_sym("not");
    sym::eq = lisp::make_sym("eq");
    sym::zero = lisp::make_sym("0");
    sym::eq0 = lisp::make_sym("eq0");
    sym::car = lisp::make_sym("car");
    sym::cdr = lisp::make_sym("cdr");
    load_warning = lisp::make_sym("load_warning");
}

