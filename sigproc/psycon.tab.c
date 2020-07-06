
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 15 "psycon.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "psycon.yacc.h"
#define strdup _strdup
#define YYPRINT(file, type, value) print_token_value (file, type, value)
/*#define DEBUG*/

char *ErrorMsg = NULL;
int yylex (void);
void yyerror (AstNode **pproot, char **errmsg, char const *s);


/* Line 189 of yacc.c  */
#line 88 "psycon.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 1
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_EOF = 0,
     T_UNKNOWN = 258,
     T_NEWLINE = 259,
     T_IF = 260,
     T_ELSE = 261,
     T_ELSEIF = 262,
     T_END = 263,
     T_WHILE = 264,
     T_FOR = 265,
     T_BREAK = 266,
     T_CONTINUE = 267,
     T_SWITCH = 268,
     T_CASE = 269,
     T_OTHERWISE = 270,
     T_FUNCTION = 271,
     T_STATIC = 272,
     T_RETURN = 273,
     T_SIGMA = 274,
     T_TRY = 275,
     T_CATCH = 276,
     T_OP_SHIFT = 277,
     T_OP_CONCAT = 278,
     T_LOGIC_EQ = 279,
     T_LOGIC_NE = 280,
     T_LOGIC_LE = 281,
     T_LOGIC_GE = 282,
     T_LOGIC_AND = 283,
     T_LOGIC_OR = 284,
     T_REPLICA = 285,
     T_MATRIXMULT = 286,
     T_NUMBER = 287,
     T_STRING = 288,
     T_ID = 289,
     T_ENDPOINT = 290,
     T_FULLRANGE = 291,
     T_NEGATIVE = 293,
     T_POSITIVE = 294,
     T_LOGIC_NOT = 295,
     T_TRANSPOSE = 296
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 35 "psycon.y"

	double dval;
	char *str;
	AstNode *pnode;



/* Line 214 of yacc.c  */
#line 173 "psycon.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */

/* Line 264 of yacc.c  */
#line 118 "psycon.y"

AstNode *newAstNode(int type, YYLTYPE loc);
AstNode *makeFunctionCall(char *name, AstNode *first, AstNode *second, YYLTYPE loc);
AstNode *makeBinaryOpNode(int op, AstNode *first, AstNode *second, YYLTYPE loc);
void print_token_value(FILE *file, int type, YYSTYPE value);
char *getT_ID_str(AstNode *p);


