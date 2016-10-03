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

#include "common.h"

#include "lisp/lisp.h"
#include "lisp/gc.h"
#include "lisp/symbols.h"

#include "imlib/window.h"
#include "imlib/input.h"
#include "imlib/dprint.h"

#include "chars.h"
#include "game.h"
#include "intsect.h"
#include "id.h"
#include "clisp.h"

#define FADING_FRAMES 26
#define FADING_MAX 32

CharacterType **figures;

int total_weapons = 0;
int *weapon_types = NULL; // maps 0..total_weapons into 'real' weapon type

char const *state_names[MAX_STATE] =
{
    "dead",
    "dieing",
    "stopped",
    "start_run_jump", "run_jump", "run_jump_fall", "end_run_jump",
    "flinch_up", "flinch_down",
    "morph_pose",
    "running"
#if 0
    "start_still_jump", "still_jump", "still_jump_fall", "end_still_jump",

    "morph_pose",

    "walking",

    "weapon_draw",
    "weapon_put_away",
    "weapon_fire",
    "weapon_end_fire",
    "weapon_fire_up",
    "weapon_end_fire_up",

    "blocking",
    "block_recoil",

    "turn_around",
    "flinch_up", "flinch_down", "flinch_back",
    "flinch_air", "flinch_ground", "flinch_getup", "woze",
    "stance"
#endif
};

char const *cflag_names[TOTAL_CFLAGS] =
{
    "hurt_all", "is_weapon", "stoppable", "can_block",
    "hurtable", "pushable", "unlistable",
    "add_front", "cached_in", "need_cache_in", "unactive_shield"
};

char const *ofun_names[TOTAL_OFUNS] =
{
    "ai_fun", "move_fun", "draw_fun", "map_draw_fun", "damage_fun",
    "next_state_fun", "user_fun",
    "constructor", "reload_fun", "get_cache_list_fun",
    "type_change_fun"
};

int CharacterType::add_state(LObject *symbol) // returns index into seq to use
{
    if (item_type(symbol) != L_SYMBOL)
    {
        lisp::print(symbol);
        lbreak("is not a symbol (in def_char)");
        exit(0);
    }

    LObject *val = symbol_value((LSymbol *)symbol);

    int num;
    if (DEFINEDP(val))
    {
        if (item_type(val) != L_NUMBER)
        {
            lisp::print(symbol);
            dprintf("expecting symbol value to be a number, instead got: ");
            lisp::print(val);
            lbreak("");
            exit(0);
        }
        num = lnumber_value(val);
    }
    else
    {
        num = lol::max(ts, MAX_STATE);
        LSpace *sp = LSpace::Current;
        LSpace::Current = &LSpace::Perm;
        ((LSymbol *)symbol)->SetNumber(num);
        LSpace::Current = sp;
    }

    if (num < ts && seq[num])
    {
        lisp::print(symbol);
        lbreak("symbol has been assigned value %d, but value already in use "
               "by state %s\n" "use a different symbol for this state\n",
               lnumber_value(seq_syms[num]->GetValue()),
               lstring_value(seq_syms[num]->GetName()));
        exit(0);
    }
    else if (num >= ts)
    {
        seq = (sequence **)realloc(seq, sizeof(sequence *) * (num + 1));
        seq_syms = (LSymbol **)realloc(seq_syms, sizeof(LSymbol *) * (num + 1));

        memset(&seq[ts], 0, sizeof(sequence *) * ((num + 1) - ts));
        memset(&seq_syms[ts], 0, sizeof(LSymbol *) * ((num + 1) - ts));

        ts = num + 1;
    }

    seq_syms[num] = (LSymbol *)symbol;
    return num;
}

int flinch_state(character_state state)
{
    if (state == flinch_up || state == flinch_down)
        return 1;
    return 0;
}

int CharacterType::cache_in()    // returns false if out of cache memory
{
    if (get_cflag(CFLAG_CACHED_IN))
        return 1;
    cflags |= 1 << CFLAG_CACHED_IN;

    for (int i = 0; i < ts; i++)
        if (seq[i])
            seq[i]->cache_in();

    return 1;
}

void CharacterType::add_sequence(character_state which, sequence *new_seq)
{
    delete seq[which];
    seq[which] = new_seq;
}

