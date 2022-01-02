
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
#define YYPRINT(file, type, value) print_token_value (file, type, value)
/*#define DEBUG*/

char *ErrorMsg = NULL;
int yylex (void);
void yyerror (AstNode **pproot, char **errmsg, char const *s);


/* Line 189 of yacc.c  */
#line 87 "psycon.tab.c"

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
#line 34 "psycon.y"

	double dval;
	char *str;
	AstNode *pnode;



/* Line 214 of yacc.c  */
#line 172 "psycon.tab.c"
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
#line 117 "psycon.y"

AstNode *newAstNode(int type, YYLTYPE loc);
AstNode *makeFunctionCall(const char *name, AstNode *first, AstNode *second, YYLTYPE loc);
AstNode *makeBinaryOpNode(int op, AstNode *first, AstNode *second, YYLTYPE loc);
void print_token_value(FILE *file, int type, YYSTYPE value);
char *getT_ID_str(AstNode *p);
void handle_tilde(AstNode *proot, AstNode *pp, YYLTYPE loc);


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
#define YYFINAL  78
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1980

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  81
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  32
/* YYNRULES -- Number of rules.  */
#define YYNRULES  148
/* YYNRULES -- Number of states.  */
#define YYNSTATES  277

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
       2,     2,     2,    62,     2,    48,    79,    46,     2,    38,
      58,    59,    49,    44,    56,    43,    76,    50,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    41,    57,
      39,    37,    40,     2,    47,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    77,     2,    78,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    60,     2,    61,    42,     2,     2,     2,
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
      35,    36,    45,    52,    53,    54,    55,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      80
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    11,    13,    16,    18,
      21,    24,    27,    29,    31,    33,    35,    37,    39,    40,
      42,    44,    47,    52,    59,    67,    77,    78,    80,    86,
      94,    96,    98,   100,   106,   111,   118,   125,   130,   137,
     145,   147,   149,   151,   153,   156,   158,   161,   162,   167,
     171,   173,   175,   179,   183,   187,   191,   195,   199,   203,
     207,   210,   214,   218,   219,   221,   225,   227,   229,   231,
     233,   237,   238,   240,   244,   247,   249,   252,   256,   260,
     266,   268,   270,   272,   274,   276,   278,   280,   282,   284,
     286,   288,   290,   292,   294,   296,   299,   302,   305,   308,
     310,   314,   319,   323,   326,   328,   333,   338,   346,   351,
     355,   357,   362,   367,   375,   378,   384,   391,   398,   405,
     412,   416,   420,   423,   426,   430,   434,   438,   440,   442,
     444,   446,   448,   451,   454,   463,   467,   471,   475,   479,
     483,   487,   491,   495,   499,   503,   507,   511,   515
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      82,     0,    -1,    -1,    83,    -1,    86,    -1,    83,    86,
      -1,    85,    -1,    84,    85,    -1,     4,    -1,     1,     4,
      -1,    93,    87,    -1,    93,    88,    -1,    85,    -1,    91,
      -1,    56,    -1,     4,    -1,     0,    -1,    57,    -1,    -1,
       0,    -1,    16,    -1,    17,    16,    -1,    90,    34,    84,
      89,    -1,    90,   109,    37,    34,    84,    89,    -1,    90,
      34,    58,   100,    59,    84,    89,    -1,    90,   109,    37,
      34,    58,   100,    59,    84,    89,    -1,    -1,     4,    -1,
      92,    14,   112,     4,    84,    -1,    92,    14,    60,   102,
      61,     4,    84,    -1,    96,    -1,   111,    -1,    98,    -1,
       5,    94,    84,    95,     8,    -1,    13,   112,    92,     8,
      -1,    13,   112,    92,    15,    84,     8,    -1,    20,    84,
      21,    34,    84,     8,    -1,     9,    94,    84,     8,    -1,
      10,    34,    37,   106,    84,     8,    -1,    10,    34,    37,
     106,    56,    84,     8,    -1,    18,    -1,    11,    -1,    12,
      -1,    99,    -1,    99,    87,    -1,   112,    -1,   112,    87,
      -1,    -1,     7,    94,    84,    95,    -1,    95,     6,    84,
      -1,    97,    -1,   106,    -1,    60,   102,    61,    -1,    58,
      99,    59,    -1,   112,    39,   112,    -1,   112,    40,   112,
      -1,   112,    24,   112,    -1,   112,    25,   112,    -1,   112,
      27,   112,    -1,   112,    26,   112,    -1,    62,    96,    -1,
      96,    28,    96,    -1,    96,    29,    96,    -1,    -1,    34,
      -1,   100,    56,    34,    -1,    41,    -1,   106,    -1,    98,
      -1,   101,    -1,   102,    56,   101,    -1,    -1,   104,    -1,
     103,    57,   104,    -1,   103,    57,    -1,   106,    -1,   104,
     106,    -1,   104,    56,   106,    -1,   112,    41,   112,    -1,
     112,    41,   112,    41,   112,    -1,   112,    -1,   105,    -1,
      99,    -1,    63,    -1,    64,    -1,    65,    -1,    66,    -1,
      67,    -1,    68,    -1,    69,    -1,    70,    -1,    71,    -1,
      72,    -1,    73,    -1,    74,    -1,    37,   106,    -1,    75,
      99,    -1,    75,   112,    -1,   107,   106,    -1,    34,    -1,
     110,    76,    34,    -1,   109,    60,   112,    61,    -1,    77,
     104,    78,    -1,    79,   109,    -1,   109,    -1,    34,    58,
     102,    59,    -1,    34,    60,   112,    61,    -1,    34,    60,
     112,    61,    58,   102,    59,    -1,   109,    58,   102,    59,
      -1,   109,    58,    59,    -1,    30,    -1,    30,    58,   102,
      59,    -1,    30,    60,   112,    61,    -1,    30,    60,   112,
      61,    58,   102,    59,    -1,   110,    38,    -1,    77,   104,
      78,    77,    78,    -1,    77,   104,    78,    77,   104,    78,
      -1,    77,   104,    78,    77,   103,    78,    -1,    77,   103,
      78,    77,   104,    78,    -1,    77,   103,    78,    77,   103,
      78,    -1,    77,   103,    78,    -1,    58,   106,    59,    -1,
     110,   108,    -1,   109,   108,    -1,   109,    37,   111,    -1,
     110,    37,   111,    -1,   109,    37,    98,    -1,    98,    -1,
     110,    -1,    32,    -1,    33,    -1,    35,    -1,    43,   112,
      -1,    44,   112,    -1,    19,    58,   110,    37,   106,    56,
     112,    59,    -1,   112,    44,   112,    -1,   112,    43,   112,
      -1,   112,    49,   112,    -1,   112,    50,   112,    -1,   112,
      31,   112,    -1,   112,    51,   112,    -1,   112,    46,   112,
      -1,   112,    42,   112,    -1,   112,    48,   112,    -1,   112,
      80,   112,    -1,   112,    45,   112,    -1,   112,    47,   112,
      -1,   112,    22,   112,    -1,   112,    23,   112,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   143,   143,   144,   148,   149,   170,   177,   199,   201,
     206,   207,   214,   215,   218,   218,   218,   221,   224,   225,
     228,   233,   240,   247,   265,   272,   293,   294,   296,   311,
     328,   329,   330,   331,   357,   365,   376,   385,   397,   411,
     425,   429,   431,   436,   436,   436,   436,   440,   443,   456,
     477,   480,   483,   492,   498,   500,   502,   504,   506,   508,
     510,   515,   517,   522,   525,   531,   539,   541,   542,   545,
     550,   561,   571,   578,   584,   587,   599,   605,   613,   617,
     624,   624,   624,   627,   631,   633,   635,   637,   639,   644,
     646,   648,   655,   662,   669,   678,   682,   691,   700,   720,
     725,   744,   750,   754,   763,   764,   770,   777,   786,   791,
     803,   807,   812,   818,   826,   831,   836,   842,   848,   856,
     864,   868,   877,   882,   887,   902,   917,   926,   927,   928,
     933,   938,   942,   947,   953,   961,   963,   965,   967,   969,
     971,   973,   975,   977,   979,   981,   983,   985,   987
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
  "'{'", "'}'", "'!'", "\"+=\"", "\"-=\"", "\"*=\"", "\"/=\"", "\"@=\"",
  "\"@@=\"", "\">>=\"", "\"%=\"", "\"->=\"", "\"~=\"", "\"<>=\"", "\"#=\"",
  "\"++=\"", "'.'", "'['", "']'", "'$'", "\"<>\"", "$accept", "input",
  "block_func", "block", "line", "line_func", "eol", "eol2", "func_end",
  "func_decl", "funcdef", "case_list", "stmt", "conditional",
  "elseif_list", "expcondition", "csig", "initcell", "condition",
  "id_list", "arg", "arg_list", "matrix", "vector", "range", "exp_range",
  "compop", "assign2this", "varblock", "tid", "assign", "exp", 0
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
     123,   125,    33,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,    46,    91,    93,    36,
     310
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    81,    82,    82,    83,    83,    84,    84,    85,    85,
      85,    85,    86,    86,    87,    87,    87,    88,    89,    89,
      90,    90,    91,    91,    91,    91,    92,    92,    92,    92,
      93,    93,    93,    93,    93,    93,    93,    93,    93,    93,
      93,    93,    93,    94,    94,    94,    94,    95,    95,    95,
      96,    97,    98,    99,    99,    99,    99,    99,    99,    99,
      99,    99,    99,   100,   100,   100,   101,   101,   101,   102,
     102,   103,   103,   103,   103,   104,   104,   104,   105,   105,
     106,   106,   106,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   108,   108,   108,   108,   109,
     109,   109,   109,   109,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   111,   111,   111,   111,   111,   112,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     1,     2,     1,     2,
       2,     2,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     2,     4,     6,     7,     9,     0,     1,     5,     7,
       1,     1,     1,     5,     4,     6,     6,     4,     6,     7,
       1,     1,     1,     1,     2,     1,     2,     0,     4,     3,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     3,     3,     0,     1,     3,     1,     1,     1,     1,
       3,     0,     1,     3,     2,     1,     2,     3,     3,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     2,     2,     1,
       3,     4,     3,     2,     1,     4,     4,     7,     4,     3,
       1,     4,     4,     7,     2,     5,     6,     6,     6,     6,
       3,     3,     2,     2,     3,     3,     3,     1,     1,     1,
       1,     1,     2,     2,     8,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     8,     0,     0,     0,    41,    42,     0,    20,
       0,    40,     0,     0,   110,   129,   130,    99,   131,     0,
       0,     0,     0,     0,    71,     0,     0,     0,    12,     4,
       0,    13,     0,    30,    50,   127,    82,    81,    51,   104,
     128,    31,    80,     9,     0,     0,   127,    43,   104,   128,
      45,     0,     0,     0,    26,    21,     0,     0,     6,     0,
       0,     0,     0,   132,   133,    82,    51,    66,   127,    69,
       0,    67,    60,     0,    72,    75,   103,     0,     1,     5,
       0,   104,    16,    15,    14,    17,    10,    11,     0,     0,
       0,     0,     0,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     0,     0,   123,     0,   114,
       0,   122,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    44,    46,     0,     0,    27,     0,
       0,     0,     7,     0,     0,     0,     0,    53,   121,     0,
      52,    74,   120,     0,   102,    76,    63,     0,     0,     0,
      61,    62,   127,    95,   124,   109,     0,     0,    96,    97,
      98,   125,   100,   147,   148,    56,    57,    59,    58,   139,
      54,    55,    78,   142,   136,   135,   145,   141,   146,   143,
     137,   138,   140,   144,     0,     0,    37,     0,    34,     0,
       0,     0,     0,   111,   112,   105,   106,    70,    73,    71,
      77,    71,    99,     0,    51,    80,    19,    22,     0,   108,
     101,     0,     0,     0,    33,     0,     0,     0,     0,     0,
      51,     0,     0,     0,     0,    72,   115,     0,    72,     0,
       0,    63,     0,    79,     0,     0,     0,    38,     0,     0,
      35,     0,    36,     0,     0,   119,   118,   117,   116,    65,
       0,     0,    23,    48,    39,    52,     0,     0,   113,   107,
      24,     0,     0,   134,     0,     0,    25
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    26,    27,    57,    58,    29,    86,    87,   217,    30,
      31,   139,    32,    44,   195,    33,    34,    46,    36,   213,
      69,    70,    73,    74,    37,    38,   106,   107,    48,    49,
      41,    42
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -238
static const yytype_int16 yypact[] =
{
     750,    30,  -238,  1717,  1717,    47,  -238,  -238,    95,  -238,
      99,  -238,    50,  1529,   109,  -238,  -238,   150,  -238,    95,
      95,  1717,   549,  1717,  1717,    82,   134,   787,  -238,  -238,
      87,  -238,    20,    17,  -238,    48,  -238,  -238,  -238,  1905,
    1886,  -238,  1848,  -238,  1529,    17,  -238,     4,   170,   -23,
     647,  1529,   104,  1717,    51,  -238,    82,  1232,  -238,   549,
      95,   549,    95,    70,    70,   103,   116,  -238,    76,  -238,
     -40,    42,  -238,    31,  1586,    42,   170,   -23,  -238,  -238,
     895,    26,  -238,  -238,  -238,  -238,  -238,  -238,  1717,  1717,
    1717,     6,    95,  -238,  -238,  -238,  -238,  -238,  -238,  -238,
    -238,  -238,  -238,  -238,  -238,  1717,  1717,  -238,  1717,  -238,
     146,  -238,    95,    95,    95,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    95,    95,    95,    95,    95,    95,
      95,    95,    95,   957,  -238,  -238,  1269,  1717,  -238,    64,
     -20,   175,  -238,    92,   494,   112,   792,  -238,  -238,   549,
    -238,  1717,    69,  1717,   164,    42,   622,   549,   858,   183,
    -238,   209,    54,    42,  -238,  -238,   117,  1887,   161,  1764,
      42,  -238,  -238,   156,   328,  1175,  1175,  1175,  1175,   -45,
    1175,  1175,   900,  1274,   328,   328,   156,   156,   156,   156,
     -45,   -45,   -45,  1175,  1717,    22,  -238,   994,  -238,   698,
    1529,  1717,  1529,  -238,   180,  -238,   185,  -238,  1655,  1717,
      42,  1686,   155,   123,   135,  1806,  -238,  -238,  1566,  -238,
    -238,    95,  1529,  1529,  -238,  1529,  1331,   549,  1017,  1368,
     191,  1430,   549,   549,    65,  1617,  -238,    66,  1648,   214,
    1529,  1723,   858,  1175,   957,  1170,  1467,  -238,   -30,  1529,
    -238,    95,  -238,   140,   145,  -238,  -238,  -238,  -238,  -238,
     858,   160,  -238,   244,  -238,   247,  1071,  1113,  -238,  -238,
    -238,  1529,  1529,  -238,   858,  1108,  -238
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -238,  -238,  -238,   -37,   120,   227,   174,  -238,  -237,  -238,
    -238,  -238,  -238,    -1,    11,   397,  -238,     0,    -2,    16,
     114,   -49,   -85,  -142,  -238,   511,  -238,   218,   252,    62,
     -79,   310
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -127
static const yytype_int16 yytable[] =
{
      35,    47,    47,    51,    82,   262,   131,   133,    83,   208,
     143,   164,   145,    35,   136,   109,   149,   201,   109,    65,
      82,   150,    68,   270,    83,    12,   149,    35,   223,   171,
     224,   265,   -82,   -82,    43,   132,    14,   276,    15,    16,
      17,    18,   166,   158,    35,    88,    89,    67,   -32,    19,
      20,    35,   -32,   110,  -126,   138,   110,    35,  -126,    68,
      84,    68,    40,   159,    21,   165,    22,   235,    23,   238,
     -51,   -51,   198,   112,   113,    40,    84,    85,   199,   200,
      35,    52,   118,    24,    91,    25,    92,    77,   151,    40,
     162,    68,    77,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   168,   -32,   -32,    40,   145,    56,   152,
    -126,  -126,    14,    40,    12,    55,    17,    14,   140,    40,
      28,    80,   151,   151,   234,    14,   237,    15,    16,    17,
      18,   132,   -68,    35,    78,   -68,    35,   -68,    19,    20,
      53,   137,    40,   255,   257,    53,   209,    28,   149,    68,
     132,   203,    40,    53,    65,    22,    68,    68,    35,    24,
     226,    25,   147,   229,    24,   231,    25,    59,   149,    60,
      40,   205,    24,   149,    25,   148,   219,   142,   248,   239,
     172,   242,   240,   253,   254,   244,   245,   118,   246,   -82,
     -82,   -67,    47,   222,   148,    40,   149,    35,    40,   268,
      35,   149,    35,   260,   269,   129,   130,   131,    61,   202,
      62,   -64,   266,    61,   -64,    62,   239,   218,    35,   271,
      40,   134,    35,    35,   135,    35,    35,    68,    91,    35,
      92,    35,    68,    68,   274,   275,   132,    88,   232,    65,
      35,   211,    35,   233,    35,    35,    35,   251,   259,    35,
     223,   272,    39,   142,    79,   263,   142,   261,   111,    40,
      35,     0,    40,   207,    40,    39,    35,     0,     0,     0,
       0,    35,    35,     0,    35,    35,     0,    76,   142,    39,
      40,     0,    81,     0,    40,    40,     0,    40,    40,     0,
       0,    40,     0,    40,     0,     0,    39,     0,     0,     0,
       0,     0,    40,    39,    40,     0,    40,    40,    40,    39,
       0,    40,     0,    50,    50,     0,     0,     0,    54,     0,
       0,     0,    40,     0,     0,     0,     0,     0,    40,    63,
      64,     0,    39,    40,    40,     0,    40,    40,     0,     0,
       0,     0,    39,     0,     0,     0,   142,     0,     0,   142,
     112,   142,     0,     0,     0,     0,     0,     0,     0,   118,
      39,     0,   142,     0,   142,   142,   142,     0,     0,     0,
     144,     0,   146,   125,   126,   127,   128,   129,   130,   131,
     142,     0,     0,     0,     0,    39,   142,     0,    39,     0,
       0,     0,     0,     0,   142,   142,     0,     0,     0,     0,
      45,    45,   167,     0,     0,     0,     0,     0,   132,     0,
      39,     0,     0,     0,     0,   169,     0,     0,    45,    45,
      72,    45,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,     0,     0,     0,     0,     0,     0,    39,
      45,     0,    39,     0,    39,     0,    45,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,   215,     0,     0,
      39,    45,     0,     0,    39,    39,     0,    39,    39,     0,
       0,    39,     0,    39,     0,   160,   161,    45,    45,     0,
       0,     0,    39,     0,    39,     0,    39,    39,    39,     0,
       0,    39,    45,    45,    50,    45,     0,     0,     0,   228,
       0,     0,    39,     0,     0,     0,   112,   113,    39,     0,
       0,     0,     0,    39,    39,   118,    39,    39,     0,     0,
       0,   243,    66,    71,    45,    75,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,    45,     0,    45,     0,
      45,     0,     0,    45,    45,   204,     0,     0,     0,     0,
       0,   267,     0,     0,    66,     0,     0,     0,    12,     0,
      71,     0,    71,     0,   132,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,   155,     0,     0,     0,     0,
      67,    45,    19,    20,     0,     0,     0,     0,    45,     0,
       0,   163,    71,     0,     0,    45,    45,    21,    45,    22,
       0,    23,     0,     0,     0,     0,     0,   170,     0,   163,
       0,     0,     0,     0,    45,     0,    24,     0,    25,    45,
      45,     0,    45,     0,     0,    45,     0,     0,    45,     0,
       0,    12,     0,     0,     0,     0,     0,    82,   197,     0,
       0,    83,    14,     0,    15,    16,   212,    18,     0,     0,
      71,     0,    75,    67,   210,    19,    20,   214,    71,   112,
     113,   114,   115,   116,   117,   -80,   -80,     0,   118,     0,
      21,     0,    22,     0,    23,     0,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,    24,
       0,    25,     0,    84,     0,     0,     0,     0,     0,     0,
       0,     0,   230,     0,     0,     0,     0,    12,     0,   155,
      75,     0,    75,     0,     0,     0,     0,   132,    14,     0,
      15,    16,    17,    18,     0,     0,     0,     0,    71,     0,
       0,    19,    20,    71,    71,     0,   155,     0,     0,   155,
      -2,     1,    66,     0,     2,     3,    53,     0,   227,     4,
       5,     6,     7,     8,     0,     0,     9,    10,    11,    12,
      13,     0,     0,     0,     0,    24,     0,    25,     0,     0,
      14,     0,    15,    16,    17,    18,     0,    -3,     1,     0,
       0,     2,     3,    19,    20,     0,     4,     5,     6,     7,
       8,     0,     0,     9,    10,    11,    12,    13,    21,     0,
      22,     0,    23,     0,   112,   113,     0,    14,     0,    15,
      16,    17,    18,   118,     0,     0,     0,    24,     0,    25,
      19,    20,     0,     0,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,     0,    21,     0,    22,     0,    23,
       0,     0,     0,   206,     0,     0,     0,     0,   216,     1,
       0,     0,     2,     3,    24,     0,    25,     4,     5,     6,
       7,     8,   132,     0,   -18,   -18,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,     0,     0,     1,     0,     0,     2,
       3,    19,    20,     0,     4,     5,     6,     7,     8,     0,
       0,     0,     0,    11,    12,    13,    21,     0,    22,     0,
      23,     0,   112,   113,     0,    14,     0,    15,    16,    17,
      18,   118,   -99,   -99,     0,    24,     0,    25,    19,    20,
       0,   221,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,     0,   156,     0,   157,     0,    23,     1,     0,
       0,     2,     3,   -47,   194,   -47,     4,     5,     6,     7,
       8,   -99,    24,     0,    25,    11,    12,    13,     0,     0,
     132,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,     0,     0,     1,     0,     0,     2,     3,
      19,    20,     0,     4,     5,     6,     7,     8,     0,     0,
       0,     0,    11,    12,    13,    21,     0,    22,     0,    23,
       0,   249,   -51,   -51,    14,     0,    15,    16,    17,    18,
       0,     0,     0,     0,    24,     0,    25,    19,    20,   112,
     113,     0,     0,     0,     0,     0,     0,     0,   118,     0,
     225,     0,    21,     0,    22,     0,    23,     0,     0,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,     0,
       0,    24,     1,    25,     0,     2,     3,     0,     0,   -28,
       4,     5,     6,     7,     8,   -28,   -28,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,   132,     0,     0,
       0,    14,     0,    15,    16,    17,    18,     0,     0,     1,
       0,     0,     2,     3,    19,    20,   -29,     4,     5,     6,
       7,     8,   -29,   -29,     0,     0,    11,    12,    13,    21,
       0,    22,     0,    23,     0,   112,   113,     0,    14,     0,
      15,    16,    17,    18,   118,     0,     0,     0,    24,     0,
      25,    19,    20,     0,     0,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,    21,     0,    22,     0,
      23,     1,   273,     0,     2,     3,   -49,     0,   -49,     4,
       5,     6,     7,     8,     0,    24,     0,    25,    11,    12,
      13,     0,     0,   132,     0,     0,     0,   112,   113,     0,
      14,     0,    15,    16,    17,    18,   118,     0,     0,     0,
       0,     0,     0,    19,    20,     0,     0,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,     0,    21,     0,
      22,     0,    23,     1,     0,     0,     2,     3,     0,     0,
       0,     4,     5,     6,     7,     8,     0,    24,     0,    25,
      11,    12,    13,   141,     0,   132,     0,     0,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,     0,     0,
       1,     0,     0,     2,     3,    19,    20,   196,     4,     5,
       6,     7,     8,     0,     0,     0,     0,    11,    12,    13,
      21,     0,    22,     0,    23,     0,   112,   113,     0,    14,
       0,    15,    16,    17,    18,   118,     0,     0,     0,    24,
       0,    25,    19,    20,     0,     0,     0,   123,   124,   125,
     126,   127,   128,   129,   130,   131,     0,    21,     0,    22,
       0,    23,     1,     0,     0,     2,     3,     0,     0,   247,
       4,     5,     6,     7,     8,     0,    24,     0,    25,    11,
      12,    13,     0,     0,   132,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,     0,     0,     1,
       0,     0,     2,     3,    19,    20,   250,     4,     5,     6,
       7,     8,     0,     0,     0,     0,    11,    12,    13,    21,
       0,    22,     0,    23,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,     0,     0,     0,     0,    24,     0,
      25,    19,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,     0,    22,     0,
      23,     1,     0,     0,     2,     3,     0,     0,   252,     4,
       5,     6,     7,     8,     0,    24,     0,    25,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,     0,     0,     1,     0,
       0,     2,     3,    19,    20,   264,     4,     5,     6,     7,
       8,     0,     0,     0,     0,    11,    12,    13,    21,     0,
      22,     0,    23,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,     0,     0,     0,     0,    24,     0,    25,
      19,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,    22,     0,    23,
       1,     0,     0,     2,     3,     0,     0,     0,     4,     5,
       6,     7,     8,     0,    24,     0,    25,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,     0,     0,     1,     0,     0,
       2,     3,    19,    20,     0,     4,     5,     6,     7,     8,
       0,     0,     0,     0,    11,    12,    13,    21,     0,    22,
       0,    23,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,     0,     0,     0,    12,    24,     0,    25,    19,
      20,     0,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,     0,     0,   241,     0,    22,     0,    23,    19,
      20,     0,     0,     0,     0,     0,    12,     0,     0,     0,
       0,     0,   153,    24,    21,    25,    22,    14,    23,    15,
      16,    17,    18,     0,     0,     0,     0,     0,     0,     0,
      19,    20,     0,    24,   154,    25,     0,    12,     0,     0,
       0,     0,     0,   153,    12,    21,     0,    22,    14,    23,
      15,    16,    17,    18,     0,    14,     0,    15,    16,    17,
      18,    19,    20,     0,    24,   256,    25,     0,    19,    20,
       0,     0,     0,     0,   153,    12,    21,     0,    22,     0,
      23,   153,     0,    21,     0,    22,    14,    23,    15,    16,
      17,    18,     0,     0,     0,    24,   258,    25,     0,    19,
      20,     0,    24,     0,    25,     0,    12,     0,     0,     0,
       0,     0,    12,     0,    21,     0,    22,    14,    23,    15,
      16,    17,    18,    14,     0,    15,    16,   212,    18,     0,
      19,    20,     0,    24,   236,    25,    19,    20,     0,     0,
       0,     0,     0,     0,     0,    21,     0,    22,     0,    23,
       0,    21,     0,    22,     0,    23,   112,   113,   114,   115,
     116,   117,   -80,   -80,    24,   118,    25,     0,     0,     0,
      24,     0,    25,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   112,   113,
     114,   115,   116,   117,     0,     0,     0,   118,     0,     0,
       0,     0,     0,     0,   132,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   206,     0,     0,
     112,   113,   114,   115,   116,   117,     0,     0,     0,   118,
       0,     0,     0,     0,     0,     0,   132,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   112,
     113,     0,     0,     0,     0,     0,     0,     0,   118,     0,
       0,     0,     0,   108,   109,     0,     0,     0,   132,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,     0,
       0,     0,    90,     0,     0,     0,     0,     0,   220,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   110,    91,     0,    92,     0,   132,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105
};

static const yytype_int16 yycheck[] =
{
       0,     3,     4,     4,     0,   242,    51,    44,     4,   151,
      59,    90,    61,    13,    51,    38,    56,    37,    38,    21,
       0,    61,    22,   260,     4,    19,    56,    27,     6,   108,
       8,    61,    28,    29,     4,    80,    30,   274,    32,    33,
      34,    35,    91,    80,    44,    28,    29,    41,     0,    43,
      44,    51,     4,    76,     0,     4,    76,    57,     4,    59,
      56,    61,     0,    37,    58,    59,    60,   209,    62,   211,
      28,    29,     8,    22,    23,    13,    56,    57,    14,    15,
      80,    34,    31,    77,    58,    79,    60,    25,    57,    27,
      90,    91,    30,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   105,    56,    57,    44,   156,    58,    78,
      56,    57,    30,    51,    19,    16,    34,    30,    56,    57,
       0,    34,    57,    57,   209,    30,   211,    32,    33,    34,
      35,    80,    56,   133,     0,    59,   136,    61,    43,    44,
      58,    37,    80,    78,    78,    58,    77,    27,    56,   149,
      80,    59,    90,    58,   156,    60,   156,   157,   158,    77,
     197,    79,    59,   200,    77,   202,    79,    58,    56,    60,
     108,    59,    77,    56,    79,    59,    59,    57,   227,    56,
      34,   218,    59,   232,   233,   222,   223,    31,   225,    28,
      29,    56,   194,   194,    59,   133,    56,   197,   136,    59,
     200,    56,   202,   240,    59,    49,    50,    51,    58,    34,
      60,    56,   249,    58,    59,    60,    56,    34,   218,    59,
     158,    47,   222,   223,    50,   225,   226,   227,    58,   229,
      60,   231,   232,   233,   271,   272,    80,    28,    58,   241,
     240,    77,   242,    58,   244,   245,   246,    56,    34,   249,
       6,     4,     0,   133,    27,   244,   136,   241,    40,   197,
     260,    -1,   200,   149,   202,    13,   266,    -1,    -1,    -1,
      -1,   271,   272,    -1,   274,   275,    -1,    25,   158,    27,
     218,    -1,    30,    -1,   222,   223,    -1,   225,   226,    -1,
      -1,   229,    -1,   231,    -1,    -1,    44,    -1,    -1,    -1,
      -1,    -1,   240,    51,   242,    -1,   244,   245,   246,    57,
      -1,   249,    -1,     3,     4,    -1,    -1,    -1,     8,    -1,
      -1,    -1,   260,    -1,    -1,    -1,    -1,    -1,   266,    19,
      20,    -1,    80,   271,   272,    -1,   274,   275,    -1,    -1,
      -1,    -1,    90,    -1,    -1,    -1,   226,    -1,    -1,   229,
      22,   231,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
     108,    -1,   242,    -1,   244,   245,   246,    -1,    -1,    -1,
      60,    -1,    62,    45,    46,    47,    48,    49,    50,    51,
     260,    -1,    -1,    -1,    -1,   133,   266,    -1,   136,    -1,
      -1,    -1,    -1,    -1,   274,   275,    -1,    -1,    -1,    -1,
       3,     4,    92,    -1,    -1,    -1,    -1,    -1,    80,    -1,
     158,    -1,    -1,    -1,    -1,   105,    -1,    -1,    21,    22,
      23,    24,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   197,
      53,    -1,   200,    -1,   202,    -1,    59,    -1,    61,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,
     218,    74,    -1,    -1,   222,   223,    -1,   225,   226,    -1,
      -1,   229,    -1,   231,    -1,    88,    89,    90,    91,    -1,
      -1,    -1,   240,    -1,   242,    -1,   244,   245,   246,    -1,
      -1,   249,   105,   106,   194,   108,    -1,    -1,    -1,   199,
      -1,    -1,   260,    -1,    -1,    -1,    22,    23,   266,    -1,
      -1,    -1,    -1,   271,   272,    31,   274,   275,    -1,    -1,
      -1,   221,    21,    22,   137,    24,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   149,    -1,   151,    -1,
     153,    -1,    -1,   156,   157,    61,    -1,    -1,    -1,    -1,
      -1,   251,    -1,    -1,    53,    -1,    -1,    -1,    19,    -1,
      59,    -1,    61,    -1,    80,    -1,    -1,    -1,    -1,    30,
      -1,    32,    33,    34,    35,    74,    -1,    -1,    -1,    -1,
      41,   194,    43,    44,    -1,    -1,    -1,    -1,   201,    -1,
      -1,    90,    91,    -1,    -1,   208,   209,    58,   211,    60,
      -1,    62,    -1,    -1,    -1,    -1,    -1,   106,    -1,   108,
      -1,    -1,    -1,    -1,   227,    -1,    77,    -1,    79,   232,
     233,    -1,   235,    -1,    -1,   238,    -1,    -1,   241,    -1,
      -1,    19,    -1,    -1,    -1,    -1,    -1,     0,   137,    -1,
      -1,     4,    30,    -1,    32,    33,    34,    35,    -1,    -1,
     149,    -1,   151,    41,   153,    43,    44,   156,   157,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    31,    -1,
      58,    -1,    60,    -1,    62,    -1,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    77,
      -1,    79,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   201,    -1,    -1,    -1,    -1,    19,    -1,   208,
     209,    -1,   211,    -1,    -1,    -1,    -1,    80,    30,    -1,
      32,    33,    34,    35,    -1,    -1,    -1,    -1,   227,    -1,
      -1,    43,    44,   232,   233,    -1,   235,    -1,    -1,   238,
       0,     1,   241,    -1,     4,     5,    58,    -1,    60,     9,
      10,    11,    12,    13,    -1,    -1,    16,    17,    18,    19,
      20,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    -1,     0,     1,    -1,
      -1,     4,     5,    43,    44,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    16,    17,    18,    19,    20,    58,    -1,
      60,    -1,    62,    -1,    22,    23,    -1,    30,    -1,    32,
      33,    34,    35,    31,    -1,    -1,    -1,    77,    -1,    79,
      43,    44,    -1,    -1,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    58,    -1,    60,    -1,    62,
      -1,    -1,    -1,    61,    -1,    -1,    -1,    -1,     0,     1,
      -1,    -1,     4,     5,    77,    -1,    79,     9,    10,    11,
      12,    13,    80,    -1,    16,    17,    18,    19,    20,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    -1,    -1,     1,    -1,    -1,     4,
       5,    43,    44,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    18,    19,    20,    58,    -1,    60,    -1,
      62,    -1,    22,    23,    -1,    30,    -1,    32,    33,    34,
      35,    31,    37,    38,    -1,    77,    -1,    79,    43,    44,
      -1,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    58,    -1,    60,    -1,    62,     1,    -1,
      -1,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    76,    77,    -1,    79,    18,    19,    20,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    -1,    -1,     1,    -1,    -1,     4,     5,
      43,    44,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    18,    19,    20,    58,    -1,    60,    -1,    62,
      -1,     4,    28,    29,    30,    -1,    32,    33,    34,    35,
      -1,    -1,    -1,    -1,    77,    -1,    79,    43,    44,    22,
      23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      56,    -1,    58,    -1,    60,    -1,    62,    -1,    -1,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    77,     1,    79,    -1,     4,     5,    -1,    -1,     8,
       9,    10,    11,    12,    13,    14,    15,    -1,    -1,    18,
      19,    20,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,
      -1,    30,    -1,    32,    33,    34,    35,    -1,    -1,     1,
      -1,    -1,     4,     5,    43,    44,     8,     9,    10,    11,
      12,    13,    14,    15,    -1,    -1,    18,    19,    20,    58,
      -1,    60,    -1,    62,    -1,    22,    23,    -1,    30,    -1,
      32,    33,    34,    35,    31,    -1,    -1,    -1,    77,    -1,
      79,    43,    44,    -1,    -1,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    58,    -1,    60,    -1,
      62,     1,    59,    -1,     4,     5,     6,    -1,     8,     9,
      10,    11,    12,    13,    -1,    77,    -1,    79,    18,    19,
      20,    -1,    -1,    80,    -1,    -1,    -1,    22,    23,    -1,
      30,    -1,    32,    33,    34,    35,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    58,    -1,
      60,    -1,    62,     1,    -1,    -1,     4,     5,    -1,    -1,
      -1,     9,    10,    11,    12,    13,    -1,    77,    -1,    79,
      18,    19,    20,    21,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    32,    33,    34,    35,    -1,    -1,
       1,    -1,    -1,     4,     5,    43,    44,     8,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    18,    19,    20,
      58,    -1,    60,    -1,    62,    -1,    22,    23,    -1,    30,
      -1,    32,    33,    34,    35,    31,    -1,    -1,    -1,    77,
      -1,    79,    43,    44,    -1,    -1,    -1,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    58,    -1,    60,
      -1,    62,     1,    -1,    -1,     4,     5,    -1,    -1,     8,
       9,    10,    11,    12,    13,    -1,    77,    -1,    79,    18,
      19,    20,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    32,    33,    34,    35,    -1,    -1,     1,
      -1,    -1,     4,     5,    43,    44,     8,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    18,    19,    20,    58,
      -1,    60,    -1,    62,    -1,    -1,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    -1,    -1,    -1,    -1,    77,    -1,
      79,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    -1,    60,    -1,
      62,     1,    -1,    -1,     4,     5,    -1,    -1,     8,     9,
      10,    11,    12,    13,    -1,    77,    -1,    79,    18,    19,
      20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    32,    33,    34,    35,    -1,    -1,     1,    -1,
      -1,     4,     5,    43,    44,     8,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    18,    19,    20,    58,    -1,
      60,    -1,    62,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    -1,    -1,    -1,    -1,    77,    -1,    79,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    58,    -1,    60,    -1,    62,
       1,    -1,    -1,     4,     5,    -1,    -1,    -1,     9,    10,
      11,    12,    13,    -1,    77,    -1,    79,    18,    19,    20,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    32,    33,    34,    35,    -1,    -1,     1,    -1,    -1,
       4,     5,    43,    44,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    18,    19,    20,    58,    -1,    60,
      -1,    62,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    -1,    -1,    -1,    19,    77,    -1,    79,    43,
      44,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    -1,    -1,    58,    -1,    60,    -1,    62,    43,
      44,    -1,    -1,    -1,    -1,    -1,    19,    -1,    -1,    -1,
      -1,    -1,    56,    77,    58,    79,    60,    30,    62,    32,
      33,    34,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    77,    78,    79,    -1,    19,    -1,    -1,
      -1,    -1,    -1,    56,    19,    58,    -1,    60,    30,    62,
      32,    33,    34,    35,    -1,    30,    -1,    32,    33,    34,
      35,    43,    44,    -1,    77,    78,    79,    -1,    43,    44,
      -1,    -1,    -1,    -1,    56,    19,    58,    -1,    60,    -1,
      62,    56,    -1,    58,    -1,    60,    30,    62,    32,    33,
      34,    35,    -1,    -1,    -1,    77,    78,    79,    -1,    43,
      44,    -1,    77,    -1,    79,    -1,    19,    -1,    -1,    -1,
      -1,    -1,    19,    -1,    58,    -1,    60,    30,    62,    32,
      33,    34,    35,    30,    -1,    32,    33,    34,    35,    -1,
      43,    44,    -1,    77,    78,    79,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    58,    -1,    60,    -1,    62,
      -1,    58,    -1,    60,    -1,    62,    22,    23,    24,    25,
      26,    27,    28,    29,    77,    31,    79,    -1,    -1,    -1,
      77,    -1,    79,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,
      24,    25,    26,    27,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    61,    -1,    -1,
      22,    23,    24,    25,    26,    27,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    37,    38,    -1,    -1,    -1,    80,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    61,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    58,    -1,    60,    -1,    80,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     4,     5,     9,    10,    11,    12,    13,    16,
      17,    18,    19,    20,    30,    32,    33,    34,    35,    43,
      44,    58,    60,    62,    77,    79,    82,    83,    85,    86,
      90,    91,    93,    96,    97,    98,    99,   105,   106,   109,
     110,   111,   112,     4,    94,    96,    98,    99,   109,   110,
     112,    94,    34,    58,   112,    16,    58,    84,    85,    58,
      60,    58,    60,   112,   112,    99,   106,    41,    98,   101,
     102,   106,    96,   103,   104,   106,   109,   110,     0,    86,
      34,   109,     0,     4,    56,    57,    87,    88,    28,    29,
      37,    58,    60,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,   107,   108,    37,    38,
      76,   108,    22,    23,    24,    25,    26,    27,    31,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    80,    84,    87,    87,    84,    37,     4,    92,
     110,    21,    85,   102,   112,   102,   112,    59,    59,    56,
      61,    57,    78,    56,    78,   106,    58,    60,    84,    37,
      96,    96,    98,   106,   111,    59,   102,   112,    99,   112,
     106,   111,    34,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,     7,    95,     8,   106,     8,    14,
      15,    37,    34,    59,    61,    59,    61,   101,   104,    77,
     106,    77,    34,   100,   106,   112,     0,    89,    34,    59,
      61,    41,    94,     6,     8,    56,    84,    60,   112,    84,
     106,    84,    58,    58,   103,   104,    78,   103,   104,    56,
      59,    58,    84,   112,    84,    84,    84,     8,   102,     4,
       8,    56,     8,   102,   102,    78,    78,    78,    78,    34,
      84,   100,    89,    95,     8,    61,    84,   112,    59,    59,
      89,    59,     4,    59,    84,    84,    89
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
#line 110 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding string \"%s\"\n", (yyvaluep->str));
#endif
  free((yyvaluep->str));
};

/* Line 1000 of yacc.c  */
#line 1735 "psycon.tab.c"
	break;
      case 34: /* "\"identifier\"" */

/* Line 1000 of yacc.c  */
#line 110 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding string \"%s\"\n", (yyvaluep->str));
#endif
  free((yyvaluep->str));
};

