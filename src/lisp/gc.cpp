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

#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "lisp/lisp.h"
#include "lisp/gc.h"
#include "lisp/stack.h"

/*  Lisp garbage collection: uses copy/free algorithm
    Places to check:
      symbol
        values
    functions
    names
      stack
*/

// Stack where user programs can push data and have it GCed
GrowStack<LObject> l_user_stack(150);

// Stack of user pointers
GrowStack<LObject *> PtrRef::stack(1500);

static size_t reg_ptr_total = 0;
static LObject ***reg_ptr_list = NULL;

static uint8_t *cstart, *cend, *collected_start, *collected_end;
static int gcdepth, maxgcdepth;

LArray *lisp::collect_array(LArray *x)
{
    size_t s = x->m_len;
    LArray *a = lisp::make_array(s, NULL);
    LObject **src = x->GetData();
    LObject **dst = a->GetData();
    for (size_t i = 0; i < s; i++)
        dst[i] = collect_object(src[i]);

    return a;
}

LList *lisp::collect_list(LList *x)
{
    LList *prev = NULL, *first = NULL;

    for (; x && item_type(x) == L_CONS_CELL; )
    {
        LList *p = lisp::make_list();
        LObject *old_car = x->m_car;
        LObject *old_x = x;
        x = (LList *)lisp::cdr(x);
        ((LRedirect *)old_x)->m_type = L_COLLECTED_OBJECT;
        ((LRedirect *)old_x)->m_ref = p;

        p->m_car = collect_object(old_car);

        if (prev)
            prev->m_cdr = p;
        else
            first = p;
        prev = p;
    }
    if (x)
        prev->m_cdr = collect_object(x);

    return first; // we already set the collection pointers
}

LObject *lisp::collect_object(LObject *x)
{
    LObject *ret = x;

    maxgcdepth = lol::max(maxgcdepth, ++gcdepth);

    if ((uint8_t *)x >= cstart && (uint8_t *)x < cend)
    {
        switch (item_type(x))
        {
        case L_BAD_CELL:
            lbreak("error: collecting corrupted cell\n");
            break;
        case L_NUMBER:
            ret = lisp::make_number(((LNumber *)x)->m_num);
            break;
        case L_SYS_FUNCTION:
            ret = new_lisp_sys_function(((LSysFunction *)x)->min_args,
                                        ((LSysFunction *)x)->max_args,
                                        ((LSysFunction *)x)->fun_number);
            break;
        case L_USER_FUNCTION:
        {
            LUserFunction *fun = (LUserFunction *)x;
            LList *arg = (LList *)collect_object(fun->arg_list);
            LList *block = (LList *)collect_object(fun->block_list);
            ret = new_lisp_user_function(arg, block);
            break;
        }
        case L_STRING:
            ret = lisp::make_str(lstring_value(x));
            break;
        case L_CHARACTER:
            ret = lisp::make_char(((LChar *)x)->m_ch);
            break;
        case L_C_FUNCTION:
            ret = new_lisp_c_function(((LSysFunction *)x)->min_args,
                                      ((LSysFunction *)x)->max_args,
                                      ((LSysFunction *)x)->fun_number);
            break;
        case L_C_BOOL:
            ret = new_lisp_c_bool(((LSysFunction *)x)->min_args,
                                  ((LSysFunction *)x)->max_args,
                                  ((LSysFunction *)x)->fun_number);
            break;
        case L_L_FUNCTION:
            ret = new_user_lisp_function(((LSysFunction *)x)->min_args,
                                         ((LSysFunction *)x)->max_args,
                                         ((LSysFunction *)x)->fun_number);
            break;
        case L_POINTER:
            ret = lisp::make_ptr(lpointer_value(x));
            break;
        case L_1D_ARRAY:
            ret = collect_array((LArray *)x);
            break;
        case L_FIXED_POINT:
            ret = lisp::make_fp(lfixed_point_value(x));
            break;
        case L_CONS_CELL:
            ret = collect_list((LList *)x);
            break;
        case L_OBJECT_VAR:
            ret = lisp::make_var(((LObjectVar *)x)->m_index);
            break;
        case L_COLLECTED_OBJECT:
            ret = ((LRedirect *)x)->m_ref;
            break;
        default:
            lbreak("error: collecting bad object 0x%x\n", item_type(x));
            break;
        }
        ((LRedirect *)x)->m_type = L_COLLECTED_OBJECT;
        ((LRedirect *)x)->m_ref = ret;
    }
    else if ((uint8_t *)x < collected_start || (uint8_t *)x >= collected_end)
    {
        // Still need to remap cons_cells lying outside of space, for
        // instance on the stack.
        for (LObject *cell = NULL; x; cell = x, x = lisp::cdr(x))
        {
            if (item_type(x) != L_CONS_CELL)
            {
                if (cell)
                    lisp::cdr(cell) = collect_object(lisp::cdr(cell));
                break;
            }
            lisp::car(x) = collect_object(lisp::car(x));
        }
    }

    --gcdepth;
    return ret;
}

void lisp::collect_symbols(LSymbol *root)
{
    if (!root)
        return;

    root->m_value = collect_object(root->m_value);
    root->m_function = collect_object(root->m_function);
    root->m_name = (LString *)collect_object(root->m_name);
    collect_symbols(root->m_left);
    collect_symbols(root->m_right);
}

void lisp::collect_stacks()
{
    LObject **d = l_user_stack.sdata;
    for (size_t i = 0; i < l_user_stack.m_size; i++, d++)
        *d = collect_object(*d);

    LObject ***d2 = PtrRef::stack.sdata;
    for (size_t i = 0; i < PtrRef::stack.m_size; i++, d2++)
    {
        LObject **ptr = *d2;
        *ptr = collect_object(*ptr);
    }

    LObject ***d3 = reg_ptr_list;
    for (size_t i = 0; i < reg_ptr_total; i++, d3++)
    {
        LObject **ptr = *d3;
        *ptr = collect_object(*ptr);
    }
}

void lisp::collect_space(LSpace *which_space, int grow)
{
    LSpace *sp = LSpace::Current;

    maxgcdepth = gcdepth = 0;

    cstart = which_space->m_data;
    cend = which_space->m_free;
    LSpace::Gc.m_size = which_space->m_size;
    if (grow)
    {
        LSpace::Gc.m_size += which_space->m_size >> 1;
        LSpace::Gc.m_size -= (LSpace::Gc.m_size & 7);
    }
    uint8_t *new_data = (uint8_t *)malloc(LSpace::Gc.m_size);
    LSpace::Current = &LSpace::Gc;
    LSpace::Gc.m_free = LSpace::Gc.m_data = new_data;

    collected_start = new_data;
    collected_end = new_data + LSpace::Gc.m_size;

    collect_symbols(symbol_root);
    collect_stacks();

    free(which_space->m_data);
    which_space->m_data = new_data;
    which_space->m_size = LSpace::Gc.m_size;
    which_space->m_free = new_data + (LSpace::Gc.m_free - LSpace::Gc.m_data);

    LSpace::Current = sp;
}