void *l_obj_get(long number) // exten lisp function switches on number
{
    CharacterType *t = figures[current_object->otype];
    if (t->tiv <= number || !t->vars[number])
    {
        lbreak("access : variable does not exists for this class\n");
        return 0;
    }
    return LNumber::Create(current_object->lvars[t->var_index[number]]);
}

void l_obj_set(long number, void *arg)  // exten lisp function switches on number
{
  CharacterType *t=figures[current_object->otype];
  if (t->tiv<=number || !t->vars[number])
  {
    lbreak("set : variable does not exists for this class\n");
    return;
  }
  current_object->lvars[t->var_index[number]]=lnumber_value(arg);
}

void l_obj_print(long number)  // exten lisp function switches on number
{
  CharacterType *t=figures[current_object->otype];
  if (t->tiv<=number || !t->vars[number])
  {
    lbreak("access : variable does not exists for this class\n");
    return;
  }
  dprintf("%d",current_object->lvars[t->var_index[number]]);
}

void CharacterType::add_var(void *symbol, void *name)
{
  /* First see if the variable has been defined for another object
     if so report a conflict if any occur */
  LSymbol *s=(LSymbol *)symbol;
  if (DEFINEDP(s->m_value) && (item_type(s->m_value)!=L_OBJECT_VAR))
  {
    lisp::print((LObject *)symbol);
    lbreak("symbol already has a value, cannot instantiate an object varible");
    exit(0);
  } else if (DEFINEDP(s->m_value))
  {
    int index = ((LObjectVar *)s->m_value)->m_index;
    if (index<tiv)
    {
      if (vars[index])
      {
    lbreak("While defining object %s :\n"
           "  var '%s' was previously defined by another\n"
           "  with index %d, but %s has a var listed '%s' with same index\n"
           "  try moving definition of %s before previously declared object",
           lstring_value(((LSymbol *)name)->GetName()),
           lstring_value(((LSymbol *)symbol)->GetName()),
           index,
           lstring_value(((LSymbol *)name)->GetName()),
           lstring_value(((LSymbol *)vars[index])->GetName()),
           lstring_value(((LSymbol *)name)->GetName())
           );
    exit(0);
      } else
      {
    var_index[index]=tv;
    vars[index]=(LSymbol *)symbol;
    tv++;
      }
    } else
    {
      int new_total=index+1;
      vars=(LSymbol **)realloc(vars,sizeof(LSymbol *)*new_total);
      var_index=(short *)realloc(var_index,sizeof(short)*new_total);
      memset(&vars[tiv],0,(new_total-tiv)*sizeof(LSymbol *));
      memset(&var_index[tiv],0,(new_total-tiv)*sizeof(short));
      tiv=new_total;

      var_index[index]=tv;
      vars[index]=(LSymbol *)symbol;
      tv++;
    }
  } else  /** Nope, looks like we have to add the variable ourself and define the assesor funs */
  {
    /* locate a free index in the index list */
    int free_index=tiv;
    for (int i=0; i<tiv; i++)
      if (!vars[i] && i<tiv) free_index=i;
    if (free_index==tiv)
    {
      int new_total=free_index+1;
      vars=(LSymbol **)realloc(vars,sizeof(LSymbol *)*new_total);
      var_index=(short *)realloc(var_index,sizeof(short)*new_total);
      memset(&vars[tiv],0,(new_total-tiv)*sizeof(LSymbol *));
      memset(&var_index[tiv],0,(new_total-tiv)*sizeof(short));
      tiv=new_total;
    }

    /* create the var and add to var list */
    LSpace *sp = LSpace::Current;
    LSpace::Current = &LSpace::Perm;

    add_c_object(symbol,free_index);

    vars[free_index]=(LSymbol *)symbol;
    var_index[free_index]=tv;
    tv++;
    LSpace::Current=sp;
  }
}


long CharacterType::isa_var_name(char *name)
{
  for (int i=0; i<TOTAL_OBJECT_VARS; i++)
    if (!strcmp(object_descriptions[i].name,name))
      return 1;
  for (int i=0; i<tiv; i++)
    if (!strcmp(lstring_value(((LSymbol *)vars[i])->GetName()),name))
      return 1;
  return 0;
}