/* Line 264 of yacc.c  */
#line 207 "psycon.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  77
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2521

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  81
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  32
/* YYNRULES -- Number of rules.  */
#define YYNRULES  155
/* YYNRULES -- Number of states.  */
#define YYNSTATES  296

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   310

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    60,     2,    48,    79,    46,     2,    38,
      58,    59,    49,    44,    56,    43,    74,    50,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    41,    57,
      39,    37,    40,     2,    47,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    77,     2,    78,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    75,     2,    76,    42,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    45,    52,    53,    54,    55,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      80
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    11,    13,    16,    18,
      21,    24,    27,    29,    31,    33,    35,    37,    39,    40,
      42,    44,    47,    49,    52,    53,    58,    62,    64,    66,
      68,    70,    72,    78,    83,    90,    97,   102,   109,   117,
     119,   121,   123,   127,   131,   135,   139,   143,   147,   151,
     154,   158,   162,   163,   165,   169,   171,   173,   175,   177,
     181,   183,   184,   186,   190,   193,   195,   198,   202,   206,
     212,   214,   216,   218,   220,   222,   224,   226,   228,   230,
     232,   234,   236,   238,   240,   243,   246,   249,   252,   255,
     257,   261,   265,   270,   277,   281,   284,   286,   291,   296,
     304,   311,   316,   320,   322,   327,   332,   340,   347,   350,
     356,   363,   370,   377,   384,   388,   392,   395,   398,   402,
     406,   410,   412,   414,   416,   418,   420,   427,   430,   433,
     442,   446,   450,   454,   458,   462,   466,   470,   474,   478,
     482,   486,   490,   494,   498,   500,   503,   508,   515,   523,
     533,   534,   536,   542,   550,   552
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      82,     0,    -1,    -1,    83,    -1,    86,    -1,    83,    86,
      -1,    85,    -1,    84,    85,    -1,     4,    -1,     1,     4,
      -1,    93,    87,    -1,    93,    88,    -1,    85,    -1,   109,
      -1,    56,    -1,     4,    -1,     0,    -1,    57,    -1,    -1,
       0,    -1,    94,    -1,    94,    87,    -1,   107,    -1,   107,
      87,    -1,    -1,     7,    90,    84,    91,    -1,    91,     6,
      84,    -1,   111,    -1,    94,    -1,    92,    -1,   106,    -1,
     112,    -1,     5,    90,    84,    91,     8,    -1,    13,   107,
     110,     8,    -1,    13,   107,   110,    15,    84,     8,    -1,
      20,    84,    21,    34,    84,     8,    -1,     9,    90,    84,
       8,    -1,    10,    34,    37,   101,    84,     8,    -1,    10,
      34,    37,   101,    56,    84,     8,    -1,    18,    -1,    11,
      -1,    12,    -1,   107,    39,   107,    -1,   107,    40,   107,
      -1,   107,    24,   107,    -1,   107,    25,   107,    -1,   107,
      27,   107,    -1,   107,    26,   107,    -1,    58,    94,    59,
      -1,    60,    92,    -1,    92,    28,    92,    -1,    92,    29,
      92,    -1,    -1,    34,    -1,    95,    56,    34,    -1,    41,
      -1,   101,    -1,   112,    -1,    96,    -1,    97,    56,    96,
      -1,    94,    -1,    -1,    99,    -1,    98,    57,    99,    -1,
      98,    57,    -1,   101,    -1,    99,   101,    -1,    99,    56,
     101,    -1,   107,    41,   107,    -1,   107,    41,   107,    41,
     107,    -1,   107,    -1,   100,    -1,    61,    -1,    62,    -1,
      63,    -1,    64,    -1,    65,    -1,    66,    -1,    67,    -1,
      68,    -1,    69,    -1,    70,    -1,    71,    -1,    72,    -1,
      37,   101,    -1,    37,    94,    -1,    73,    94,    -1,    73,
     107,    -1,   102,   101,    -1,    34,    -1,   105,    74,    34,
      -1,    94,    74,    34,    -1,   104,    75,   107,    76,    -1,
     104,    58,   107,    42,   107,    59,    -1,    77,    99,    78,
      -1,    79,   104,    -1,   104,    -1,    34,    58,    97,    59,
      -1,    34,    75,   107,    76,    -1,    34,    75,   107,    76,
      58,    97,    59,    -1,    34,    58,   107,    42,   107,    59,
      -1,   104,    58,    97,    59,    -1,   104,    58,    59,    -1,
      30,    -1,    30,    58,    97,    59,    -1,    30,    75,   107,
      76,    -1,    30,    75,   107,    76,    58,    97,    59,    -1,
      30,    58,   107,    42,   107,    59,    -1,   105,    38,    -1,
      77,    99,    78,    77,    78,    -1,    77,    99,    78,    77,
      99,    78,    -1,    77,    99,    78,    77,    98,    78,    -1,
      77,    98,    78,    77,    99,    78,    -1,    77,    98,    78,
      77,    98,    78,    -1,    77,    98,    78,    -1,    58,   101,
      59,    -1,   105,   103,    -1,   104,   103,    -1,   104,    37,
     106,    -1,   105,    37,   106,    -1,   104,    37,   112,    -1,
      32,    -1,    33,    -1,    35,    -1,   112,    -1,   105,    -1,
     105,    58,   107,    42,   107,    59,    -1,    43,   107,    -1,
      44,   107,    -1,    19,    58,   105,    37,   101,    56,   107,
      59,    -1,   107,    44,   107,    -1,   107,    43,   107,    -1,
     107,    49,   107,    -1,   107,    50,   107,    -1,   107,    31,
     107,    -1,   107,    51,   107,    -1,   107,    46,   107,    -1,
     107,    42,   107,    -1,   107,    48,   107,    -1,   107,    80,
     107,    -1,   107,    45,   107,    -1,   107,    47,   107,    -1,
     107,    22,   107,    -1,   107,    23,   107,    -1,    16,    -1,
      17,    16,    -1,   108,    34,    84,    89,    -1,   108,   104,
      37,    34,    84,    89,    -1,   108,    34,    58,    95,    59,
      84,    89,    -1,   108,   104,    37,    34,    58,    95,    59,
      84,    89,    -1,    -1,     4,    -1,   110,    14,   107,     4,
      84,    -1,   110,    14,    75,    97,    76,     4,    84,    -1,
     101,    -1,    75,    97,    76,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   138,   138,   139,   143,   145,   166,   173,   195,   197,
     202,   203,   210,   211,   214,   214,   214,   217,   220,   220,
     223,   223,   223,   223,   227,   231,   245,   266,   267,   270,
     271,   272,   273,   299,   307,   318,   327,   339,   353,   367,
     371,   373,   379,   381,   383,   385,   387,   389,   391,   397,
     402,   404,   409,   412,   418,   426,   428,   429,   432,   437,
     445,   449,   459,   466,   472,   475,   487,   493,   501,   505,
     512,   512,   514,   518,   520,   522,   524,   526,   531,   533,
     535,   541,   547,   553,   561,   565,   570,   579,   588,   608,
     613,   632,   644,   650,   657,   661,   669,   670,   676,   683,
     705,   713,   721,   733,   737,   742,   749,   756,   763,   768,
     773,   779,   785,   793,   801,   805,   814,   819,   824,   839,
     854,   864,   869,   874,   878,   879,   883,   899,   904,   910,
     918,   920,   922,   924,   926,   928,   930,   932,   934,   936,
     938,   940,   942,   944,   948,   953,   960,   967,   985,   992,
    1013,  1014,  1016,  1031,  1048,  1051
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of text\"", "error", "$undefined", "T_UNKNOWN",
  "\"end of line\"", "\"if\"", "\"else\"", "\"elseif\"", "\"end\"",
  "\"while\"", "\"for\"", "\"break\"", "\"continue\"", "\"switch\"",
  "\"case\"", "\"otherwise\"", "\"function\"", "\"static\"", "\"return\"",
  "\"sigma\"", "\"try\"", "\"catch\"", "\">>\"", "\"++\"", "\"==\"",
  "\"!=\"", "\"<=\"", "\">=\"", "\"&&\"", "\"||\"", "\"..\"", "\"**\"",
  "\"number\"", "\"string\"", "\"identifier\"", "T_ENDPOINT",
  "T_FULLRANGE", "'='", "'\\''", "'<'", "'>'", "':'", "'~'", "'-'", "'+'",
  "\"->\"", "'%'", "'@'", "'#'", "'*'", "'/'", "'^'", "T_NEGATIVE",
  "T_POSITIVE", "T_LOGIC_NOT", "T_TRANSPOSE", "','", "';'", "'('", "')'",
  "'!'", "\"+=\"", "\"-=\"", "\"*=\"", "\"/=\"", "\"@=\"", "\"@@=\"",
  "\">>=\"", "\"%=\"", "\"->=\"", "\"~=\"", "\"<>=\"", "\"#=\"", "\"++=\"",
  "'.'", "'{'", "'}'", "'['", "']'", "'$'", "\"<>\"", "$accept", "input",
  "block_func", "block", "line", "line_func", "eol", "eol2", "func_end",
  "conditional", "elseif_list", "expcondition", "stmt", "condition",
  "id_list", "arg", "arg_list", "matrix", "vector", "range", "exp_range",
  "compop", "assign2this", "varblock", "tid", "assign", "exp",
  "func_static_or_not", "funcdef", "case_list", "csig", "initcell", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,    61,    39,    60,
      62,    58,   126,    45,    43,   292,    37,    64,    35,    42,
      47,    94,   293,   294,   295,   296,    44,    59,    40,    41,
      33,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,    46,   123,   125,    91,    93,    36,
     310
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    81,    82,    82,    83,    83,    84,    84,    85,    85,
      85,    85,    86,    86,    87,    87,    87,    88,    89,    89,
      90,    90,    90,    90,    91,    91,    91,    92,    92,    93,
      93,    93,    93,    93,    93,    93,    93,    93,    93,    93,
      93,    93,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    95,    95,    95,    96,    96,    96,    97,    97,
      97,    98,    98,    98,    98,    99,    99,    99,   100,   100,
     101,   101,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   103,   103,   103,   103,   103,   104,
     104,   104,   104,   104,   104,   104,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   106,   106,   106,   106,
     106,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   108,   108,   109,   109,   109,   109,
     110,   110,   110,   110,   111,   112
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     1,     2,     1,     2,
       2,     2,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     2,     1,     2,     0,     4,     3,     1,     1,     1,
       1,     1,     5,     4,     6,     6,     4,     6,     7,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     2,
       3,     3,     0,     1,     3,     1,     1,     1,     1,     3,
       1,     0,     1,     3,     2,     1,     2,     3,     3,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     2,     2,     1,
       3,     3,     4,     6,     3,     2,     1,     4,     4,     7,
       6,     4,     3,     1,     4,     4,     7,     6,     2,     5,
       6,     6,     6,     6,     3,     3,     2,     2,     3,     3,
       3,     1,     1,     1,     1,     1,     6,     2,     2,     8,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     2,     4,     6,     7,     9,
       0,     1,     5,     7,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     8,     0,     0,     0,    40,    41,     0,   144,
       0,    39,     0,     0,   103,   121,   122,    89,   123,     0,
       0,     0,     0,     0,    61,     0,     0,     0,    12,     4,
      29,     0,    28,    71,   154,    96,   125,    30,    70,     0,
      13,    27,   124,     9,     0,     0,    20,    96,   125,    22,
     124,     0,     0,   150,   145,     0,     0,     6,     0,     0,
       0,     0,   127,   128,    28,   154,    49,    55,    60,    58,
       0,    56,   124,     0,    62,    65,    95,     1,     5,     0,
       0,    16,    15,    14,    17,    10,    11,     0,     0,     0,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,     0,     0,     0,   117,     0,   108,     0,     0,
     116,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    96,     0,    21,    23,     0,     0,   151,
       0,   125,     0,     7,     0,    70,    70,     0,    70,    70,
      48,   115,     0,   155,    64,   114,     0,    94,    66,    50,
      51,    91,    85,    84,   118,   124,   102,     0,    70,    86,
      87,    70,    88,   119,    70,    90,   142,   143,    44,    45,
      47,    46,   134,    42,    43,    68,   137,   131,   130,   140,
     136,   141,   138,   132,   133,   135,   139,    52,     0,     0,
       0,     0,     0,    36,     0,    33,     0,     0,     0,     0,
     104,     0,   105,    97,     0,    98,    59,    63,    61,    67,
      61,   101,     0,    92,     0,     0,    89,    28,     0,   154,
      19,   146,     0,     0,     0,    32,     0,     0,     0,    70,
       0,   154,     0,   137,     0,   137,     0,     0,    62,   109,
       0,    62,   137,   137,    69,     0,     0,    52,     0,     0,
       0,     0,    37,     0,     0,    34,     0,    35,   107,     0,
     100,     0,   113,   112,   111,   110,    93,   126,    54,     0,
       0,   147,    25,    38,   155,     0,    70,   106,    99,   148,
       0,     0,   129,     0,     0,   149
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    26,    27,    56,    57,    29,    85,    86,   231,    44,
     202,    45,    31,    32,   228,    69,    70,    73,    74,    33,
      34,   104,   105,    47,    48,    37,    38,    39,    40,   140,
      41,    50
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -183
static const yytype_int16 yypact[] =
{
     947,    43,  -183,  1813,  1813,   -30,  -183,  -183,  1813,  -183,
      64,  -183,   -16,  1633,   -44,  -183,  -183,    21,  -183,  1813,
    1813,  1813,  1813,   106,  1813,  1813,    82,   984,  -183,  -183,
      26,    16,    13,  -183,  -183,   495,   809,  -183,  2079,  1831,
    -183,  -183,    61,  -183,  1633,    26,     1,   103,    36,   467,
    -183,  1633,    84,  1096,  -183,  1813,  1316,  -183,   106,  1813,
     106,  1813,   -21,   -21,    56,    89,  -183,  -183,    -3,  -183,
      57,    63,    67,   -35,   385,    63,   103,  -183,  -183,  1813,
    1813,  -183,  -183,  -183,  -183,  -183,  -183,    93,  1813,   668,
    -183,  -183,  -183,  -183,  -183,  -183,  -183,  -183,  -183,  -183,
    -183,  -183,  1813,  1813,  1813,  -183,  1813,  -183,  1813,   129,
    -183,  1813,  1813,  1813,  1813,  1813,  1813,  1813,  1813,  1813,
    1813,  1813,  1813,  1813,  1813,  1813,  1813,  1813,  1813,  1813,
    1813,  1813,   895,    -9,   760,  -183,  -183,  1353,  1813,  -183,
      38,    71,   134,  -183,   121,  2121,  1911,   130,  2163,  1953,
    -183,  -183,   106,  -183,  1813,    78,  1813,   102,    63,  -183,
     142,  -183,    -3,    63,  -183,   116,  -183,   137,  2205,    -3,
    1376,  1995,    63,  -183,  2247,  -183,   294,   -10,  2079,  2079,
    2079,  2079,   -18,  2079,  2079,  2289,  2441,   -10,   -10,   294,
     294,   294,   294,   -18,   -18,   -18,  1376,  1738,   106,  1036,
     150,  1813,     9,  -183,  1073,  -183,  1851,  1633,  1813,  1633,
    -183,  1813,   143,  -183,  1813,   148,  -183,  1756,  1813,    63,
    1776,  -183,  1813,  -183,  1813,  1813,   101,    95,   157,   165,
    -183,  -183,  1670,  1633,  1633,  -183,  1633,  1430,   106,  1210,
    1467,   132,  1544,    55,   106,  2327,   106,   -33,   780,  -183,
       7,  1688,  2365,  2403,  1490,   161,  1633,  1888,  1036,   760,
    1264,  1581,  -183,    66,  1633,  -183,  1813,  -183,  -183,   167,
    -183,   169,  -183,  -183,  -183,  -183,  -183,  -183,  -183,  1036,
     171,  -183,   202,  -183,   211,  1150,  2037,  -183,  -183,  -183,
    1633,  1633,  -183,  1036,  1187,  -183
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -183,  -183,  -183,   -42,   243,   190,   201,  -183,  -182,     2,
     -40,   131,  -183,   584,   -28,    79,   -26,  -170,  -151,  -183,
     646,  -183,   199,    68,   342,    40,   535,  -183,  -183,  -183,
    -183,     0
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -155
static const yytype_int16 yytable[] =
{
      42,    81,   134,   217,    52,    82,    51,   -70,   -70,   137,
     -70,   -70,   111,    42,    58,   234,    81,   235,   -70,   -70,
      82,   117,   154,    72,   154,   -28,   -28,    42,   200,   -28,
     -28,    59,   144,   130,   147,   124,   125,   126,   127,   128,
     129,   130,    55,   155,    42,   272,   205,    43,   247,    89,
     250,    42,   206,   207,    79,    80,    42,    83,    72,   131,
      72,   -31,   131,   167,   154,   -31,   103,   248,    35,   251,
     131,    87,    83,    84,   107,    87,   281,   111,   112,    60,
      54,    35,    77,   -70,   -70,   274,   117,    87,   165,    72,
     199,  -154,  -154,    76,   108,    35,    61,   289,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   133,   208,   107,
     109,   295,    35,   152,   268,   150,  -120,   -31,   -31,    35,
    -120,   138,   152,   -57,    35,    12,   -57,   161,   164,   108,
      87,    30,    42,   153,    42,   131,    14,    42,    15,    16,
      17,    18,   284,   -57,    30,   109,   173,    67,   151,    19,
      20,   -60,    72,    66,   150,   218,    35,   -53,    30,    60,
     -53,    89,   237,   175,    21,   240,    22,   242,   209,    87,
      79,   147,  -120,  -120,    35,    30,    61,   152,   103,   220,
     210,    23,    30,    24,   232,    25,   152,    30,   266,   213,
     258,   259,   260,   152,   261,   278,   221,    72,    72,    42,
      35,   244,    35,   233,    42,    35,   246,    42,   234,    42,
     159,   160,   263,   255,   279,   291,   256,    78,   269,   282,
     271,   -56,   285,   152,   151,   152,   287,   255,   288,   280,
     290,   216,    42,    42,    42,   110,    42,    42,    72,     0,
      42,     0,    42,    28,    72,     0,    72,   135,   293,   294,
     136,     0,     0,     0,     0,     0,    42,     0,    42,    42,
      42,    42,     0,    30,    42,    30,     0,    35,    30,     0,
      28,     0,    35,     0,     0,    35,     0,    35,     0,    42,
       0,     0,     0,     0,     0,    42,     0,     0,     0,     0,
      42,    42,     0,    42,    42,     0,     0,     0,     0,   143,
      35,    35,    35,     0,    35,    35,     0,     0,    35,     0,
      35,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   -70,   -70,    35,   117,    35,    35,    35,    35,
      30,     0,    35,     0,     0,    30,     0,     0,    30,     0,
      30,     0,    36,   128,   129,   130,     0,    35,     0,     0,
       0,     0,     0,    35,     0,    36,     0,     0,    35,    35,
       0,    35,    35,    30,    30,    30,     0,    30,    30,    36,
       0,    30,     0,    30,   131,     0,     0,   143,     0,     0,
     143,     0,     0,     0,     0,     0,    36,    30,     0,    30,
      30,    30,    30,    36,     0,    30,     0,   141,    36,     0,
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
      30,     0,     0,     0,     0,    14,    30,    15,    16,    17,
      18,    30,    30,     0,    30,    30,     0,     0,    19,    20,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   156,   143,    21,     0,    22,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      23,     0,    24,   157,    25,     0,     0,    81,     0,     0,
       0,    82,     0,     0,    36,     0,    36,     0,     0,    36,
     143,     0,     0,   143,     0,   143,     0,     0,     0,   111,
     112,   113,   114,   115,   116,   -70,   -70,     0,   117,     0,
       0,   143,   143,   143,   143,     0,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,     0,
       0,     0,   143,    83,     0,     0,     0,     0,   143,     0,
       0,     0,    88,     0,     0,     0,   143,   143,    49,    49,
       0,    36,     0,    53,     0,     0,    36,   131,     0,    36,
       0,    36,     0,    89,    62,    63,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,     0,
     103,     0,     0,     0,    36,    36,    36,     0,    36,    36,
       0,     0,    36,     0,    36,     0,     0,    46,    46,     0,
       0,     0,     0,   145,   146,   148,   149,     0,    36,     0,
      36,    36,    36,    36,     0,    64,    36,    68,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,   168,     0,     0,    36,     0,     0,
       0,     0,    36,    36,     0,    36,    36,   170,   171,     0,
       0,     0,    68,   174,    68,     0,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    65,     0,    71,
      75,     0,   162,    68,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   169,    12,     0,     0,
     162,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    71,     0,    71,     0,     0,    67,
       0,    19,    20,     0,     0,     0,     0,     0,     0,     0,
     158,     0,     0,     0,     0,     0,    21,   166,    22,     0,
       0,     0,   148,   149,   163,    71,    49,     0,     0,     0,
       0,   239,     0,    23,     0,    24,   243,    25,     0,   245,
     172,     0,   163,     0,     0,     0,     0,   252,     0,   253,
     254,     1,     0,     0,     2,     3,   -24,   201,   -24,     4,
       5,     6,     7,     8,     0,     0,     0,     0,    11,    12,
      13,   227,    68,     0,   204,    46,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,     0,     0,    71,    12,
      75,   286,   219,    19,    20,     0,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,     0,     0,    21,     0,
      22,     0,    68,    19,    20,     0,     0,     0,    68,     0,
      68,     0,     0,     0,     0,    23,   156,    24,    21,    25,
      22,    64,     0,   229,    71,     0,   106,   107,     0,     0,
       0,     0,     0,     0,   241,    23,     0,    24,   273,    25,
       0,     0,     0,   158,    75,     0,    75,   108,     0,     0,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   109,    71,     0,     0,     0,     0,     0,
      71,     0,    71,     0,   158,     0,     1,   158,     0,     2,
       3,     0,     0,    65,     4,     5,     6,     7,     8,     0,
       0,     0,     0,    11,    12,    13,     0,   -89,   -89,   -89,
     -89,   -89,   -89,   -89,   -89,    14,   -89,    15,    16,    17,
      18,     0,   -89,   -89,   -89,   -89,   -89,   -89,    19,    20,
     -89,   -89,   -89,   -89,   -89,   -89,   -89,    -2,     1,     0,
       0,     2,     3,   197,     0,    22,     4,     5,     6,     7,
       8,     0,     0,     9,    10,    11,    12,    13,     0,   -89,
     198,     0,    24,     0,    25,   -89,     0,    14,     0,    15,
      16,    17,    18,     0,    -3,     1,     0,     0,     2,     3,
      19,    20,     0,     4,     5,     6,     7,     8,     0,     0,
       9,    10,    11,    12,    13,    21,     0,    22,     0,     0,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
       0,     0,    23,     0,    24,     0,    25,    19,    20,     0,
       0,     0,     0,     0,     0,     0,   230,     1,     0,     0,
       2,     3,    21,     0,    22,     4,     5,     6,     7,     8,
       0,     0,   -18,   -18,    11,    12,    13,     0,     0,    23,
       0,    24,     0,    25,     0,     0,    14,     0,    15,    16,
      17,    18,     0,     0,     1,     0,     0,     2,     3,    19,
      20,     0,     4,     5,     6,     7,     8,     0,     0,     0,
       0,    11,    12,    13,    21,     0,    22,     0,     0,     0,
     139,  -154,  -154,    14,     0,    15,    16,    17,    18,     0,
       0,    23,     0,    24,     0,    25,    19,    20,   111,   112,
     113,   114,   115,   116,   -70,   -70,     0,   117,     0,   236,
       0,    21,     0,    22,     0,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,    23,     0,
      24,     1,    25,     0,     2,     3,     0,     0,  -152,     4,
       5,     6,     7,     8,  -152,  -152,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,   131,     0,     0,     0,
      14,     0,    15,    16,    17,    18,     0,     0,     1,     0,
       0,     2,     3,    19,    20,  -153,     4,     5,     6,     7,
       8,  -153,  -153,     0,     0,    11,    12,    13,    21,     0,
      22,     0,     0,     0,   264,     0,     0,    14,     0,    15,
      16,    17,    18,     0,     0,    23,     0,    24,     0,    25,
      19,    20,   111,   112,   113,   114,   115,   116,     0,     0,
       0,   117,     0,     0,     0,    21,     0,    22,     0,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,    23,     0,    24,     1,    25,     0,     2,     3,
     -26,     0,   -26,     4,     5,     6,     7,     8,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
     131,     0,     0,     0,    14,     0,    15,    16,    17,    18,
       0,     0,     0,     0,     0,     0,     0,    19,    20,     0,
       0,     0,     0,     0,     0,     0,     0,     1,     0,     0,
       2,     3,    21,     0,    22,     4,     5,     6,     7,     8,
       0,     0,     0,     0,    11,    12,    13,   142,     0,    23,
       0,    24,     0,    25,     0,     0,    14,     0,    15,    16,
      17,    18,     0,     0,     1,     0,     0,     2,     3,    19,
      20,   203,     4,     5,     6,     7,     8,     0,     0,     0,
       0,    11,    12,    13,    21,     0,    22,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,     0,
       0,    23,     0,    24,     0,    25,    19,    20,   111,   112,
     113,   114,   115,   116,   -70,   -70,     0,   117,     0,     0,
       0,    21,     0,    22,     0,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,    23,     0,
      24,     1,    25,     0,     2,     3,     0,     0,   262,     4,
       5,     6,     7,     8,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,   131,     0,     0,     0,
      14,     0,    15,    16,    17,    18,     0,     0,     1,     0,
       0,     2,     3,    19,    20,   265,     4,     5,     6,     7,
       8,     0,     0,     0,     0,    11,    12,    13,    21,     0,
      22,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,     0,     0,    23,     0,    24,     0,    25,
      19,    20,   111,   112,   113,   114,   115,   116,   -68,   -68,
       0,   117,     0,     0,     0,    21,     0,    22,     0,   118,
     119,   225,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,    23,     0,    24,     1,    25,     0,     2,     3,
       0,     0,   267,     4,     5,     6,     7,     8,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
     131,     0,     0,     0,    14,     0,    15,    16,    17,    18,
       0,     0,     1,     0,     0,     2,     3,    19,    20,   283,
       4,     5,     6,     7,     8,     0,     0,     0,     0,    11,
      12,    13,    21,     0,    22,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,     0,     0,    23,
       0,    24,     0,    25,    19,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     1,     0,     0,     2,     3,    21,
       0,    22,     4,     5,     6,     7,     8,     0,     0,     0,
       0,    11,    12,    13,     0,     0,    23,     0,    24,     0,
      25,     0,     0,    14,     0,    15,    16,    17,    18,     0,
       0,     1,     0,     0,     2,     3,    19,    20,     0,     4,
       5,     6,     7,     8,     0,     0,     0,     0,    11,    12,
      13,    21,     0,    22,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,     0,    12,    23,     0,
      24,     0,    25,    19,    20,     0,     0,     0,    14,     0,
      15,    16,    17,    18,     0,     0,     0,     0,   257,     0,
      22,    19,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   156,    23,    21,    24,    22,    25,
       0,     0,     0,     0,     0,     0,     0,    12,     0,     0,
       0,     0,     0,    23,     0,    24,   275,    25,    14,     0,
      15,    16,   226,    18,     0,    12,     0,     0,     0,    67,
       0,    19,    20,     0,     0,     0,    14,     0,    15,    16,
      17,    18,     0,     0,     0,    12,    21,     0,    22,    19,
      20,     0,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,   156,    23,    21,    24,    22,    25,     0,    19,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    23,    12,    24,    21,    25,    22,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,     0,
      12,    23,     0,    24,   249,    25,    19,    20,     0,     0,
       0,    14,     0,    15,    16,   132,    18,     0,     0,     0,
      12,    21,     0,    22,    19,    20,     0,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,     0,    23,    21,
      24,    22,    25,     0,    19,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    23,    12,    24,    21,
      25,    22,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,   226,    18,     0,     0,   238,     0,    24,     0,
      25,    19,    20,   111,   112,   113,   114,   115,   116,     0,
       0,     0,   117,     0,     0,     0,    21,     0,    22,     0,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,    23,     0,    24,     0,    25,     0,     0,
       0,     0,     0,     0,     0,   111,   112,   113,   114,   115,
     116,     0,     0,     0,   117,     0,     0,   212,     0,     0,
       0,   131,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   111,   112,   113,
     114,   115,   116,     0,     0,     0,   117,     0,     0,   215,
       0,     0,     0,   131,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   111,
     112,   113,   114,   115,   116,     0,     0,     0,   117,     0,
       0,   223,     0,     0,     0,   131,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,     0,
       0,     0,     0,     0,     0,     0,   292,     0,     0,     0,
       0,   111,   112,   113,   114,   115,   116,     0,     0,     0,
     117,     0,     0,     0,     0,     0,     0,   131,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   111,   112,   113,   114,   115,   116,     0,
       0,     0,   117,     0,     0,     0,     0,     0,     0,   131,
     118,   119,   120,   211,   122,   123,   124,   125,   126,   127,
     128,   129,   130,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   111,   112,   113,   114,   115,
     116,     0,     0,     0,   117,     0,     0,     0,     0,     0,
       0,   131,   118,   119,   120,   214,   122,   123,   124,   125,
     126,   127,   128,   129,   130,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   111,   112,   113,
     114,   115,   116,     0,     0,     0,   117,     0,     0,     0,
       0,     0,     0,   131,   118,   119,   120,   222,   122,   123,
     124,   125,   126,   127,   128,   129,   130,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   111,
     112,   113,   114,   115,   116,     0,     0,     0,   117,     0,
       0,     0,     0,     0,     0,   131,   118,   119,   120,   224,
     122,   123,   124,   125,   126,   127,   128,   129,   130,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   111,   112,   113,   114,   115,   116,     0,     0,     0,
     117,     0,     0,     0,     0,     0,     0,   131,   118,   119,
     225,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,     0,     0,     0,     0,     0,     0,     0,     0,   111,
     112,     0,     0,     0,     0,   -70,   -70,     0,   117,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   131,
     122,   123,   124,   125,   126,   127,   128,   129,   130,     0,
       0,     0,     0,     0,     0,     0,   270,   111,   112,     0,
       0,     0,     0,   -70,   -70,     0,   117,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   131,   122,   123,
     124,   125,   126,   127,   128,   129,   130,     0,     0,     0,
       0,     0,     0,     0,   276,   111,   112,     0,     0,     0,
       0,   -70,   -70,     0,   117,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   131,   122,   123,   124,   125,
     126,   127,   128,   129,   130,     0,     0,     0,     0,     0,
       0,     0,   277,   111,   112,     0,     0,     0,     0,   -70,
     -70,     0,   117,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   131,   122,   123,   124,   125,   126,   127,
     128,   129,   130,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   131
};

static const yytype_int16 yycheck[] =
{
       0,     0,    44,   154,    34,     4,     4,    28,    29,    51,
      28,    29,    22,    13,    58,     6,     0,     8,    28,    29,
       4,    31,    57,    23,    57,    28,    29,    27,    37,    28,
      29,    75,    58,    51,    60,    45,    46,    47,    48,    49,
      50,    51,    58,    78,    44,    78,     8,     4,   218,    58,
     220,    51,    14,    15,    28,    29,    56,    56,    58,    80,
      60,     0,    80,    89,    57,     4,    75,   218,     0,   220,
      80,    74,    56,    57,    38,    74,   258,    22,    23,    58,
      16,    13,     0,    28,    29,    78,    31,    74,    88,    89,
     132,    28,    29,    25,    58,    27,    75,   279,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    39,    37,    38,
      74,   293,    44,    56,    59,    59,     0,    56,    57,    51,
       4,    37,    56,    56,    56,    19,    59,    34,    88,    58,
      74,     0,   132,    76,   134,    80,    30,   137,    32,    33,
      34,    35,    76,    76,    13,    74,   106,    41,    59,    43,
      44,    56,   152,    22,    59,    77,    88,    56,    27,    58,
      59,    58,   204,    34,    58,   207,    60,   209,    34,    74,
      28,   197,    56,    57,   106,    44,    75,    56,    75,    77,
      59,    75,    51,    77,    34,    79,    56,    56,    56,    59,
     232,   233,   234,    56,   236,    34,    59,   197,   198,   199,
     132,    58,   134,   201,   204,   137,    58,   207,     6,   209,
      79,    80,   238,    56,   256,     4,    59,    27,   244,   259,
     246,    56,   264,    56,    59,    56,    59,    56,    59,   257,
      59,   152,   232,   233,   234,    36,   236,   237,   238,    -1,
     240,    -1,   242,     0,   244,    -1,   246,    46,   290,   291,
      49,    -1,    -1,    -1,    -1,    -1,   256,    -1,   258,   259,
     260,   261,    -1,   132,   264,   134,    -1,   199,   137,    -1,
      27,    -1,   204,    -1,    -1,   207,    -1,   209,    -1,   279,
      -1,    -1,    -1,    -1,    -1,   285,    -1,    -1,    -1,    -1,
     290,   291,    -1,   293,   294,    -1,    -1,    -1,    -1,    56,
     232,   233,   234,    -1,   236,   237,    -1,    -1,   240,    -1,
     242,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    29,   256,    31,   258,   259,   260,   261,
     199,    -1,   264,    -1,    -1,   204,    -1,    -1,   207,    -1,
     209,    -1,     0,    49,    50,    51,    -1,   279,    -1,    -1,
      -1,    -1,    -1,   285,    -1,    13,    -1,    -1,   290,   291,
      -1,   293,   294,   232,   233,   234,    -1,   236,   237,    27,
      -1,   240,    -1,   242,    80,    -1,    -1,   134,    -1,    -1,
     137,    -1,    -1,    -1,    -1,    -1,    44,   256,    -1,   258,
     259,   260,   261,    51,    -1,   264,    -1,    55,    56,    -1,
      -1,    -1,    -1,    -1,    19,    -1,    -1,    -1,    -1,    -1,
     279,    -1,    -1,    -1,    -1,    30,   285,    32,    33,    34,
      35,   290,   291,    -1,   293,   294,    -1,    -1,    43,    44,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    56,   199,    58,    -1,    60,    -1,    -1,   106,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      75,    -1,    77,    78,    79,    -1,    -1,     0,    -1,    -1,
      -1,     4,    -1,    -1,   132,    -1,   134,    -1,    -1,   137,
     237,    -1,    -1,   240,    -1,   242,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    31,    -1,
      -1,   258,   259,   260,   261,    -1,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,   279,    56,    -1,    -1,    -1,    -1,   285,    -1,
      -1,    -1,    37,    -1,    -1,    -1,   293,   294,     3,     4,
      -1,   199,    -1,     8,    -1,    -1,   204,    80,    -1,   207,
      -1,   209,    -1,    58,    19,    20,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,   232,   233,   234,    -1,   236,   237,
      -1,    -1,   240,    -1,   242,    -1,    -1,     3,     4,    -1,
      -1,    -1,    -1,    58,    59,    60,    61,    -1,   256,    -1,
     258,   259,   260,   261,    -1,    21,   264,    23,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   279,    -1,    -1,    89,    -1,    -1,   285,    -1,    -1,
      -1,    -1,   290,   291,    -1,   293,   294,   102,   103,    -1,
      -1,    -1,    58,   108,    60,    -1,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,    21,    -1,    23,
      24,    -1,    88,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,    19,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    58,    -1,    60,    -1,    -1,    41,
      -1,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      74,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    -1,
      -1,    -1,   197,   198,    88,    89,   201,    -1,    -1,    -1,
      -1,   206,    -1,    75,    -1,    77,   211,    79,    -1,   214,
     104,    -1,   106,    -1,    -1,    -1,    -1,   222,    -1,   224,
     225,     1,    -1,    -1,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    18,    19,
      20,   197,   198,    -1,   138,   201,    -1,    -1,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    -1,    -1,   152,    19,
     154,   266,   156,    43,    44,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    -1,    -1,    58,    -1,
      60,    -1,   238,    43,    44,    -1,    -1,    -1,   244,    -1,
     246,    -1,    -1,    -1,    -1,    75,    56,    77,    58,    79,
      60,   257,    -1,   197,   198,    -1,    37,    38,    -1,    -1,
      -1,    -1,    -1,    -1,   208,    75,    -1,    77,    78,    79,
      -1,    -1,    -1,   217,   218,    -1,   220,    58,    -1,    -1,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,   238,    -1,    -1,    -1,    -1,    -1,
     244,    -1,   246,    -1,   248,    -1,     1,   251,    -1,     4,
       5,    -1,    -1,   257,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    18,    19,    20,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    -1,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,     0,     1,    -1,
      -1,     4,     5,    58,    -1,    60,     9,    10,    11,    12,
      13,    -1,    -1,    16,    17,    18,    19,    20,    -1,    74,
      75,    -1,    77,    -1,    79,    80,    -1,    30,    -1,    32,
      33,    34,    35,    -1,     0,     1,    -1,    -1,     4,     5,
      43,    44,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      16,    17,    18,    19,    20,    58,    -1,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,
      -1,    -1,    75,    -1,    77,    -1,    79,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     0,     1,    -1,    -1,
       4,     5,    58,    -1,    60,     9,    10,    11,    12,    13,
      -1,    -1,    16,    17,    18,    19,    20,    -1,    -1,    75,
      -1,    77,    -1,    79,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    -1,    -1,     1,    -1,    -1,     4,     5,    43,
      44,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    18,    19,    20,    58,    -1,    60,    -1,    -1,    -1,
       4,    28,    29,    30,    -1,    32,    33,    34,    35,    -1,
      -1,    75,    -1,    77,    -1,    79,    43,    44,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    31,    -1,    56,
      -1,    58,    -1,    60,    -1,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    75,    -1,
      77,     1,    79,    -1,     4,     5,    -1,    -1,     8,     9,
      10,    11,    12,    13,    14,    15,    -1,    -1,    18,    19,
      20,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    -1,    -1,     1,    -1,
      -1,     4,     5,    43,    44,     8,     9,    10,    11,    12,
      13,    14,    15,    -1,    -1,    18,    19,    20,    58,    -1,
      60,    -1,    -1,    -1,     4,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    -1,    -1,    75,    -1,    77,    -1,    79,
      43,    44,    22,    23,    24,    25,    26,    27,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    58,    -1,    60,    -1,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    75,    -1,    77,     1,    79,    -1,     4,     5,
       6,    -1,     8,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    18,    19,    20,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,
       4,     5,    58,    -1,    60,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    18,    19,    20,    21,    -1,    75,
      -1,    77,    -1,    79,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    -1,    -1,     1,    -1,    -1,     4,     5,    43,
      44,     8,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    18,    19,    20,    58,    -1,    60,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,    -1,
      -1,    75,    -1,    77,    -1,    79,    43,    44,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    31,    -1,    -1,
      -1,    58,    -1,    60,    -1,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    75,    -1,
      77,     1,    79,    -1,     4,     5,    -1,    -1,     8,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    18,    19,
      20,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    -1,    -1,     1,    -1,
      -1,     4,     5,    43,    44,     8,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    18,    19,    20,    58,    -1,
      60,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    -1,    -1,    75,    -1,    77,    -1,    79,
      43,    44,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    31,    -1,    -1,    -1,    58,    -1,    60,    -1,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    75,    -1,    77,     1,    79,    -1,     4,     5,
      -1,    -1,     8,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    18,    19,    20,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,
      -1,    -1,     1,    -1,    -1,     4,     5,    43,    44,     8,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    18,
      19,    20,    58,    -1,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    32,    33,    34,    35,    -1,    -1,    75,
      -1,    77,    -1,    79,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,    -1,     4,     5,    58,
      -1,    60,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    18,    19,    20,    -1,    -1,    75,    -1,    77,    -1,
      79,    -1,    -1,    30,    -1,    32,    33,    34,    35,    -1,
      -1,     1,    -1,    -1,     4,     5,    43,    44,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    18,    19,
      20,    58,    -1,    60,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    -1,    19,    75,    -1,
      77,    -1,    79,    43,    44,    -1,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    -1,    -1,    -1,    -1,    58,    -1,
      60,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    75,    58,    77,    60,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    -1,    -1,
      -1,    -1,    -1,    75,    -1,    77,    78,    79,    30,    -1,
      32,    33,    34,    35,    -1,    19,    -1,    -1,    -1,    41,
      -1,    43,    44,    -1,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    -1,    -1,    -1,    19,    58,    -1,    60,    43,
      44,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    56,    75,    58,    77,    60,    79,    -1,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    75,    19,    77,    58,    79,    60,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    32,    33,    34,    35,    -1,
      19,    75,    -1,    77,    78,    79,    43,    44,    -1,    -1,
      -1,    30,    -1,    32,    33,    34,    35,    -1,    -1,    -1,
      19,    58,    -1,    60,    43,    44,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    32,    33,    34,    35,    -1,    75,    58,
      77,    60,    79,    -1,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    19,    77,    58,
      79,    60,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    -1,    -1,    75,    -1,    77,    -1,
      79,    43,    44,    22,    23,    24,    25,    26,    27,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    58,    -1,    60,    -1,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    75,    -1,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    23,    24,    25,    26,
      27,    -1,    -1,    -1,    31,    -1,    -1,    76,    -1,    -1,
      -1,    80,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,
      25,    26,    27,    -1,    -1,    -1,    31,    -1,    -1,    76,
      -1,    -1,    -1,    80,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    -1,    -1,    -1,    31,    -1,
      -1,    76,    -1,    -1,    -1,    80,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,    -1,
      -1,    22,    23,    24,    25,    26,    27,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    80,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    23,    24,    25,    26,    27,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    80,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    23,    24,    25,    26,
      27,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    80,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,
      25,    26,    27,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    80,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    23,    24,    25,    26,    27,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    80,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    -1,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    22,    23,    -1,
      -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    59,    22,    23,    -1,    -1,    -1,
      -1,    28,    29,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    80,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    59,    22,    23,    -1,    -1,    -1,    -1,    28,
      29,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    80
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     4,     5,     9,    10,    11,    12,    13,    16,
      17,    18,    19,    20,    30,    32,    33,    34,    35,    43,
      44,    58,    60,    75,    77,    79,    82,    83,    85,    86,
      92,    93,    94,   100,   101,   104,   105,   106,   107,   108,
     109,   111,   112,     4,    90,    92,    94,   104,   105,   107,
     112,    90,    34,   107,    16,    58,    84,    85,    58,    75,
      58,    75,   107,   107,    94,   101,    92,    41,    94,    96,
      97,   101,   112,    98,    99,   101,   104,     0,    86,    28,
      29,     0,     4,    56,    57,    87,    88,    74,    37,    58,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    75,   102,   103,    37,    38,    58,    74,
     103,    22,    23,    24,    25,    26,    27,    31,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    80,    34,   104,    84,    87,    87,    84,    37,     4,
     110,   105,    21,    85,    97,   107,   107,    97,   107,   107,
      59,    59,    56,    76,    57,    78,    56,    78,   101,    92,
      92,    34,    94,   101,   106,   112,    59,    97,   107,    94,
     107,   107,   101,   106,   107,    34,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,    58,    75,    84,
      37,     7,    91,     8,   101,     8,    14,    15,    37,    34,
      59,    42,    76,    59,    42,    76,    96,    99,    77,   101,
      77,    59,    42,    76,    42,    41,    34,    94,    95,   101,
       0,    89,    34,    90,     6,     8,    56,    84,    75,   107,
      84,   101,    84,   107,    58,   107,    58,    98,    99,    78,
      98,    99,   107,   107,   107,    56,    59,    58,    84,    84,
      84,    84,     8,    97,     4,     8,    56,     8,    59,    97,
      59,    97,    78,    78,    78,    78,    59,    59,    34,    84,
      95,    89,    91,     8,    76,    84,   107,    59,    59,    89,
      59,     4,    59,    84,    84,    89
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (pproot, errmsg, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location, pproot, errmsg); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, AstNode **pproot, char **errmsg)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, pproot, errmsg)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    AstNode **pproot;
    char **errmsg;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
  YYUSE (pproot);
  YYUSE (errmsg);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, AstNode **pproot, char **errmsg)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp, pproot, errmsg)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    AstNode **pproot;
    char **errmsg;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, pproot, errmsg);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, AstNode **pproot, char **errmsg)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule, pproot, errmsg)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
    AstNode **pproot;
    char **errmsg;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       , pproot, errmsg);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule, pproot, errmsg); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, AstNode **pproot, char **errmsg)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp, pproot, errmsg)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
    AstNode **pproot;
    char **errmsg;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (pproot);
  YYUSE (errmsg);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 33: /* "\"string\"" */

