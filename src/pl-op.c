/*  $Id$

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    See ../LICENCE to find out about your rights.
    jan@swi.psy.uva.nl

    Purpose: operator functions and declarations
*/

#include "pl-incl.h"

forwards int	atomToOperatorType(atom_t);
forwards atom_t	operatorTypeToAtom(int);

static Operator operatorTable[OPERATORHASHSIZE];

/*  Find an operator in the table. Type is one of OP_PREFIX, OP_INFIX or
    op_POSTFIX.

 ** Wed Apr 20 10:34:55 1988  jan@swivax.UUCP (Jan Wielemaker)  */

Operator
isCurrentOperator(atom_t name, int type)
{ register int v = pointerHashValue(name, OPERATORHASHSIZE);
  register Operator op;

  for(op=operatorTable[v]; op && !isTableRef(op); op=op->next)
  { if (op->name != name)
      continue;
    if (op->priority <= 0 )
      continue;
    switch(op->type)
    { case OP_FX:
      case OP_FY:	if (type == OP_PREFIX)
			  return op;
			continue;
      case OP_XF:
      case OP_YF:	if (type == OP_POSTFIX)
			  return op;
			continue;
      case OP_XFX:
      case OP_XFY:
      case OP_YFX:
      case OP_YFY:	if (type == OP_INFIX)
			  return op;
			continue;
    }
  }

  return (Operator) NULL;
}


static int
atomToOperatorType(atom_t atom)
{ if (atom == ATOM_fx)			return OP_FX;
  else if (atom == ATOM_fy)		return OP_FY;
  else if (atom == ATOM_xfx)		return OP_XFX;
  else if (atom == ATOM_xfy)		return OP_XFY;
  else if (atom == ATOM_yfx)		return OP_YFX;
  else if (atom == ATOM_yfy)		return OP_YFY;
  else if (atom == ATOM_yf)		return OP_YF;
  else if (atom == ATOM_xf)		return OP_XF;

  return -1;
}

static atom_t
operatorTypeToAtom(int type)
{ switch(type)
  { case OP_FX:				return ATOM_fx;
    case OP_FY:				return ATOM_fy;
    case OP_XFX:			return ATOM_xfx;
    case OP_XFY:			return ATOM_xfy;
    case OP_YFX:			return ATOM_yfx;
    case OP_YFY:			return ATOM_yfy;
    case OP_YF:				return ATOM_yf;
    case OP_XF:				return ATOM_xf;
  }
  return NULL_ATOM;
}

word
pl_current_op(term_t prec, term_t type, term_t name, word h)
{ int Prec = 0;					/* not specified */
  int Type = -1;				/* not specified */
  atom_t Name = NULL_ATOM;			/* not specified */
  Operator op;
  atom_t a;

  switch( ForeignControl(h) )
  { case FRG_FIRST_CALL:
      op = operatorTable[0];
      break;
    case FRG_REDO:
      op = ForeignContextPtr(h);
      break;
    case FRG_CUTTED:
    default:
      succeed;
  }

  if ( !PL_get_integer(prec, &Prec) &&
       !PL_is_variable(prec) )
    fail;

  if ( PL_get_atom(type, &a) )
  { if ( (Type = atomToOperatorType(a)) < 0 )
      fail;
  } else if ( !PL_is_variable(type))
    fail;

  if ( !PL_get_atom(name, &Name) &&
       !PL_is_variable(name) )
    fail;

  for( ; op; op = op->next )
  { while(isTableRef(op))
    { op = unTableRef(Operator, op);
      if ( !op )
	fail;
    }
    if ( Name && Name != op->name )
      continue;
    if ( Type >= 0 && Type != op->type  )
      continue;
    if ( Prec > 0 && Prec != op->priority )
      continue;
    if ( op->priority <= 0 )
      continue;

    if ( !PL_unify_atom(name, op->name) ||
	 !PL_unify_atom(type, operatorTypeToAtom(op->type)) ||
	 !PL_unify_integer(prec, op->priority) )
      fail;

    if ( Name && Type >=0 )
      succeed;

    return_next_table(Operator, op, ;);
  }

  fail;
}