CharacterType::CharacterType(LList *args, LSymbol *name)
{
    PtrRef r2(args);
    ts=tv=0;
    seq=NULL;
    seq_syms=NULL;
    vars=NULL;
    var_index=NULL;
    tiv=0;

    LSymbol *l_abil =   lisp::make_sym("abilities");
    LSymbol *l_funs =   lisp::make_sym("funs");
    LSymbol *l_states = lisp::make_sym("states");
    LSymbol *l_flags =  lisp::make_sym("flags");
    LSymbol *l_range =  lisp::make_sym("range");
    LSymbol *l_draw_range = lisp::make_sym("draw_range");
    LSymbol *l_fields = lisp::make_sym("fields");
    LSymbol *l_logo =   lisp::make_sym("logo");
    LSymbol *l_vars =   lisp::make_sym("vars");

    memset(fun_table,0,sizeof(fun_table));     // destory all hopes of fun
    fields=NULL;
    cflags=0;
    morph_mask=-1;
    morph_power=0;
    total_fields=0;
    logo=-1;
    rangex=rangey=0;
    draw_rangex=draw_rangey=0;

    for (int i=0; i<TOTAL_ABILITIES; i++)
        abil[i]=get_ability_default((ability)i);
    LObject *field = args;
    PtrRef r7(field);
    for (; field; field=lisp::cdr(field))
    {
        LObject *f=lisp::caar(field);
        PtrRef r1(f);

        if (f==l_abil)
        {
            LList *l = (LList *)lisp::cdar(field);
            PtrRef r4(l);
            for (int i=0; i<TOTAL_ABILITIES; i++)
            {
                Cell *ab = l->Assoc(lisp::make_sym(ability_names[i]));
                PtrRef r5(ab);
                if (!NILP(ab))
                    abil[i] = lnumber_value(lisp::eval(lisp::cadr(ab)));
            }
        } else if (f==l_funs)
        {
            LList *l = (LList *)lisp::cdar(field);
            PtrRef r4(l);
            for (int i=0; i<TOTAL_OFUNS; i++)
            {
                Cell *ab = l->Assoc(lisp::make_sym(ofun_names[i]));
                PtrRef r5(ab);
                if (!NILP(ab) && lisp::cadr(ab))
                    fun_table[i]=lisp::cadr(ab);
            }
        } else if (f==l_flags)
        {
            LList *l = (LList *)lisp::cdar(field);
            PtrRef r4(l);
            for (int i=0; i<TOTAL_CFLAGS; i++)
            {

                Cell *ab = l->Assoc(lisp::make_sym(cflag_names[i]));
                PtrRef r5(ab);
                if (!NILP(ab) && lisp::eval(lisp::cadr(ab)))
                    cflags|=(1<<i);
            }

            if (get_cflag(CFLAG_IS_WEAPON))  // if this is a weapon add to weapon array
            {
                total_weapons++;
                weapon_types=(int *)realloc(weapon_types,sizeof(int)*total_weapons);
                weapon_types[total_weapons-1]=total_objects;
            }
        } else if (f==l_range)
        {
            rangex = lnumber_value(lisp::eval(lisp::cadar(field)));
            rangey = lnumber_value(lisp::eval(lisp::caddar(field)));
        } else if (f==l_draw_range)
        {
            draw_rangex = lnumber_value(lisp::eval(lisp::cadar(field)));
            draw_rangey = lnumber_value(lisp::eval(lisp::caddar(field)));
        } else if (f==l_states)
        {
            LObject *l=lisp::cdar(field);
            PtrRef r4(l);
            char fn[100];
            strcpy(fn,lstring_value(lisp::eval(lisp::car(l)))); l=lisp::cdr(l);
            while (l)
            {
                int index;
                void *e;
                sequence *mem;
                index = add_state(lisp::caar(l));
                e = lisp::eval(lisp::cadar(l));
                mem = new sequence(fn,e,NULL);
                seq[index]=mem;
                l=lisp::cdr(l);
            }
        } else if (f==l_fields)
        {
            void *mf=lisp::cdar(field);
            PtrRef r4(mf);
            while (!NILP(mf))
            {
                char *real = lstring_value(lisp::eval(lisp::caar(mf)));
                char *fake = lstring_value(lisp::eval(lisp::cadar(mf)));
                if (!isa_var_name(real))
                {
                    lisp::print((LObject *)field);
                    lbreak("fields : no such var name \"%s\"\n",name);
                    exit(0);
                }
                total_fields++;

                fields=(named_field **)realloc(fields,sizeof(named_field *)*total_fields);
                fields[total_fields-1]=new named_field(real,fake);
                mf=lisp::cdr(mf);
            }
        } else if (f==l_logo)
        {
            char *fn = lstring_value(lisp::eval(lisp::cadar(field)));
            char *o = lstring_value(lisp::eval(lisp::caddar(field)));
            logo=cache.reg(fn,o,SPEC_IMAGE,1);
        } else if (f==l_vars)
        {
            void *l=lisp::cdar(field);
            PtrRef r8(l);
            while (l)
            {
                add_var(lisp::car(l),name);
                l=lisp::cdr(l);
            }
        }
        else
        {
            lisp::print(lisp::car(field));
            lbreak("Unknown field for character definition");
            exit(0);
        }
    }

    if (!seq[stopped])
        lbreak("object (%s) has no stopped state, please define one!\n",
             lstring_value(name->GetName()));

/*  char *fn=lstring_value(lisp::car(desc));
  if (!fn)
  {
    printf("No filename given for def-character (%s)\n",name);
    exit(0);
  }
  desc=lisp::cdr(desc);  //  skip filename


  Cell *mrph = desc->Assoc(l_morph);     // check for morph info
  morph_power=0;
  if (!NILP(mrph))
  {
    mrph=lisp::cdr(mrph);
    morph_mask=cache.reg_object(fn,lisp::car(mrph),SPEC_IMAGE,1);
    morph_power=lnumber_value(lisp::cadr(mrph));
  } else morph_mask=-1;

  Cell *sa = desc->Assoc(l_state_art);
  if (NILP(sa))
  {
    printf("missing state state art in def-character (%s)\n",name);
    exit(0);
  }

  sa=lisp::cdr(sa);   // list of state sequences
  while (!NILP(sa))
  {
    int num=lnumber_value(lisp::caar(sa));
    if (seq[num])
      printf("Warning : state '%s' defined multiply for object %s\n"
         "          using first definition\n",state_names[num],name);
    else
      seq[lnumber_value(lisp::caar(sa))]=new sequence(fn,lisp::cadar(sa),lisp::caddar(sa));
    sa=lisp::cdr(sa);
  }

  Cell *range = desc->Assoc(l_range);
  if (!NILP(range))
  {
    rangex=lnumber_value(lisp::cadr(range));
    rangey=lnumber_value(lisp::caddr(range));
  } else
  {
    rangex=100;
    rangey=50;
  }




  Cell *mf = desc->Assoc(l_fields);
  if (!NILP(mf))
  {
    mf=lisp::cdr(mf);
    total_fields=0;
    while (!NILP(mf))
    {
      char *name=lstring_value(lisp::caar(mf));
      int t = g_default_simple.total_vars(),find=-1;
      for (int i=0; find<0 && i<t; i++)
        if (!strcmp(g_default_simple.var_name(i),name))
      find=i;
      if (find<0)
      {
    lprint(desc->Assoc(l_fields));
    printf("fields : no such var name \"%s\"\n",name);
    printf("current possiblities are : \n");
    for (int i=0; i<t; i++) printf("\"%s\" ",g_default_simple.var_name(i));
    printf("\n");
    exit(0);
      }
      char *new_name=lstring_value(lisp::cadar(mf));
      total_fields++;

      fields=(named_field **)realloc(fields,sizeof(named_field *)*total_fields);
      fields[total_fields-1]=new named_field(find,new_name);
      mf=lisp::cdr(mf);
    }
  } else total_fields=0;


  Cell *lg = desc->Assoc(l_logo);
  if (NILP(lg))
  {
    if (get_cflag(CFLAG_IS_WEAPON))
    {
      lprint(desc);
      lbreak("object must have a logo defined if it is a weapon\n"
         "example '(logo . (""art/misc.spe"" . ""big waepon""))\n");
    }
    logo=-1;
  }
  else
    logo=cache.reg_object(fn,lisp::cdr(lg),SPEC_IMAGE,1);
    */
}


sequence *CharacterType::get_sequence(character_state s)
{
    if (!seq[s])
        return seq[stopped];

    return seq[s];
}

CharacterType::~CharacterType()
{
    for (int i = 0; i < ts; i++)
        delete seq[i];

    if (ts)
        free(seq);

    if (total_fields)
    {
        for (int i = 0; i < total_fields; i++)
            delete fields[i];
        free(fields);
    }

    if (ts)
        free(seq_syms);

    if (tiv)
    {
        free(vars);
        free(var_index);
    }
}