/* Line 1000 of yacc.c  */
#line 1749 "psycon.tab.c"
	break;
      case 83: /* "block_func" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1763 "psycon.tab.c"
	break;
      case 84: /* "block" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1777 "psycon.tab.c"
	break;
      case 85: /* "line" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1791 "psycon.tab.c"
	break;
      case 86: /* "line_func" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1805 "psycon.tab.c"
	break;
      case 90: /* "func_decl" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1819 "psycon.tab.c"
	break;
      case 91: /* "funcdef" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1833 "psycon.tab.c"
	break;
      case 92: /* "case_list" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1847 "psycon.tab.c"
	break;
      case 93: /* "stmt" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1861 "psycon.tab.c"
	break;
      case 94: /* "conditional" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1875 "psycon.tab.c"
	break;
      case 95: /* "elseif_list" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1889 "psycon.tab.c"
	break;
      case 96: /* "expcondition" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1903 "psycon.tab.c"
	break;
      case 97: /* "csig" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1917 "psycon.tab.c"
	break;
      case 98: /* "initcell" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1931 "psycon.tab.c"
	break;
      case 99: /* "condition" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1945 "psycon.tab.c"
	break;
      case 100: /* "id_list" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1959 "psycon.tab.c"
	break;
      case 101: /* "arg" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1973 "psycon.tab.c"
	break;
      case 102: /* "arg_list" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1987 "psycon.tab.c"
	break;
      case 103: /* "matrix" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2001 "psycon.tab.c"
	break;
      case 104: /* "vector" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2015 "psycon.tab.c"
	break;
      case 105: /* "range" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2029 "psycon.tab.c"
	break;
      case 106: /* "exp_range" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2043 "psycon.tab.c"
	break;
      case 107: /* "compop" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2057 "psycon.tab.c"
	break;
      case 108: /* "assign2this" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2071 "psycon.tab.c"
	break;
      case 109: /* "varblock" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2085 "psycon.tab.c"
	break;
      case 110: /* "tid" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2099 "psycon.tab.c"
	break;
      case 111: /* "assign" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2113 "psycon.tab.c"
	break;
      case 112: /* "exp" */