/*  The following three functions check whether an atom is declared as
    an operator. 'type' and 'priority' are integer pointers. Their
    value is filled with the corresponding definition of the operator.

 ** Sun Apr 17 13:25:17 1988  jan@swivax.UUCP (Jan Wielemaker)  */

bool
isPrefixOperator(atom_t atom, int *type, int *priority)
{ register Operator op;

  if ((op = isCurrentOperator(atom, OP_PREFIX)) != (Operator) NULL)
  { if (op->priority != 0)
    { *priority = op->priority;
      *type = op->type;

      succeed;
    }
  }

  fail;
}

bool
isPostfixOperator(atom_t atom, int *type, int *priority)
{ Operator op;

  if ((op = isCurrentOperator(atom, OP_POSTFIX)) != (Operator) NULL)
  { if (op->priority != 0)
    { *priority = op->priority;
      *type = op->type;

      succeed;
    }
  }

  fail;
}

bool
isInfixOperator(atom_t atom, int *type, int *priority)
{ Operator op;

  if ((op = isCurrentOperator(atom, OP_INFIX)) != (Operator) NULL)
  { if (op->priority != 0)
    { *priority = op->priority;
      *type = op->type;

      succeed;
    }
  }

  fail;
}

/*  Declare a new operator. 'f' is a functor definition pointer, 'type'
    if one of OP_FX, ... and 'priority' is the priority (0-1200].

 ** Sun Apr 17 13:24:04 1988  jan@swivax.UUCP (Jan Wielemaker)  */

static bool
operator(atom_t name, int type, int priority)
{ Operator op = (Operator) NULL;

  switch(type)
  { case OP_FX:
    case OP_FY:		op = isCurrentOperator(name, OP_PREFIX);
			break;
    case OP_XF:
    case OP_YF:		op = isCurrentOperator(name, OP_POSTFIX);
			break;
    default:		op = isCurrentOperator(name, OP_INFIX);
			break;
  }

  if (op == (Operator) NULL)
  { int v;

    v = pointerHashValue(name, OPERATORHASHSIZE);
    op = (Operator) allocHeap(sizeof(struct operator));
    op->next = operatorTable[v];
    operatorTable[v] = op;
    op->name = name;
  }
  op->priority = priority;
  op->type = type;

  succeed;
}

word
pl_op1(term_t priority, term_t type, term_t name)
{ atom_t nm;
  atom_t tp;
  int t;
  int pri;

  if ( !PL_get_atom(name, &nm) ||
       !PL_get_atom(type, &tp) ||
       !PL_get_integer(priority, &pri) ||
       pri < 0 || pri > 1200 ||
       (t = atomToOperatorType(tp)) < 0 )
    fail;

  return operator(nm, t, pri);
}

/*  Define standard system operators.

 ** Sun Apr 17 13:25:40 1988  jan@swivax.UUCP (Jan Wielemaker)  */

bool
newOp(char *name, int type, int pri)
{ return operator(lookupAtom(name), type, pri);
}

#define OP(a, t, p) { (Operator)NULL, a, t, p }