/* Line 1000 of yacc.c  */
#line 111 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding string \"%s\"\n", (yyvaluep->str));
#endif
  free((yyvaluep->str));
};

/* Line 1000 of yacc.c  */
#line 1857 "psycon.tab.c"
	break;
      case 34: /* "\"identifier\"" */

/* Line 1000 of yacc.c  */
#line 111 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding string \"%s\"\n", (yyvaluep->str));
#endif
  free((yyvaluep->str));
};

/* Line 1000 of yacc.c  */
#line 1871 "psycon.tab.c"
	break;
      case 83: /* "block_func" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1885 "psycon.tab.c"
	break;
      case 84: /* "block" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1899 "psycon.tab.c"
	break;
      case 85: /* "line" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1913 "psycon.tab.c"
	break;
      case 86: /* "line_func" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1927 "psycon.tab.c"
	break;
      case 90: /* "conditional" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1941 "psycon.tab.c"
	break;
      case 91: /* "elseif_list" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1955 "psycon.tab.c"
	break;
      case 92: /* "expcondition" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1969 "psycon.tab.c"
	break;
      case 93: /* "stmt" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1983 "psycon.tab.c"
	break;
      case 94: /* "condition" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1997 "psycon.tab.c"
	break;
      case 95: /* "id_list" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2011 "psycon.tab.c"
	break;
      case 96: /* "arg" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2025 "psycon.tab.c"
	break;
      case 97: /* "arg_list" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2039 "psycon.tab.c"
	break;
      case 98: /* "matrix" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2053 "psycon.tab.c"
	break;
      case 99: /* "vector" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2067 "psycon.tab.c"
	break;
      case 100: /* "range" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2081 "psycon.tab.c"
	break;
      case 101: /* "exp_range" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2095 "psycon.tab.c"
	break;
      case 102: /* "compop" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2109 "psycon.tab.c"
	break;
      case 103: /* "assign2this" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2123 "psycon.tab.c"
	break;
      case 104: /* "varblock" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2137 "psycon.tab.c"
	break;
      case 105: /* "tid" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2151 "psycon.tab.c"
	break;
      case 106: /* "assign" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2165 "psycon.tab.c"
	break;
      case 107: /* "exp" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2179 "psycon.tab.c"
	break;
      case 108: /* "func_static_or_not" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2193 "psycon.tab.c"
	break;
      case 109: /* "funcdef" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2207 "psycon.tab.c"
	break;
      case 110: /* "case_list" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2221 "psycon.tab.c"
	break;
      case 111: /* "csig" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2235 "psycon.tab.c"
	break;
      case 112: /* "initcell" */