/* Line 1000 of yacc.c  */
#line 103 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2127 "psycon.tab.c"
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
#line 94 "psycon.y"
{
  if (ErrorMsg) {
	free(ErrorMsg);
	ErrorMsg = NULL;
  }
  *errmsg = NULL;
}

/* Line 1242 of yacc.c  */
#line 2288 "psycon.tab.c"

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
#line 143 "psycon.y"
    { *pproot = NULL;;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 145 "psycon.y"
    { *pproot = (yyvsp[(1) - (1)].pnode);;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 150 "psycon.y"
    {
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
#line 171 "psycon.y"
    {
		if ((yyvsp[(1) - (1)].pnode)) // if cond1, x=1, end ==> x=1 comes here.
			(yyval.pnode) = (yyvsp[(1) - (1)].pnode);
		else
			(yyval.pnode) = newAstNode(N_BLOCK, (yyloc));
	;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 178 "psycon.y"
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
#line 200 "psycon.y"
    { (yyval.pnode) = NULL;;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 202 "psycon.y"
    {
		(yyval.pnode) = NULL;
		yyerrok;
	;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 208 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
		(yyval.pnode)->suppress=1;
	;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 229 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_FUNCTION, (yyloc));
		(yyval.pnode)->suppress = 2;
	;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 234 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_FUNCTION, (yyloc));
		(yyval.pnode)->suppress = 3;
	;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 241 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (4)].pnode);
		(yyval.pnode)->str = (yyvsp[(2) - (4)].str);
		(yyval.pnode)->child = newAstNode(N_IDLIST, (yyloc));
		(yyval.pnode)->child->next = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 248 "psycon.y"
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

  case 24:

/* Line 1455 of yacc.c  */
#line 266 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (7)].pnode);
		(yyval.pnode)->str = (yyvsp[(2) - (7)].str);
		(yyval.pnode)->child = (yyvsp[(4) - (7)].pnode);
		(yyvsp[(4) - (7)].pnode)->next = (yyvsp[(6) - (7)].pnode);
	;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 273 "psycon.y"
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

  case 26:

/* Line 1455 of yacc.c  */
#line 293 "psycon.y"
    { (yyval.pnode) = newAstNode(T_SWITCH, (yyloc));;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 295 "psycon.y"
    { (yyval.pnode) = newAstNode(T_SWITCH, (yyloc));;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 297 "psycon.y"
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

  case 29:

/* Line 1455 of yacc.c  */
#line 312 "psycon.y"
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

  case 33:

/* Line 1455 of yacc.c  */
#line 332 "psycon.y"
    { // This works, too, for "if cond, act; end" without else, because elseif_list can be empty
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

  case 34:

/* Line 1455 of yacc.c  */
#line 358 "psycon.y"
    { // case is cascaded through alt
		(yyval.pnode) = (yyvsp[(3) - (4)].pnode);
		(yyval.pnode)->alt = (yyvsp[(3) - (4)].pnode)->alt;
		(yyval.pnode)->child = (yyvsp[(2) - (4)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 366 "psycon.y"
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

  case 36:

/* Line 1455 of yacc.c  */
#line 377 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_TRY, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->alt = newAstNode(T_CATCH, (yylsp[(4) - (6)]));
		(yyval.pnode)->alt->child = newAstNode(T_ID, (yylsp[(4) - (6)]));
		(yyval.pnode)->alt->child->str = (yyvsp[(4) - (6)].str);
		(yyval.pnode)->alt->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 386 "psycon.y"
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

  case 38:

/* Line 1455 of yacc.c  */
#line 398 "psycon.y"
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

  case 39:

/* Line 1455 of yacc.c  */
#line 412 "psycon.y"
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

  case 40:

/* Line 1455 of yacc.c  */
#line 426 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_RETURN, (yyloc));
	;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 430 "psycon.y"
    { (yyval.pnode) = newAstNode(T_BREAK, (yyloc));;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 432 "psycon.y"
    { (yyval.pnode) = newAstNode(T_CONTINUE, (yyloc));;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 440 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_IF, (yyloc));
	;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 444 "psycon.y"
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
	;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 457 "psycon.y"
    {
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

  case 52:

/* Line 1455 of yacc.c  */
#line 484 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
		(yyval.pnode)->type = N_INITCELL;
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 493 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 499 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('<', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 501 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('>', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 503 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_EQ, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 505 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_NE, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 507 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_GE, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 509 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_LE, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 511 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_LOGIC_NOT, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 516 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_AND, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 518 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_OR, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 522 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_IDLIST, (yyloc));
	;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 526 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_IDLIST, (yyloc));
		(yyval.pnode)->child = (yyval.pnode)->tail = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->tail->str = (yyvsp[(1) - (1)].str);
	;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 532 "psycon.y"
    {
		(yyvsp[(1) - (3)].pnode)->tail = (yyvsp[(1) - (3)].pnode)->tail->next = newAstNode(T_ID, (yylsp[(3) - (3)]));
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		(yyval.pnode)->tail->str = (yyvsp[(3) - (3)].str);
	;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 540 "psycon.y"
    {	(yyval.pnode) = newAstNode(T_FULLRANGE, (yyloc)); ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 546 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_ARGS, (yyloc));
		(yyval.pnode)->tail = (yyval.pnode)->child = (yyvsp[(1) - (1)].pnode);
	;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 551 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		if ((yyval.pnode)->tail)
			(yyval.pnode)->tail = (yyval.pnode)->tail->next = (yyvsp[(3) - (3)].pnode);
		else
			(yyval.pnode)->tail = (yyval.pnode)->next = (yyvsp[(3) - (3)].pnode);
	;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 561 "psycon.y"
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

  case 72:

/* Line 1455 of yacc.c  */
#line 572 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_MATRIX, (yyloc));
		AstNode * p = newAstNode(N_VECTOR, (yyloc));
		p->alt = p->tail = (yyvsp[(1) - (1)].pnode);
		(yyval.pnode)->str = (char*)p;
	;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 579 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		AstNode * p = (AstNode *)(yyvsp[(1) - (3)].pnode)->str;
		p->tail = p->tail->next = (AstNode *)(yyvsp[(3) - (3)].pnode);
	;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 588 "psycon.y"
    {
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

  case 76:

/* Line 1455 of yacc.c  */
#line 600 "psycon.y"
    {
		AstNode * p = (AstNode *)(yyvsp[(1) - (2)].pnode)->str;
		p->tail = p->tail->next = (yyvsp[(2) - (2)].pnode);
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
	;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 606 "psycon.y"
    {
		AstNode * p = (AstNode *)(yyvsp[(1) - (3)].pnode)->str;
		p->tail = p->tail->next = (yyvsp[(3) - (3)].pnode);
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
	;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 614 "psycon.y"
    {
		(yyval.pnode) = makeFunctionCall(":", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));
	;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 618 "psycon.y"
    {
		(yyval.pnode) = makeFunctionCall(":", (yyvsp[(1) - (5)].pnode), (yyvsp[(5) - (5)].pnode), (yyloc));
		(yyvsp[(5) - (5)].pnode)->next = (yyvsp[(3) - (5)].pnode);
	;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 628 "psycon.y"
    {
		(yyval.pnode) = newAstNode('+', (yyloc));
	;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 632 "psycon.y"
    { 		(yyval.pnode) = newAstNode('-', (yyloc));	;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 634 "psycon.y"
    { 		(yyval.pnode) = newAstNode('*', (yyloc));	;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 636 "psycon.y"
    { 		(yyval.pnode) = newAstNode('/', (yyloc));	;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 638 "psycon.y"
    { 		(yyval.pnode) = newAstNode('@', (yyloc));	;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 640 "psycon.y"
    {
		(yyval.pnode) = newAstNode('@', (yyloc));
		(yyval.pnode)->child = newAstNode('@', (yyloc));
	;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 645 "psycon.y"
    { 		(yyval.pnode) = newAstNode(T_OP_SHIFT, (yyloc));	;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 647 "psycon.y"
    { 		(yyval.pnode) = newAstNode('%', (yyloc));	;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 649 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (char*)calloc(16, 1);
		strcpy((yyval.pnode)->str, "movespec");
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_ARGS, (yyloc));
	;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 656 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (char*)calloc(16, 1);
		strcpy((yyval.pnode)->str, "respeed");
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_ARGS, (yyloc));
	;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 663 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (char*)calloc(16, 1);
		strcpy((yyval.pnode)->str, "timestretch");
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_ARGS, (yyloc));
	;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 670 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (char*)calloc(16, 1);
		strcpy((yyval.pnode)->str, "pitchscale");
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_ARGS, (yyloc));
	;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 679 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 683 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_OP_CONCAT, (yyloc));
		AstNode *p = (yyval.pnode);
		if (p->alt)
			p = p->alt;
		p->child = 	newAstNode(T_REPLICA, (yylsp[(2) - (2)]));
		p->tail = p->child->next = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 692 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_OP_CONCAT, (yyloc));
		AstNode *p = (yyval.pnode);
		if (p->alt)
			p = p->alt;
		p->child = 	newAstNode(T_REPLICA, (yylsp[(2) - (2)]));
		p->tail = p->child->next = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 701 "psycon.y"
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

  case 99:

/* Line 1455 of yacc.c  */
#line 721 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (1)].str);
	;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 726 "psycon.y"
    {
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

  case 101:

/* Line 1455 of yacc.c  */
#line 745 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (4)].pnode);
		(yyval.pnode)->tail = (yyval.pnode)->alt->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->tail->child = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 751 "psycon.y"
    {//tid-vector --> what's this comment? 12/30/2020
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
	;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 755 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_HOOK, (yyloc));
		(yyval.pnode)->str = (char*)calloc(1, strlen((yyvsp[(2) - (2)].pnode)->str)+1);
		strcpy((yyval.pnode)->str, (yyvsp[(2) - (2)].pnode)->str);
		(yyval.pnode)->alt = (yyvsp[(2) - (2)].pnode)->alt;
	;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 765 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (4)].str);
		handle_tilde((yyval.pnode), (yyvsp[(3) - (4)].pnode), (yylsp[(3) - (4)]));
	;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 771 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (4)].str);
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->alt->child = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 778 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (7)].str);
		(yyval.pnode)->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->alt->child = (yyvsp[(3) - (7)].pnode);
		handle_tilde((yyval.pnode)->alt, (yyvsp[(6) - (7)].pnode), (yylsp[(6) - (7)]));
		(yyval.pnode)->tail = (yyval.pnode)->alt->alt; // we need this; or tail is broken and can't put '.' tid at the end
	;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 787 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (4)].pnode);
		handle_tilde((yyval.pnode), (yyvsp[(3) - (4)].pnode), (yylsp[(3) - (4)]));
	;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 792 "psycon.y"
    {
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

  case 110:

/* Line 1455 of yacc.c  */
#line 804 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
	;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 808 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
		handle_tilde((yyval.pnode), (yyvsp[(3) - (4)].pnode), (yylsp[(3) - (4)]));
	;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 813 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->alt->child = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 819 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
		(yyval.pnode)->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->alt->child = (yyvsp[(3) - (7)].pnode);
		handle_tilde((yyval.pnode)->alt, (yyvsp[(6) - (7)].pnode), (yylsp[(6) - (7)]));
		(yyval.pnode)->tail = (yyval.pnode)->alt->alt; // we need this; or tail is broken and can't put '.' tid at the end
	;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 827 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_TRANSPOSE, (yyloc));
 		(yyval.pnode)->child = (yyvsp[(1) - (2)].pnode);
	;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 832 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (5)].pnode);
	;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 837 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 843 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 849 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->str = (char*)malloc(8);
		strcpy((yyval.pnode)->str, "R");
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 857 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->str = (char*)malloc(8);
		strcpy((yyval.pnode)->str, "R");
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 865 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
	;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 869 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 878 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
		(yyval.pnode)->child = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 883 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
		(yyval.pnode)->child = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 888 "psycon.y"
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

  case 125:

/* Line 1455 of yacc.c  */
#line 903 "psycon.y"
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

  case 126:

/* Line 1455 of yacc.c  */
#line 918 "psycon.y"
    { // x={"bjk",noise(300), 4.5555}
		(yyval.pnode)->str = getT_ID_str((yyvsp[(1) - (3)].pnode));
		(yyval.pnode)->child = (yyvsp[(3) - (3)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 929 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_NUMBER, (yyloc));
		(yyval.pnode)->dval = (yyvsp[(1) - (1)].dval);
	;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 934 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_STRING, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (1)].str);
	;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 939 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ENDPOINT, (yyloc));
	;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 943 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_NEGATIVE, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 948 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (2)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 954 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_SIGMA, (yyloc));
		(yyval.pnode)->child = newAstNode(T_ID, (yylsp[(3) - (8)]));
		(yyval.pnode)->child->str = getT_ID_str((yyvsp[(3) - (8)].pnode));
		(yyval.pnode)->child->child = (yyvsp[(5) - (8)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(7) - (8)].pnode);
	;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 962 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('+', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 964 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('-', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 966 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('*', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 968 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('/', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 970 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_MATRIXMULT, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 972 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("^", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 974 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("mod", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 976 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("respeed", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 978 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("pitchscale", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 980 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("timestretch", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 982 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("movespec", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 984 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('@', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 986 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_OP_SHIFT, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 988 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_OP_CONCAT, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;



/* Line 1455 of yacc.c  */
#line 3834 "psycon.tab.c"
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
#line 1006 "psycon.y"


/* Called by yyparse on error. */
void yyerror (AstNode **pproot, char **errmsg, char const *s)
{
  static size_t errmsg_len = 0;
#define ERRMSG_MAX 999
  char msgbuf[ERRMSG_MAX], *p;
  size_t msglen;

  sprintf(msgbuf, "Invalid syntax: Line %d, Col %d: %s.\n", yylloc.first_line, yylloc.first_column, s + (strncmp(s, "syntax error, ", 14) ? 0 : 14));
  if ((p=strstr(msgbuf, "$undefined"))) {
	sprintf(p, "'%c'(%d)", yychar, yychar);
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
  strcpy(ErrorMsg+errmsg_len, msgbuf);
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
    sprintf(buf, "[%s=]", p->str);
    break;
  case T_ID:
    sprintf(buf, "[%s]", p->str);
    break;
  case T_STRING:
    sprintf(buf, "\"%s\"", p->str);
    break;
  case N_CALL:
    sprintf(buf, "%s()", p->str);
    break;
  case N_CELL:
    sprintf(buf, "%s()", p->str);
    break;
  case T_NUMBER:
    sprintf(buf, "%.1f", p->dval);
    break;
  case N_BLOCK:
    sprintf(buf, "BLOCK");
    break;
  case N_ARGS:
    sprintf(buf, "ARGS");
    break;
  case N_MATRIX:
    sprintf(buf, "MATRIX");
    break;
  case N_VECTOR:
    sprintf(buf, "VECTOR");
    break;
  case N_IDLIST:
    sprintf(buf, "ID_LIST");
    break;
  case N_TIME_EXTRACT:
    sprintf(buf, "TIME_EXTRACT");
    break;
  case N_CELLASSIGN:
    sprintf(buf, "INITCELL");
    break;
  case N_IXASSIGN:
    sprintf(buf, "ASSIGN1");
    break;
  default:
    if (YYTRANSLATE(p->type) == 2)
      sprintf(buf, "[%d]", p->type);
    else
      sprintf(buf, "%s", yytname[YYTRANSLATE(p->type)]);
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

AstNode *makeFunctionCall(const char *name, AstNode *first, AstNode *second, YYLTYPE loc)
{
	AstNode *node;

	node = newAstNode(T_ID, loc);
	node->str = (char*)calloc(1, strlen(name)+1);
	strcpy(node->str, name);
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

void handle_tilde(AstNode *proot, AstNode *pp, YYLTYPE loc)
{
	AstNode *p = pp->child;
	if (p->type==T_ID && !strcmp(p->str,"respeed"))
	{ // x{2}(t1~t2) checks here because t1~t2 can be arg_list through
        AstNode *q = newAstNode(N_TIME_EXTRACT, loc);
		q->child = p->alt->child;
		q->child->next = p->alt->child->next;
        if (proot->tail)
            proot->tail = proot->tail->alt = q;
		else
			proot->tail = proot->alt = q;
		p->alt->child = NULL;
		yydeleteAstNode(p, 1);
	}
	else
	{
	    if (proot->tail)
			proot->tail = proot->tail->alt = pp;
		else
			proot->tail = proot->alt = pp;
	}
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