static struct operator operators[] = {
  OP(ATOM_star,		OP_YFX,		400),		/* * */
  OP(ATOM_plus,		OP_FX,		500),		/* + */
  OP(ATOM_plus,		OP_YFX,		500),
  OP(ATOM_comma,	OP_XFY,	       1000),		/* , */
  OP(ATOM_minus,	OP_FX,		500),		/* - */
  OP(ATOM_minus,	OP_YFX,		500),
  OP(ATOM_grammar,	OP_XFX,	       1200),		/* --> */
  OP(ATOM_ifthen,	OP_XFY,	       1050),		/* -> */
  OP(ATOM_divide,	OP_YFX,		400),		/* / */
  OP(ATOM_div,		OP_YFX,		400),		/* // */
  OP(ATOM_and,		OP_YFX,		500),		/* /\ */
  OP(ATOM_module,	OP_XFY,		600),		/* : */
  OP(ATOM_prove,	OP_FX,	       1200),		/* :- */
  OP(ATOM_prove,	OP_XFX,	       1200),
  OP(ATOM_semicolon,	OP_XFY,	       1100),		/* ; */
  OP(ATOM_bar,		OP_XFY,	       1100),		/* | */
  OP(ATOM_smaller,	OP_XFX,		700),		/* < */
  OP(ATOM_lshift,	OP_YFX,		400),		/* << */
  OP(ATOM_equals,	OP_XFX,		700),		/* = */
  OP(ATOM_univ,		OP_XFX,		700),		/* =.. */
  OP(ATOM_ar_equals,	OP_XFX,		700),		/* =:= */
  OP(ATOM_smaller_equal,OP_XFX,		700),		/* =< */
  OP(ATOM_larger_equal,	OP_XFX,		700),		/* >= */
  OP(ATOM_strick_equal,	OP_XFX,		700),		/* == */
  OP(ATOM_ar_not_equal,	OP_XFX,		700),		/* =\= */
  OP(ATOM_larger,	OP_XFX,		700),		/* > */
  OP(ATOM_rshift,	OP_YFX,		400),		/* >> */
  OP(ATOM_obtain,	OP_FX,		500),		/* ? */
  OP(ATOM_query,	OP_FX,	       1200),		/* ?- */
  OP(ATOM_at_smaller,	OP_XFX,		700),		/* @< */
  OP(ATOM_at_smaller_eq,OP_XFX,		700),		/* @=< */
  OP(ATOM_at_larger,	OP_XFX,		700),		/* @> */
  OP(ATOM_at_larger_eq,	OP_XFX,		700),		/* @>= */
  OP(ATOM_backslash,	OP_FX,		500),		/* \ */
  OP(ATOM_not_provable,	OP_FY,		900),		/* \+ */
  OP(ATOM_or,		OP_YFX,		500),		/* \/ */
  OP(ATOM_not_equals,	OP_XFX,		700),		/* \= */
  OP(ATOM_not_strickt_equals,OP_XFX,	700),		/* \== */
  OP(ATOM_at_equals,	OP_XFX,		700),		/* =@= */
  OP(ATOM_at_not_equals,OP_XFX,		700),		/* \=@= */
  OP(ATOM_hat,		OP_XFY,		200),		/* ^ */
  OP(ATOM_doublestar,	OP_XFX,		200), 		/* ** */
  OP(ATOM_discontiguous,OP_FX,	       1150),		/* discontiguous */
  OP(ATOM_dynamic,	OP_FX,	       1150),		/* dynamic */
  OP(ATOM_volatile,	OP_FX,	       1150), 		/* volatile */
  OP(ATOM_initialization,OP_FX,	       1150), 		/* initialization */
  OP(ATOM_is,		OP_XFX,		700),		/* is */
  OP(ATOM_mod,		OP_YFX,		400),		/* mod */
  OP(ATOM_rem,		OP_YFX,		400),		/* rem */
  OP(ATOM_module_transparent,OP_FX,    1150),		/* module_transparent */
  OP(ATOM_multifile,	OP_FX,	       1150),		/* multifile */
  OP(ATOM_not,		OP_FY,		900),		/* not */
  OP(ATOM_xor,		OP_YFX,		400),		/* xor */
/*OP(ATOM_tilde,	OP_FX,		900),*/		/* ~ */

  OP(NULL_ATOM,		0,		0)
};


void
initOperators(void)
{ { register Operator *op;
    register int n;

    for(n=0, op=operatorTable; n < (OPERATORHASHSIZE-1); n++, op++)
      *op = makeTableRef(op+1);

    *op = NULL;
  }

  { register Operator op;

    for( op = operators; op->name; op++ )
      operator(op->name, op->type, op->priority);
  }
}


word
pl_reset_operators()
{ int n;

  for(n=0; n<OPERATORHASHSIZE; n++)
  { Operator op = operatorTable[n];
    Operator next;

    for( ; op && !isTableRef(op); op = next )
    { next = op->next;

      freeHeap(op, sizeof(*op));
    }
  }

  initOperators();

  succeed;
}