/* Line 1000 of yacc.c  */
#line 104 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2249 "psycon.tab.c"
	break;

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (AstNode **pproot, char **errmsg);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (AstNode **pproot, char **errmsg)
#else
int
yyparse (pproot, errmsg)
    AstNode **pproot;
    char **errmsg;
#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[2];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif

/* User initialization code.  */

/* Line 1242 of yacc.c  */
#line 95 "psycon.y"
{
  if (ErrorMsg) {
	free(ErrorMsg);
	ErrorMsg = NULL;
  }
  *errmsg = NULL;
}

/* Line 1242 of yacc.c  */
#line 2410 "psycon.tab.c"

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);

	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 138 "psycon.y"
    { *pproot = NULL;;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 140 "psycon.y"
    { *pproot = (yyvsp[(1) - (1)].pnode);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 144 "psycon.y"
    { (yyval.pnode) = (yyvsp[(1) - (1)].pnode); ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 146 "psycon.y"
    { //yyn=5
		if ((yyvsp[(2) - (2)].pnode)) {
			if ((yyvsp[(1) - (2)].pnode) == NULL)
				(yyval.pnode) = (yyvsp[(2) - (2)].pnode);
			else if ((yyvsp[(1) - (2)].pnode)->type == N_BLOCK) 
			{
				(yyvsp[(1) - (2)].pnode)->tail->next = (yyvsp[(2) - (2)].pnode);
				(yyvsp[(1) - (2)].pnode)->tail = (yyvsp[(2) - (2)].pnode);
			} 
			else 
			{ // a=1; b=2; ==> $1->type is '='. So first, a N_BLOCK tree should be made.
				(yyval.pnode) = newAstNode(N_BLOCK, (yyloc));
				(yyval.pnode)->next = (yyvsp[(1) - (2)].pnode);
				(yyvsp[(1) - (2)].pnode)->next = (yyval.pnode)->tail = (yyvsp[(2) - (2)].pnode);
			}
		} else
			(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
	;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 167 "psycon.y"
    {
		if ((yyvsp[(1) - (1)].pnode)) // if cond1, x=1, end ==> x=1 comes here.
			(yyval.pnode) = (yyvsp[(1) - (1)].pnode);
		else
			(yyval.pnode) = newAstNode(N_BLOCK, (yyloc));
	;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 174 "psycon.y"
    {
		if ((yyvsp[(2) - (2)].pnode)) {
			if ((yyvsp[(1) - (2)].pnode)->type == N_BLOCK) {
				if ((yyval.pnode)->next) {
					(yyvsp[(1) - (2)].pnode)->tail->next = (yyvsp[(2) - (2)].pnode);
					(yyvsp[(1) - (2)].pnode)->tail = (yyvsp[(2) - (2)].pnode);
				} else {
					(yyval.pnode) = (yyvsp[(2) - (2)].pnode);
					free((yyvsp[(1) - (2)].pnode));
				}
			} else { //if the second argument doesn't have N_BLOCK, make one
				(yyval.pnode) = newAstNode(N_BLOCK, (yyloc));
				(yyval.pnode)->next = (yyvsp[(1) - (2)].pnode);
				(yyvsp[(1) - (2)].pnode)->next = (yyval.pnode)->tail = (yyvsp[(2) - (2)].pnode);
			}
		}
		else // only "block" is given
			(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
	;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 196 "psycon.y"
    { (yyval.pnode) = NULL;;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 198 "psycon.y"
    { //yyn=9
		(yyval.pnode) = NULL;
		yyerrok;
	;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 204 "psycon.y"
    { //yyn=11
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode); 
		(yyval.pnode)->suppress=1;
	;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 227 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_IF, (yyloc));
		(yyval.pnode)->col = 3923;
	;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 232 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_IF, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (4)].pnode);
		AstNode *p = (yyvsp[(3) - (4)].pnode);
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, (yylsp[(3) - (4)]));
			p->next = (yyvsp[(3) - (4)].pnode);
		}
		(yyvsp[(2) - (4)].pnode)->next = p;
		(yyval.pnode)->alt = (yyvsp[(4) - (4)].pnode);
		(yyval.pnode)->col = 4000;
	;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 246 "psycon.y"
    { //yyn=26; 
		AstNode *p = (yyvsp[(3) - (3)].pnode);
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, (yylsp[(3) - (3)]));				
			p->next = (yyvsp[(3) - (3)].pnode);
		}
		if ((yyvsp[(1) - (3)].pnode)->child==NULL) // if there's no elseif; i.e., elseif_list is empty
		{  
			yydeleteAstNode((yyvsp[(1) - (3)].pnode), 1);
			(yyval.pnode) = p;
		}
		else
		{
			(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
			(yyvsp[(1) - (3)].pnode)->alt = p;
		}
	;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 274 "psycon.y"
    { // yyn=32; This works, too, for "if cond, act; end" without else, because elseif_list can be empty
		(yyval.pnode) = newAstNode(T_IF, (yyloc));
		AstNode *p = (yyvsp[(3) - (5)].pnode);
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, (yylsp[(3) - (5)]));
			p->next = (yyvsp[(3) - (5)].pnode);
		}
		(yyval.pnode)->child = (yyvsp[(2) - (5)].pnode);
		(yyvsp[(2) - (5)].pnode)->next = p;
		AstNode *pElse = (yyvsp[(4) - (5)].pnode);
		if (pElse->type!=N_BLOCK)
		{
			pElse = newAstNode(N_BLOCK, (yylsp[(4) - (5)]));
			pElse->next = (yyvsp[(4) - (5)].pnode);
		}
		(yyval.pnode)->alt = pElse;
		if ((yyvsp[(4) - (5)].pnode)->child==NULL && (yyvsp[(4) - (5)].pnode)->next==NULL) // When elseif_list is empty, T_IF is made, but no child and next
		{
			yydeleteAstNode((yyval.pnode)->alt, 1);
			(yyval.pnode)->alt=NULL;
		}
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 300 "psycon.y"
    { // case is cascaded through alt
		(yyval.pnode) = (yyvsp[(3) - (4)].pnode);
		(yyval.pnode)->alt = (yyvsp[(3) - (4)].pnode)->alt;
		(yyval.pnode)->child = (yyvsp[(2) - (4)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 308 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(3) - (6)].pnode);
		(yyval.pnode)->alt = (yyvsp[(3) - (6)].pnode)->alt;
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		AstNode *p = newAstNode(T_OTHERWISE, (yylsp[(5) - (6)]));
		p->next = (yyvsp[(5) - (6)].pnode);
		(yyval.pnode)->tail = (yyvsp[(3) - (6)].pnode)->tail->alt = p;
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 319 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_TRY, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->alt = newAstNode(T_CATCH, (yylsp[(4) - (6)]));
		(yyval.pnode)->alt->child = newAstNode(T_ID, (yylsp[(4) - (6)]));
		(yyval.pnode)->alt->child->str = (yyvsp[(4) - (6)].str);
		(yyval.pnode)->alt->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 328 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_WHILE, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (4)].pnode);
		AstNode *p = (yyvsp[(3) - (4)].pnode);
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, (yylsp[(3) - (4)]));
			p->next = (yyvsp[(3) - (4)].pnode);
		}
		(yyval.pnode)->alt = p;
	;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 340 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_FOR, (yyloc));
		(yyval.pnode)->child = newAstNode(T_ID, (yylsp[(2) - (6)]));
		(yyval.pnode)->child->str = (yyvsp[(2) - (6)].str);
		(yyval.pnode)->child->child = (yyvsp[(4) - (6)].pnode);
		AstNode *p = (yyvsp[(5) - (6)].pnode);
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, (yylsp[(5) - (6)]));
			p->next = (yyvsp[(5) - (6)].pnode);
		}
		(yyval.pnode)->alt = p;
	;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 354 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_FOR, (yyloc));
		(yyval.pnode)->child = newAstNode(T_ID, (yylsp[(2) - (7)]));
		(yyval.pnode)->child->str = (yyvsp[(2) - (7)].str);
		(yyval.pnode)->child->child = (yyvsp[(4) - (7)].pnode);
		AstNode *p = (yyvsp[(6) - (7)].pnode);
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, (yylsp[(6) - (7)]));
			p->next = (yyvsp[(6) - (7)].pnode);
		}
		(yyval.pnode)->alt = p;
	;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 368 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_RETURN, (yyloc));
	;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 372 "psycon.y"
    { (yyval.pnode) = newAstNode(T_BREAK, (yyloc));;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 374 "psycon.y"
    { (yyval.pnode) = newAstNode(T_CONTINUE, (yyloc));;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 380 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('<', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 382 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('>', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 384 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_EQ, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 386 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_NE, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 388 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_GE, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 390 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_LE, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 392 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 398 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_LOGIC_NOT, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 403 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_AND, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 405 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_OR, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 409 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_IDLIST, (yyloc));
	;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 413 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_IDLIST, (yyloc));
		(yyval.pnode)->child = (yyval.pnode)->tail = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->tail->str = (yyvsp[(1) - (1)].str);
	;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 419 "psycon.y"
    {
		(yyvsp[(1) - (3)].pnode)->tail = (yyvsp[(1) - (3)].pnode)->tail->next = newAstNode(T_ID, (yylsp[(3) - (3)]));
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		(yyval.pnode)->tail->str = (yyvsp[(3) - (3)].str);
	;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 427 "psycon.y"
    {	(yyval.pnode) = newAstNode(T_FULLRANGE, (yyloc)); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 433 "psycon.y"
    { //yyn=58
		(yyval.pnode) = newAstNode(N_ARGS, (yyloc));
		(yyval.pnode)->tail = (yyval.pnode)->child = (yyvsp[(1) - (1)].pnode);
	;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 438 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		if ((yyval.pnode)->tail)
			(yyval.pnode)->tail = (yyval.pnode)->tail->next = (yyvsp[(3) - (3)].pnode);
		else
			(yyval.pnode)->tail = (yyval.pnode)->next = (yyvsp[(3) - (3)].pnode);
	;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 449 "psycon.y"
    {
	// N_MATRIX consists of "outer" N_MATRIX--alt for dot notation 
	// and "inner" N_VECTOR--alt for all successive items thru next
	// the str field of the outer N_MATRIX node is cast to the inner N_VECTOR.
	// this "fake" str pointer is freed during normal clean-up
	// 11/4/2019
		(yyval.pnode) = newAstNode(N_MATRIX, (yyloc));
		AstNode * p = newAstNode(N_VECTOR, (yyloc));
		(yyval.pnode)->str = (char*)p;
	;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 460 "psycon.y"
    { //yyn=62
		(yyval.pnode) = newAstNode(N_MATRIX, (yyloc));
		AstNode * p = newAstNode(N_VECTOR, (yyloc));
		p->alt = p->tail = (yyvsp[(1) - (1)].pnode);
		(yyval.pnode)->str = (char*)p;
	;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 467 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		AstNode * p = (AstNode *)(yyvsp[(1) - (3)].pnode)->str;
		p->tail = p->tail->next = (AstNode *)(yyvsp[(3) - (3)].pnode); 
	;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 476 "psycon.y"
    { //yyn=65
	// N_VECTOR consists of "outer" N_VECTOR--alt for dot notation 
	// and "inner" N_VECTOR--alt for all successive items thru next
	// Because N_VECTOR doesn't use str, the inner N_VECTOR is created there and cast for further uses.
	// this "fake" str pointer is freed during normal clean-up
	// 11/4/2019
		(yyval.pnode) = newAstNode(N_VECTOR, (yyloc));
		AstNode * p = newAstNode(N_VECTOR, (yyloc));
		p->alt = p->tail = (yyvsp[(1) - (1)].pnode);
		(yyval.pnode)->str = (char*)p;
	;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 488 "psycon.y"
    {
		AstNode * p = (AstNode *)(yyvsp[(1) - (2)].pnode)->str;
		p->tail = p->tail->next = (yyvsp[(2) - (2)].pnode);
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
	;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 494 "psycon.y"
    {
		AstNode * p = (AstNode *)(yyvsp[(1) - (3)].pnode)->str;
		p->tail = p->tail->next = (yyvsp[(3) - (3)].pnode);
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
	;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 502 "psycon.y"
    {
		(yyval.pnode) = makeFunctionCall(":", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));
	;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 506 "psycon.y"
    {//69
		(yyval.pnode) = makeFunctionCall(":", (yyvsp[(1) - (5)].pnode), (yyvsp[(5) - (5)].pnode), (yyloc));
		(yyvsp[(5) - (5)].pnode)->next = (yyvsp[(3) - (5)].pnode);
	;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 515 "psycon.y"
    { 	
		(yyval.pnode) = newAstNode('+', (yyloc));
	;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 519 "psycon.y"
    { 		(yyval.pnode) = newAstNode('-', (yyloc));	;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 521 "psycon.y"
    { 		(yyval.pnode) = newAstNode('*', (yyloc));	;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 523 "psycon.y"
    { 		(yyval.pnode) = newAstNode('/', (yyloc));	;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 525 "psycon.y"
    { 		(yyval.pnode) = newAstNode('@', (yyloc));	;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 527 "psycon.y"
    {
		(yyval.pnode) = newAstNode('@', (yyloc));	
		(yyval.pnode)->child = newAstNode('@', (yyloc));
	;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 532 "psycon.y"
    { 		(yyval.pnode) = newAstNode(T_OP_SHIFT, (yyloc));	;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 534 "psycon.y"
    { 		(yyval.pnode) = newAstNode('%', (yyloc));	;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 536 "psycon.y"
    { 		
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = strdup("movespec");
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_ARGS, (yyloc));
	;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 542 "psycon.y"
    { 		
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = strdup("respeed");
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_ARGS, (yyloc));
	;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 548 "psycon.y"
    { 		
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = strdup("timestretch");
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_ARGS, (yyloc));
	;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 554 "psycon.y"
    { 		
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = strdup("pitchscale");
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_ARGS, (yyloc));
	;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 562 "psycon.y"
    { //84
		(yyval.pnode) = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 566 "psycon.y"
    { // LHS OP RHS: if RHS is condition, OP should be either '=' or "++="
	  // For other OP's (such as += @= #= etc), condition on RHS is not allowed.
		(yyval.pnode) = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 571 "psycon.y"
    { 
		(yyval.pnode) = newAstNode(T_OP_CONCAT, (yyloc));
		AstNode *p = (yyval.pnode);
		if (p->alt)
			p = p->alt;
		p->child = 	newAstNode(T_REPLICA, (yylsp[(2) - (2)]));
		p->tail = p->child->next = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 580 "psycon.y"
    { 
		(yyval.pnode) = newAstNode(T_OP_CONCAT, (yyloc));	
		AstNode *p = (yyval.pnode);
		if (p->alt)
			p = p->alt;
		p->child = 	newAstNode(T_REPLICA, (yylsp[(2) - (2)]));
		p->tail = p->child->next = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 589 "psycon.y"
    { 
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
		if ((yyval.pnode)->child) // compop should be "@@=" and $$->child->type should be '@'  (64)
		{
			(yyval.pnode)->child->child = newAstNode(T_REPLICA, (yyloc));
			(yyval.pnode)->child->tail = (yyval.pnode)->child->child->next = newAstNode(T_REPLICA, (yyloc));
			(yyval.pnode)->tail = (yyval.pnode)->child->next = (yyvsp[(2) - (2)].pnode);
		}
		else
		{
			AstNode *p = (yyval.pnode);
			if (p->alt)
				p = p->alt;
			p->child = 	newAstNode(T_REPLICA, (yylsp[(2) - (2)]));
			p->tail = p->child->next = (yyvsp[(2) - (2)].pnode);
		}
	;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 609 "psycon.y"
    { //89
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (1)].str);
	;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 614 "psycon.y"
    {//90
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		AstNode *p = newAstNode(N_STRUCT, (yyloc));
		p->str = (yyvsp[(3) - (3)].str);
		if ((yyval.pnode)->type==N_CELL)
		{
			(yyval.pnode)->alt->alt = p; //always initiating
			(yyval.pnode)->tail = p; // so that next concatenation can point to p, even thought p is "hidden" underneath $$->alt
		}
		if ((yyval.pnode)->tail) 
		{
			(yyval.pnode)->tail = (yyval.pnode)->tail->alt = p;
		}
		else
		{
			(yyval.pnode)->tail = (yyval.pnode)->alt = p;
		}
	;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 633 "psycon.y"
    { // Questionable. What is this? Marked 7/5/2020
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		AstNode *p = newAstNode(N_STRUCT, (yyloc));
		p->str = (yyvsp[(3) - (3)].str);
		//Concatenated dot expressions go through $$->tail
		if ((yyval.pnode)->tail!=NULL)
			(yyval.pnode)->tail->alt = p; // update $$->tail
		else
			(yyval.pnode)->alt = p; // initiate $$->tail
		(yyval.pnode)->tail = p;
	;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 645 "psycon.y"
    {//92
		(yyval.pnode) = (yyvsp[(1) - (4)].pnode);
		(yyval.pnode)->tail = (yyval.pnode)->alt->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->tail->child = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 651 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (6)].pnode);
		(yyval.pnode)->tail = (yyval.pnode)->tail->alt = newAstNode(N_TIME_EXTRACT, (yylsp[(2) - (6)]));
		(yyval.pnode)->tail->child = (yyvsp[(3) - (6)].pnode);
		(yyval.pnode)->tail->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 658 "psycon.y"
    {//tid-vector 94
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
	;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 662 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_HOOK, (yyloc));
		(yyval.pnode)->str = strdup((yyvsp[(2) - (2)].pnode)->str);
		(yyval.pnode)->alt = (yyvsp[(2) - (2)].pnode)->alt;
	;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 671 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (4)].str);
		(yyval.pnode)->tail = (yyval.pnode)->alt = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 677 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (4)].str);
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->alt->child = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 684 "psycon.y"
    { //99
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (7)].str);
		(yyval.pnode)->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->alt->child = (yyvsp[(3) - (7)].pnode);
		AstNode *p = (yyvsp[(6) - (7)].pnode)->child;
		if (p->type==T_ID && !strcmp(p->str,"respeed"))
		{ // x{2}(t1~t2) checks here because t1~t2 can be arg_list through
		  // exp:exp '~' exp --> exp_range:exp --> arg:exp_range --> arg_list:arg
		  // that's why T_ID '{' exp '}' '(' exp '~' exp ')' creates a conflict and can't be a node
		  // Take care of it manually here, but it's better to re-desig grammar rules in the future. 7/5/2020
			(yyval.pnode)->tail = (yyval.pnode)->alt->alt = newAstNode(N_TIME_EXTRACT, (yylsp[(2) - (7)]));
			(yyval.pnode)->alt->alt->child = p->alt->child;
			(yyval.pnode)->alt->alt->child->next = p->alt->child->next;
			p->alt->child = NULL;
			yydeleteAstNode(p, 1);
		}
		else
			(yyval.pnode)->tail = (yyval.pnode)->alt->alt = (yyvsp[(6) - (7)].pnode);
		
	;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 706 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (6)].str);
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_TIME_EXTRACT, (yylsp[(2) - (6)]));
		(yyval.pnode)->tail->child = (yyvsp[(3) - (6)].pnode);
		(yyvsp[(3) - (6)].pnode)->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 714 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (4)].pnode);
		if ((yyval.pnode)->tail)
			(yyval.pnode)->tail = (yyval.pnode)->tail->alt = (yyvsp[(3) - (4)].pnode); // just remember--- this is different from $$->tail->alt = $$->tail = $3; 8/20/2018
		else
			(yyval.pnode)->tail = (yyval.pnode)->alt = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 722 "psycon.y"
    { // 102
		if ((yyval.pnode)->alt != NULL  && (yyval.pnode)->alt->type==N_STRUCT)
		{ // dot notation with a blank parentheses, e.g., a.sqrt() or (1:2:5).sqrt()
			(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		}
		else // no longer used.,,.. absorbed by tid:  T_ID
		{ // udf_func()
			(yyval.pnode) = newAstNode(N_CALL, (yyloc));
			(yyval.pnode)->str = getT_ID_str((yyvsp[(1) - (3)].pnode));
		}
	;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 734 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
	;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 738 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
		(yyval.pnode)->tail = (yyval.pnode)->alt = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 743 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->alt->child = (yyvsp[(3) - (4)].pnode);
		(yyval.pnode)->tail = (yyval.pnode)->alt = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 750 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
		(yyval.pnode)->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->alt->child = (yyvsp[(3) - (7)].pnode);
		(yyval.pnode)->tail = (yyval.pnode)->alt->alt = (yyvsp[(6) - (7)].pnode);
	;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 757 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_TIME_EXTRACT, (yylsp[(2) - (6)]));
		(yyval.pnode)->tail->child = (yyvsp[(3) - (6)].pnode);
		(yyvsp[(3) - (6)].pnode)->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 764 "psycon.y"
    { 	//108
		(yyval.pnode) = newAstNode(T_TRANSPOSE, (yyloc));
 		(yyval.pnode)->child = (yyvsp[(1) - (2)].pnode);
	;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 769 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (5)].pnode);
	;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 774 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 780 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 786 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->str = malloc(8);
		strcpy((yyval.pnode)->str, "R");
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 794 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->str = malloc(8);
		strcpy((yyval.pnode)->str, "R");
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 802 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
	;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 806 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 815 "psycon.y"
    { //116
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
		(yyval.pnode)->child = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 820 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
		(yyval.pnode)->child = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 825 "psycon.y"
    { //c=a(2)=44
		if (!(yyvsp[(1) - (3)].pnode)->child) 
			(yyvsp[(1) - (3)].pnode)->child = (yyvsp[(3) - (3)].pnode);
		else
			for (AstNode *p = (yyvsp[(1) - (3)].pnode)->child; p; p=p->child)
			{
				if (!p->child) 
				{
					p->child = (yyvsp[(3) - (3)].pnode);
					break;
				}
			}
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
	;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 840 "psycon.y"
    { //a(2)=d=11
		if (!(yyvsp[(1) - (3)].pnode)->child) 
			(yyvsp[(1) - (3)].pnode)->child = (yyvsp[(3) - (3)].pnode);
		else
			for (AstNode *p = (yyvsp[(1) - (3)].pnode)->child; p; p=p->child)
			{
				if (!p->child) 
				{
					p->child = (yyvsp[(3) - (3)].pnode);
					break;
				}
			}
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
	;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 855 "psycon.y"
    { // x={"bjk",noise(300), 4.5555}
		(yyval.pnode)->str = getT_ID_str((yyvsp[(1) - (3)].pnode));
		(yyval.pnode)->child = (yyvsp[(3) - (3)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 865 "psycon.y"
    { // 121
		(yyval.pnode) = newAstNode(T_NUMBER, (yyloc));
		(yyval.pnode)->dval = (yyvsp[(1) - (1)].dval);
	;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 870 "psycon.y"
    { // 122
		(yyval.pnode) = newAstNode(T_STRING, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (1)].str);
	;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 875 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ENDPOINT, (yyloc));
	;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 880 "psycon.y"
    {//125
		(yyval.pnode) = (yyvsp[(1) - (1)].pnode);
	;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 884 "psycon.y"
    {
		if ((yyvsp[(1) - (6)].pnode)->type==N_CELL)
		{
			(yyval.pnode) = (yyvsp[(1) - (6)].pnode);
			(yyval.pnode)->child = newAstNode(N_TIME_EXTRACT, (yyloc));
			(yyval.pnode)->child->child = (yyvsp[(3) - (6)].pnode);
		}
		else
		{
			(yyval.pnode) = newAstNode(N_TIME_EXTRACT, (yyloc));
			(yyval.pnode)->str = getT_ID_str((yyvsp[(1) - (6)].pnode));
			(yyval.pnode)->child = (yyvsp[(3) - (6)].pnode);
		}
		(yyvsp[(3) - (6)].pnode)->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 900 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_NEGATIVE, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 905 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (2)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 911 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_SIGMA, (yyloc));
		(yyval.pnode)->child = newAstNode(T_ID, (yylsp[(3) - (8)]));
		(yyval.pnode)->child->str = getT_ID_str((yyvsp[(3) - (8)].pnode));
		(yyval.pnode)->child->child = (yyvsp[(5) - (8)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(7) - (8)].pnode);
	;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 919 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('+', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 921 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('-', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 923 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('*', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 925 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('/', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 927 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_MATRIXMULT, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 929 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("^", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 931 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("mod", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 933 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("respeed", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 935 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("pitchscale", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 937 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("timestretch", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 939 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("movespec", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 941 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('@', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 943 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_OP_SHIFT, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 945 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_OP_CONCAT, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 949 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_FUNCTION, (yyloc));
		(yyval.pnode)->suppress = 2;
	;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 954 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_FUNCTION, (yyloc));
		(yyval.pnode)->suppress = 3;
	;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 961 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (4)].pnode);
		(yyval.pnode)->str = (yyvsp[(2) - (4)].str);
		(yyval.pnode)->child = newAstNode(N_IDLIST, (yyloc));
		(yyval.pnode)->child->next = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 968 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (6)].pnode);
		(yyval.pnode)->str = (yyvsp[(4) - (6)].str);
		(yyval.pnode)->child = newAstNode(N_IDLIST, (yyloc));
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
		if ((yyvsp[(2) - (6)].pnode)->type!=N_VECTOR)
		{
			(yyval.pnode)->alt = newAstNode(N_VECTOR, (yylsp[(2) - (6)]));
			AstNode *p = newAstNode(N_VECTOR, (yylsp[(2) - (6)]));
			p->alt = p->tail = (yyvsp[(2) - (6)].pnode);
			(yyval.pnode)->alt->str = (char*)p;
		}
		else
		{
			(yyval.pnode)->alt = (yyvsp[(2) - (6)].pnode);
		}
	;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 986 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (7)].pnode);
		(yyval.pnode)->str = (yyvsp[(2) - (7)].str);
		(yyval.pnode)->child = (yyvsp[(4) - (7)].pnode);
		(yyvsp[(4) - (7)].pnode)->next = (yyvsp[(6) - (7)].pnode);
	;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 993 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (9)].pnode);
		(yyval.pnode)->str = (yyvsp[(4) - (9)].str);
		(yyval.pnode)->child = (yyvsp[(6) - (9)].pnode);
		(yyvsp[(6) - (9)].pnode)->next = (yyvsp[(8) - (9)].pnode);
		if ((yyvsp[(2) - (9)].pnode)->type!=N_VECTOR)
		{
			(yyval.pnode)->alt = newAstNode(N_VECTOR, (yylsp[(2) - (9)]));
			AstNode *p = newAstNode(N_VECTOR, (yylsp[(2) - (9)]));
			p->alt = p->tail = (yyvsp[(2) - (9)].pnode);
			(yyval.pnode)->alt->str = (char*)p;
		}
		else
		{
			(yyval.pnode)->alt = (yyvsp[(2) - (9)].pnode);
		}
	;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1013 "psycon.y"
    { (yyval.pnode) = newAstNode(T_SWITCH, (yyloc));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1015 "psycon.y"
    { (yyval.pnode) = newAstNode(T_SWITCH, (yyloc));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1017 "psycon.y"
    {
		if ((yyvsp[(1) - (5)].pnode)->alt)
			(yyvsp[(1) - (5)].pnode)->tail->alt = (yyvsp[(3) - (5)].pnode);
		else
			(yyvsp[(1) - (5)].pnode)->alt = (yyvsp[(3) - (5)].pnode);
		AstNode *p = (yyvsp[(5) - (5)].pnode);
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, (yylsp[(5) - (5)]));
			p->next = (yyvsp[(5) - (5)].pnode);
		}
		(yyvsp[(1) - (5)].pnode)->tail = (yyvsp[(3) - (5)].pnode)->next = p;
		(yyval.pnode) = (yyvsp[(1) - (5)].pnode);
	;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1032 "psycon.y"
    {
		if ((yyvsp[(1) - (7)].pnode)->alt)
			(yyvsp[(1) - (7)].pnode)->tail->alt = (yyvsp[(4) - (7)].pnode);
		else
			(yyvsp[(1) - (7)].pnode)->alt = (yyvsp[(4) - (7)].pnode);
		AstNode *p = (yyvsp[(7) - (7)].pnode);
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, (yylsp[(7) - (7)]));
			p->next = (yyvsp[(7) - (7)].pnode);
		}
		(yyvsp[(1) - (7)].pnode)->tail = (yyvsp[(4) - (7)].pnode)->next = p;
		(yyval.pnode) = (yyvsp[(1) - (7)].pnode);
	;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1052 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
		(yyval.pnode)->type = N_INITCELL;
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;



/* Line 1455 of yacc.c  */
#line 4070 "psycon.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (pproot, errmsg, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (pproot, errmsg, yymsg);
	  }
	else
	  {
	    yyerror (pproot, errmsg, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc, pproot, errmsg);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp, pproot, errmsg);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (pproot, errmsg, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc, pproot, errmsg);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp, pproot, errmsg);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 1074 "psycon.y"


/* Called by yyparse on error. */
void yyerror (AstNode **pproot, char **errmsg, char const *s)
{
  static size_t errmsg_len = 0;
#define ERRMSG_MAX 999
  char msgbuf[ERRMSG_MAX], *p;
  size_t msglen;

  sprintf_s(msgbuf, ERRMSG_MAX, "Invalid syntax: Line %d, Col %d: %s.\n", yylloc.first_line, yylloc.first_column, s + (strncmp(s, "syntax error, ", 14) ? 0 : 14));
  if ((p=strstr(msgbuf, "$undefined"))) {
	sprintf_s(p, 10, "'%c'(%d)", yychar, yychar);
    strcpy(p+strlen(p), p+10);
  }
  if ((p=strstr(msgbuf, "end of text or ")))
    strcpy(p, p+15);
  if ((p=strstr(msgbuf, " or ','")))
    strcpy(p, p+7);
  msglen = strlen(msgbuf);
  if (ErrorMsg == NULL)
    errmsg_len = 0;
  ErrorMsg = (char *)realloc(ErrorMsg, errmsg_len+msglen+1);
  strcpy_s(ErrorMsg+errmsg_len, msglen+1, msgbuf);
  errmsg_len += msglen;
  *errmsg = ErrorMsg;
}


int getTokenID(const char *str)
{
	size_t len, i;
	len = strlen(str);
	for (i = 0; i < YYNTOKENS; i++) {
		if (yytname[i] != 0
			&& yytname[i][0] == '"'
			&& !strncmp (yytname[i] + 1, str, len)
			&& yytname[i][len + 1] == '"'
			&& yytname[i][len + 2] == 0)
				break;
	}
	if (i < YYNTOKENS)
		return yytoknum[i];
	else
		return T_UNKNOWN;
}


void print_token_value(FILE *file, int type, YYSTYPE value)
{
	if (type == T_ID)
		fprintf (file, "%s", value.str);
	else if (type == T_NUMBER)
		fprintf (file, "%f", value.dval);
}


char *getAstNodeName(AstNode *p)
{
#define N_NAME_MAX 99
  static char buf[N_NAME_MAX];

  if (!p)
	return NULL;
  switch (p->type) {
  case '=':
    sprintf_s(buf, N_NAME_MAX, "[%s=]", p->str);
    break;
  case T_ID:
    sprintf_s(buf, N_NAME_MAX, "[%s]", p->str);
    break;
  case T_STRING:
    sprintf_s(buf, N_NAME_MAX, "\"%s\"", p->str);
    break;
  case N_CALL:
    sprintf_s(buf, N_NAME_MAX, "%s()", p->str);
    break;
  case N_CELL:
    sprintf_s(buf, N_NAME_MAX, "%s()", p->str);
    break;
  case T_NUMBER:
    sprintf_s(buf, N_NAME_MAX, "%.1f", p->dval);
    break;
  case N_BLOCK:
    sprintf_s(buf, N_NAME_MAX, "BLOCK");
    break;
  case N_ARGS:
    sprintf_s(buf, N_NAME_MAX, "ARGS");
    break;
  case N_MATRIX:
    sprintf_s(buf, N_NAME_MAX, "MATRIX");
    break;
  case N_VECTOR:
    sprintf_s(buf, N_NAME_MAX, "VECTOR");
    break;
  case N_IDLIST:
    sprintf_s(buf, N_NAME_MAX, "ID_LIST");
    break;
  case N_TIME_EXTRACT:
    sprintf_s(buf, N_NAME_MAX, "TIME_EXTRACT");
    break;
  case N_CELLASSIGN:
    sprintf_s(buf, N_NAME_MAX, "INITCELL");
    break;
  case N_IXASSIGN:
    sprintf_s(buf, N_NAME_MAX, "ASSIGN1");
    break;
  default:
    if (YYTRANSLATE(p->type) == 2)
      sprintf_s(buf, N_NAME_MAX, "[%d]", p->type);
    else
      sprintf_s(buf, N_NAME_MAX, "%s", yytname[YYTRANSLATE(p->type)]);
  }
  return buf;
}

/* As of 4/17/2018
In makeFunctionCall and makeBinaryOpNode, 
node->tail is removed, because it caused conflict with tail made in 
tid: tid '.' T_ID or tid: tid '(' arg_list ')'
or possibly other things.
The only downside from this change is, during debugging, the last argument is not seen at the top node where several nodes are cascaded: e.g., a+b+c
*/

AstNode *makeFunctionCall(char *name, AstNode *first, AstNode *second, YYLTYPE loc)
{
	AstNode *node;

	node = newAstNode(T_ID, loc);
	node->str = strdup(name);
	node->tail = node->alt = newAstNode(N_ARGS, loc);
	node->alt->child = first;
	first->next = second;
	return node;
}

AstNode *makeBinaryOpNode(int op, AstNode *first, AstNode *second, YYLTYPE loc)
{
	AstNode *node;

	node = newAstNode(op, loc);
	node->child = first;
	first->next = second;
	return node;
}

AstNode *newAstNode(int type, YYLTYPE loc)
{
#ifdef DEBUG
    static int cnt=0;
#endif
  AstNode *node;

  node = (AstNode *)malloc(sizeof(AstNode));
  if (node==NULL)
    exit(2);
  memset(node, 0, sizeof(AstNode));
  node->type = type;
#ifdef DEBUG
    printf("created node %d: %s\n", ++cnt, getAstNodeName(node));
#endif
  node->line = loc.first_line;
  node->col = loc.first_column;
  return node;
}

char *getT_ID_str(AstNode *p)
{
	if (p->type==T_ID)
		return p->str;
	printf("Must be T_ID\n");
	return NULL;
}


int yydeleteAstNode(AstNode *p, int fSkipNext)
{
#ifdef DEBUG
    static int cnt=0;
#endif
  AstNode *tmp, *next;
  
  if (!p)
	return 0;
#ifdef DEBUG
    printf("deleting node %d: %s\n", ++cnt, getAstNodeName(p));
#endif
  if (p->str)
    free(p->str);
  if (p->child)
    yydeleteAstNode(p->child, 0);
  if (!fSkipNext && p->next) {
	for (tmp=p->next; tmp; tmp=next) {
      next = tmp->next;
      yydeleteAstNode(tmp, 1);
    }
  }
  free(p);
  return 0;
}

int yyPrintf(const char *msg, AstNode *p)
{
	if (p)
		printf("[%16s]token type: %d, %s, \n", msg, p->type, p->str);
	return 1;
}
