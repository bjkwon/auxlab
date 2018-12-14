
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
#line 2 "psycon.y"

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
     T_OP_SHIFT = 275,
     T_OP_CONCAT = 276,
     T_LOGIC_EQ = 277,
     T_LOGIC_NE = 278,
     T_LOGIC_LE = 279,
     T_LOGIC_GE = 280,
     T_LOGIC_AND = 281,
     T_LOGIC_OR = 282,
     T_REPLICA = 283,
     T_MATRIXMULT = 284,
     T_NUMBER = 285,
     T_STRING = 286,
     T_ID = 287,
     T_ENDPOINT = 288,
     T_FULLRANGE = 289,
     T_NEGATIVE = 291,
     T_POSITIVE = 292,
     T_LOGIC_NOT = 293,
     T_TRANSPOSE = 294
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 22 "psycon.y"

	double dval;
	char *str;
	AstNode *pnode;



/* Line 214 of yacc.c  */
#line 171 "psycon.tab.c"
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
#line 103 "psycon.y"

AstNode *newAstNode(int type, YYLTYPE loc);
AstNode *makeFunctionCall(char *name, AstNode *first, AstNode *second, YYLTYPE loc);
AstNode *makeBinaryOpNode(int op, AstNode *first, AstNode *second, YYLTYPE loc);
void print_token_value(FILE *file, int type, YYSTYPE value);
char *getT_ID_str(AstNode *p);


/* Line 264 of yacc.c  */
#line 205 "psycon.tab.c"

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
#define YYFINAL  74
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2171

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  74
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  32
/* YYNRULES -- Number of rules.  */
#define YYNRULES  148
/* YYNRULES -- Number of states.  */
#define YYNSTATES  280

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   304

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    57,     2,     2,    73,    43,     2,    36,
      55,    56,    46,    41,    53,    40,    68,    47,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    39,    54,
      37,    35,    38,     2,    44,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    71,     2,    72,    48,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    69,     2,    70,    45,     2,     2,     2,
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
      42,    49,    50,    51,    52,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    11,    13,    16,    18,
      21,    24,    27,    29,    31,    33,    35,    37,    39,    40,
      42,    44,    47,    49,    52,    53,    58,    62,    64,    66,
      68,    70,    72,    78,    83,    90,    95,   102,   110,   112,
     114,   116,   120,   124,   128,   132,   136,   140,   144,   147,
     151,   155,   156,   158,   162,   164,   166,   168,   170,   174,
     176,   177,   179,   183,   186,   188,   191,   195,   199,   205,
     207,   209,   211,   213,   215,   217,   219,   221,   223,   225,
     227,   229,   232,   235,   238,   240,   244,   248,   253,   260,
     264,   266,   271,   276,   284,   291,   296,   300,   302,   307,
     312,   320,   327,   330,   336,   343,   350,   357,   364,   368,
     371,   375,   378,   381,   385,   389,   393,   395,   397,   399,
     401,   403,   410,   413,   416,   425,   429,   433,   437,   441,
     445,   449,   453,   457,   461,   465,   469,   473,   475,   478,
     483,   490,   498,   508,   509,   511,   517,   524,   526
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      75,     0,    -1,    -1,    76,    -1,    79,    -1,    76,    79,
      -1,    78,    -1,    77,    78,    -1,     4,    -1,     1,     4,
      -1,    86,    80,    -1,    86,    81,    -1,    78,    -1,   102,
      -1,    53,    -1,     4,    -1,     0,    -1,    54,    -1,    -1,
       0,    -1,    87,    -1,    87,    80,    -1,   100,    -1,   100,
      80,    -1,    -1,     7,    83,    77,    84,    -1,    84,     6,
      77,    -1,   104,    -1,    87,    -1,    85,    -1,    99,    -1,
     105,    -1,     5,    83,    77,    84,     8,    -1,    13,   100,
     103,     8,    -1,    13,   100,   103,    15,    77,     8,    -1,
       9,    83,    77,     8,    -1,    10,    32,    35,    94,    77,
       8,    -1,    10,    32,    35,    94,    53,    77,     8,    -1,
      18,    -1,    11,    -1,    12,    -1,   100,    37,   100,    -1,
     100,    38,   100,    -1,   100,    22,   100,    -1,   100,    23,
     100,    -1,   100,    25,   100,    -1,   100,    24,   100,    -1,
      55,    87,    56,    -1,    57,    85,    -1,    85,    26,    85,
      -1,    85,    27,    85,    -1,    -1,    32,    -1,    88,    53,
      32,    -1,    39,    -1,    94,    -1,   105,    -1,    89,    -1,
      90,    53,    89,    -1,    87,    -1,    -1,    92,    -1,    91,
      54,    92,    -1,    91,    54,    -1,    94,    -1,    92,    94,
      -1,    92,    53,    94,    -1,   100,    39,   100,    -1,   100,
      39,   100,    39,   100,    -1,   100,    -1,    93,    -1,    58,
      -1,    59,    -1,    60,    -1,    61,    -1,    62,    -1,    63,
      -1,    64,    -1,    65,    -1,    66,    -1,    67,    -1,    35,
      94,    -1,    35,    87,    -1,    95,    94,    -1,    32,    -1,
      98,    68,    32,    -1,    87,    68,    32,    -1,    97,    69,
     100,    70,    -1,    97,    55,   100,    45,   100,    56,    -1,
      71,    92,    72,    -1,    97,    -1,    32,    55,    90,    56,
      -1,    32,    69,   100,    70,    -1,    32,    69,   100,    70,
      55,    90,    56,    -1,    32,    55,   100,    45,   100,    56,
      -1,    97,    55,    90,    56,    -1,    97,    55,    56,    -1,
      28,    -1,    28,    55,    90,    56,    -1,    28,    69,   100,
      70,    -1,    28,    69,   100,    70,    55,    90,    56,    -1,
      28,    55,   100,    45,   100,    56,    -1,    98,    36,    -1,
      71,    92,    72,    71,    72,    -1,    71,    92,    72,    71,
      92,    72,    -1,    71,    92,    72,    71,    91,    72,    -1,
      71,    91,    72,    71,    92,    72,    -1,    71,    91,    72,
      71,    91,    72,    -1,    71,    91,    72,    -1,    73,    98,
      -1,    55,    94,    56,    -1,    98,    96,    -1,    97,    96,
      -1,    97,    35,    99,    -1,    98,    35,    99,    -1,    97,
      35,   105,    -1,    30,    -1,    31,    -1,    33,    -1,   105,
      -1,    98,    -1,    98,    55,   100,    45,   100,    56,    -1,
      40,   100,    -1,    41,   100,    -1,    19,    55,    98,    35,
      94,    53,   100,    56,    -1,   100,    41,   100,    -1,   100,
      40,   100,    -1,   100,    46,   100,    -1,   100,    47,   100,
      -1,   100,    29,   100,    -1,   100,    48,   100,    -1,   100,
      43,   100,    -1,   100,    45,   100,    -1,   100,    42,   100,
      -1,   100,    44,   100,    -1,   100,    20,   100,    -1,   100,
      21,   100,    -1,    16,    -1,    17,    16,    -1,   101,    32,
      77,    82,    -1,   101,    97,    35,    32,    77,    82,    -1,
     101,    32,    55,    88,    56,    77,    82,    -1,   101,    97,
      35,    32,    55,    88,    56,    77,    82,    -1,    -1,     4,
      -1,   103,    14,   100,     4,    77,    -1,   103,    14,    69,
      90,    70,    77,    -1,    94,    -1,    69,    90,    70,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   123,   123,   124,   128,   130,   156,   163,   189,   191,
     196,   198,   202,   204,   208,   208,   208,   211,   214,   214,
     217,   217,   217,   217,   221,   224,   237,   258,   259,   262,
     263,   264,   265,   291,   299,   308,   320,   334,   348,   352,
     354,   360,   362,   364,   366,   368,   370,   372,   378,   383,
     385,   390,   393,   399,   407,   409,   410,   413,   418,   426,
     430,   434,   439,   444,   447,   452,   457,   464,   468,   475,
     475,   477,   481,   483,   485,   487,   489,   494,   496,   498,
     500,   508,   512,   516,   532,   537,   560,   572,   578,   585,
     591,   592,   598,   605,   613,   621,   629,   641,   645,   650,
     657,   664,   671,   682,   687,   693,   699,   707,   715,   719,
     724,   733,   738,   749,   764,   779,   795,   800,   805,   809,
     810,   814,   830,   835,   841,   849,   851,   853,   855,   857,
     859,   861,   863,   865,   867,   869,   871,   875,   880,   887,
     894,   911,   918,   938,   939,   941,   951,   963,   966
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
  "\"sigma\"", "\">>\"", "\"++\"", "\"==\"", "\"!=\"", "\"<=\"", "\">=\"",
  "\"&&\"", "\"||\"", "\"..\"", "\"**\"", "\"number\"", "\"string\"",
  "\"identifier\"", "T_ENDPOINT", "T_FULLRANGE", "'='", "'\\''", "'<'",
  "'>'", "':'", "'-'", "'+'", "\"->\"", "'%'", "'@'", "'~'", "'*'", "'/'",
  "'^'", "T_NEGATIVE", "T_POSITIVE", "T_LOGIC_NOT", "T_TRANSPOSE", "','",
  "';'", "'('", "')'", "'!'", "\"+=\"", "\"-=\"", "\"*=\"", "\"/=\"",
  "\"@=\"", "\"@@=\"", "\"++=\"", "\">>=\"", "\"%=\"", "\"->=\"", "'.'",
  "'{'", "'}'", "'['", "']'", "'$'", "$accept", "input", "block_func",
  "block", "line", "line_func", "eol", "eol2", "func_end", "conditional",
  "elseif_list", "expcondition", "stmt", "condition", "id_list", "arg",
  "arg_list", "matrix", "vector", "range", "exp_range", "compop",
  "assign2this", "varblock", "tid", "assign", "exp", "func_static_or_not",
  "funcdef", "case_list", "csig", "initcell", 0
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
     285,   286,   287,   288,   289,    61,    39,    60,    62,    58,
      45,    43,   290,    37,    64,   126,    42,    47,    94,   291,
     292,   293,   294,    44,    59,    40,    41,    33,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,    46,   123,
     125,    91,    93,    36
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    74,    75,    75,    76,    76,    77,    77,    78,    78,
      78,    78,    79,    79,    80,    80,    80,    81,    82,    82,
      83,    83,    83,    83,    84,    84,    84,    85,    85,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    88,    88,    88,    89,    89,    89,    90,    90,    90,
      91,    91,    91,    91,    92,    92,    92,    93,    93,    94,
      94,    95,    95,    95,    95,    95,    95,    95,    95,    95,
      95,    96,    96,    96,    97,    97,    97,    97,    97,    97,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    99,    99,    99,    99,    99,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   101,   101,   102,
     102,   102,   102,   103,   103,   103,   103,   104,   105
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     1,     2,     1,     2,
       2,     2,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     2,     1,     2,     0,     4,     3,     1,     1,     1,
       1,     1,     5,     4,     6,     4,     6,     7,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       3,     0,     1,     3,     1,     1,     1,     1,     3,     1,
       0,     1,     3,     2,     1,     2,     3,     3,     5,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     2,     1,     3,     3,     4,     6,     3,
       1,     4,     4,     7,     6,     4,     3,     1,     4,     4,
       7,     6,     2,     5,     6,     6,     6,     6,     3,     2,
       3,     2,     2,     3,     3,     3,     1,     1,     1,     1,
       1,     6,     2,     2,     8,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     2,     4,
       6,     7,     9,     0,     1,     5,     6,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     8,     0,     0,     0,    39,    40,     0,   137,
       0,    38,     0,    97,   116,   117,    84,   118,     0,     0,
       0,     0,     0,    60,     0,     0,     0,    12,     4,    29,
       0,    28,    70,   147,    90,   120,    30,    69,     0,    13,
      27,   119,     9,     0,     0,    20,    90,   120,    22,   119,
       0,     0,   143,   138,     0,     0,     0,     0,     0,   122,
     123,    28,   147,    48,    54,    59,    57,     0,    55,   119,
       0,    61,    64,   109,     1,     5,     0,     0,    16,    15,
      14,    17,    10,    11,     0,     0,     0,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,     0,     0,   112,
       0,   102,     0,     0,   111,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    90,     0,     6,    21,    23,
       0,     0,   144,     0,   120,     0,    69,    69,     0,    69,
      69,    47,   110,     0,   148,    63,   108,     0,    89,    65,
      49,    50,    86,    82,    81,   113,   119,    96,     0,    69,
      69,    83,   114,    69,    85,   135,   136,    43,    44,    46,
      45,   129,    41,    42,    67,   126,   125,   133,   131,   134,
     132,   127,   128,   130,    51,     0,     0,     0,     0,     7,
       0,    35,     0,    33,     0,     0,     0,    98,     0,    99,
      91,     0,    92,    58,    62,    60,    66,    60,    95,     0,
      87,     0,     0,    84,    28,     0,   147,    19,   139,     0,
       0,     0,    32,     0,     0,     0,    69,     0,   147,   132,
       0,   132,     0,     0,    61,   103,     0,    61,   132,   132,
      68,     0,     0,    51,     0,     0,     0,     0,    36,     0,
       0,    34,     0,   101,     0,    94,     0,   107,   106,   105,
     104,    88,   121,    53,     0,     0,   140,    25,    37,     0,
       0,    69,   100,    93,   141,     0,     0,   124,     0,   142
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    25,    26,   126,   127,    28,    82,    83,   218,    43,
     190,    44,    30,    31,   215,    66,    67,    70,    71,    32,
      33,    98,    99,    46,    47,    36,    37,    38,    39,   133,
      40,    49
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -230
static const yytype_int16 yypact[] =
{
     625,    52,  -230,  1576,  1576,     6,  -230,  -230,  1576,  -230,
      82,  -230,    48,    27,  -230,  -230,    37,  -230,  1576,  1576,
    1576,  1576,  1504,  1576,  1576,    78,   702,  -230,  -230,   147,
       8,    55,  -230,  -230,    99,  1801,  -230,  1978,  1610,  -230,
    -230,    14,  -230,  1347,   147,     1,    40,   -23,  1701,  -230,
    1347,    84,  1854,  -230,  1576,  1504,  1576,  1504,  1576,   149,
     149,     4,    71,  -230,  -230,   -20,  -230,   -36,   154,    24,
     -31,    80,   154,   -23,  -230,  -230,  1576,  1576,  -230,  -230,
    -230,  -230,  -230,  -230,   104,  1576,  1402,  -230,  -230,  -230,
    -230,  -230,  -230,  -230,  -230,  -230,  -230,  1576,  1576,  -230,
    1576,  -230,  1576,   137,  -230,  1576,  1576,  1576,  1576,  1576,
    1576,  1576,  1576,  1576,  1576,  1576,  1576,  1576,  1576,  1576,
    1576,  1576,  1576,  1576,   807,   -25,   487,  -230,  -230,  -230,
    1137,  1576,  -230,   124,   -16,    35,  2007,  1735,    51,  2036,
    1769,  -230,  -230,  1504,  -230,  1576,   100,  1576,   107,   154,
    -230,   156,  -230,   -20,   154,  -230,    21,  -230,    61,  2065,
    1803,   154,  -230,  2094,  -230,   170,   641,  1978,  1978,  1978,
    1978,    10,  1978,  1978,  2123,   641,   641,   170,   170,   170,
     170,    10,    10,    10,  1523,  1504,   736,   151,  1576,  -230,
      45,  -230,   927,  -230,  1629,  1347,  1576,  -230,  1576,   135,
    -230,  1576,   136,  -230,  1557,  1576,   154,   576,  -230,  1576,
    -230,  1576,  1576,    28,    34,    62,    72,  -230,  -230,  1396,
    1347,  1347,  -230,  1347,  1208,  1504,  1883,  1242,   141,   290,
    1504,   343,  1504,    -8,  1449,  -230,    -7,  1455,   358,   734,
    1949,   166,  1347,  1663,   736,   487,  1103,  1313,  -230,    23,
    1347,  -230,  1576,  -230,    88,  -230,    92,  -230,  -230,  -230,
    -230,  -230,  -230,  -230,   736,    94,  -230,   194,  -230,   878,
     998,  1912,  -230,  -230,  -230,  1347,  1032,  -230,   736,  -230
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -230,  -230,  -230,   312,    63,   175,   122,  -230,  -229,    -1,
     -41,    79,  -230,   341,   -37,    64,   -53,  -176,  -134,  -230,
     467,  -230,   177,   188,    16,   -76,   363,  -230,  -230,  -230,
    -230,     0
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -149
static const yytype_int16 yytable[] =
{
      41,    78,   135,    50,   138,    79,   -28,   -28,    78,   155,
     187,   204,    79,   101,   -31,   266,    35,   143,   -31,   196,
     101,  -115,    69,   145,   162,  -115,    41,   -28,   -28,   233,
      86,   236,   102,   158,   144,   274,   -69,   -69,    51,   102,
      73,   146,    35,    41,    97,   103,   145,   145,    84,   279,
      41,   221,   103,   222,    80,    69,    42,    69,   123,    35,
     141,    80,    81,    27,   257,   259,    35,   -31,   -31,    84,
     134,   234,    84,   237,  -115,  -115,   143,   -56,    74,    29,
     -56,   -52,    55,    57,   -52,   156,    69,   -59,   143,    27,
     141,   197,    57,   269,   -56,    86,    56,    58,    53,    12,
      63,    35,    84,    54,   143,    29,    58,   200,    13,    97,
      14,    15,    16,    17,   143,   241,    35,   208,   242,   131,
      18,    19,    29,    84,    41,   -55,    41,   142,   142,    29,
      41,   138,   193,   147,    85,    20,   152,    21,   194,   195,
      35,   143,    35,    69,   272,   143,    35,   241,   273,    22,
     275,    23,   148,    24,    86,   150,   151,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,   128,    97,   164,
     129,   205,   249,    76,    77,   -69,   -69,   254,   207,   256,
    -147,  -147,    76,   219,    69,    69,    41,   220,    34,   189,
     230,   232,    41,   189,   252,    41,   -69,   -69,   263,   111,
     221,    75,    35,    29,   267,    29,   265,   203,    35,    29,
       0,    35,   104,     0,    34,     0,   121,   122,   123,    41,
      41,    41,     0,    41,    41,    69,   125,    41,     0,     0,
      69,    34,    69,     0,     0,    35,    35,    35,    34,    35,
      35,     0,    41,    35,    41,    41,    41,    41,     0,   189,
      41,     0,     0,     0,     0,     0,     0,     0,    35,     0,
      35,    35,    35,    35,    41,    29,    35,     0,     0,    41,
      41,    29,     0,    34,    29,    41,    41,     0,    41,     0,
      35,     0,     0,     0,     0,    35,    35,   189,    34,     0,
     189,    35,    35,     0,    35,     0,     0,     0,    29,    29,
      29,     0,    29,    29,     0,     0,    29,   189,   189,   189,
     189,     0,    34,     0,    34,     0,   -69,   -69,    34,   111,
       0,    29,     0,    29,    29,    29,    29,   189,     0,    29,
       0,     0,     0,   189,     0,     0,   121,   122,   123,   189,
       0,   189,     0,    29,    45,    45,   253,     0,    29,    29,
       0,     0,     0,     0,    29,    29,     0,    29,     0,     0,
       0,    61,   130,    65,     0,     0,    48,    48,     0,   -69,
     -69,    52,   111,     0,    34,     0,     0,     0,     0,     0,
      34,    59,    60,    34,   -69,   -69,     0,   111,     0,   121,
     122,   123,     0,     0,     0,     0,    65,     0,    65,   255,
       0,     0,     0,     0,   121,   122,   123,    34,    34,    34,
       0,    34,    34,     0,   261,    34,     0,     0,   136,   137,
     139,   140,     0,     0,     0,     0,   153,    65,     0,     0,
      34,     0,    34,    34,    34,    34,   186,     0,    34,     0,
       0,   153,     0,     0,     0,     0,     0,     0,     0,   159,
       0,     0,    34,     0,     0,     0,     0,    34,    34,     0,
     160,     0,     0,    34,    34,   163,    34,     0,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,    62,     1,    68,
      72,     2,     3,   -24,   188,   -24,     4,     5,     6,     7,
       8,     0,     0,     0,   224,    11,    12,   227,     0,     0,
       0,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,     0,    68,     0,    68,   214,    65,    18,    19,    45,
       0,   244,   245,   246,     0,   247,     0,     0,   149,     0,
       0,     0,    20,     0,    21,     0,     0,   139,   140,     0,
       0,    48,   154,    68,   264,     0,    22,   226,    23,     0,
      24,   229,   270,     0,   231,   161,    65,   154,     0,     0,
       0,    65,   238,    65,   239,   240,     0,     0,     0,     0,
       0,   276,     0,     0,    61,     0,     0,   278,     0,     0,
       0,     0,     0,     0,     0,    12,     0,     0,   192,     0,
       0,     0,     0,     0,    13,     0,    14,    15,    16,    17,
      68,     0,    72,     0,   206,   271,    18,    19,     0,     0,
       0,     0,     0,     0,     0,    -2,     1,     0,     0,     2,
       3,    20,     0,    21,     4,     5,     6,     7,     8,     0,
       0,     9,    10,    11,    12,    22,     0,    23,   235,    24,
       0,   216,    68,    13,     0,    14,    15,    16,    17,     0,
       0,   105,     0,   228,     0,    18,    19,   -69,   -69,     0,
     111,   149,    72,     0,    72,     0,     0,     0,     0,     0,
      20,     0,    21,   117,   118,   119,   120,   121,   122,   123,
       0,     0,    68,     0,    22,     0,    23,    68,    24,    68,
       0,   149,    -3,     1,   149,     0,     2,     3,     0,     0,
      62,     4,     5,     6,     7,     8,     0,     0,     9,    10,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,   217,     1,     0,     0,
       2,     3,    18,    19,     0,     4,     5,     6,     7,     8,
       0,     0,   -18,   -18,    11,    12,     0,    20,     0,    21,
     -69,   -69,     0,   111,    13,     0,    14,    15,    16,    17,
       0,    22,     0,    23,     0,    24,    18,    19,     0,     0,
     121,   122,   123,     0,     0,     0,     0,     0,     0,     0,
     262,    20,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,     0,    23,     1,    24,
       0,     2,     3,     0,     0,     0,     4,     5,     6,     7,
       8,     0,     0,     0,     0,    11,    12,   -84,   -84,   -84,
     -84,   -84,   -84,   -84,   -84,    13,   -84,    14,    15,    16,
      17,     0,   -84,   -84,   -84,   -84,   -84,    18,    19,   -84,
     -84,   -84,   -84,   -84,   -84,   -84,     0,     0,     0,     0,
       0,     0,   184,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   -84,   185,     0,    23,     1,
      24,     0,     2,     3,     0,     0,     0,     4,     5,     6,
       7,     8,     0,     0,     0,     0,    11,    12,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,    13,  -148,    14,    15,
      16,    17,     0,     0,     0,  -148,  -148,  -148,    18,    19,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,     0,     1,     0,
       0,     2,     3,    20,     0,    21,     4,     5,     6,     7,
       8,     0,     0,     0,     0,    11,    12,    22,     0,    23,
       0,    24,     0,  -147,  -147,    13,     0,    14,    15,    16,
      17,     0,     0,     0,     0,     0,     0,    18,    19,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     223,     0,    20,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,    23,     1,
      24,     0,     2,     3,     0,     0,  -145,     4,     5,     6,
       7,     8,  -145,  -145,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,    13,     0,    14,    15,
      16,    17,     0,     1,     0,     0,     2,     3,    18,    19,
    -146,     4,     5,     6,     7,     8,  -146,  -146,     0,     0,
      11,    12,     0,    20,     0,    21,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,     0,    22,     0,    23,
       0,    24,    18,    19,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    20,     0,    21,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,    23,     1,    24,     0,     2,     3,   -26,
       0,   -26,     4,     5,     6,     7,     8,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,     0,     1,     0,
       0,     2,     3,    18,    19,   191,     4,     5,     6,     7,
       8,     0,     0,     0,     0,    11,    12,     0,    20,     0,
      21,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,     0,    22,     0,    23,     0,    24,    18,    19,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    20,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,    23,     1,
      24,     0,     2,     3,     0,     0,   248,     4,     5,     6,
       7,     8,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,    13,     0,    14,    15,
      16,    17,     0,     1,     0,     0,     2,     3,    18,    19,
     251,     4,     5,     6,     7,     8,     0,     0,     0,     0,
      11,    12,     0,    20,     0,    21,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,     0,    22,     0,    23,
       0,    24,    18,    19,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    20,     0,    21,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,    23,     1,    24,     0,     2,     3,     0,
       0,   268,     4,     5,     6,     7,     8,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,     0,     1,     0,
       0,     2,     3,    18,    19,     0,     4,     5,     6,     7,
       8,     0,     0,     0,     0,    11,    12,     0,    20,     0,
      21,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,     0,    22,     0,    23,     0,    24,    18,    19,     0,
       0,     0,     0,     0,     0,     0,     0,     1,     0,     0,
       2,     3,    20,     0,    21,     4,     5,     6,     7,     8,
       0,     0,     0,     0,    11,    12,    22,     0,    23,     0,
      24,    12,     0,     0,    13,     0,    14,    15,    16,    17,
      13,     0,    14,    15,    16,    17,    18,    19,     0,     0,
       0,    64,    18,    19,     0,     0,     0,     0,     0,     0,
       0,   243,     0,    21,     0,     0,     0,    20,   157,    21,
       0,     0,     0,     0,     0,    22,     0,    23,    12,    24,
       0,    22,     0,    23,    12,    24,     0,    13,     0,    14,
      15,    16,    17,    13,     0,    14,    15,    16,    17,    18,
      19,     0,     0,     0,     0,    18,    19,     0,     0,     0,
       0,     0,   147,     0,    20,     0,    21,     0,   147,     0,
      20,     0,    21,     0,     0,     0,     0,     0,    22,     0,
      23,   258,    24,    12,    22,     0,    23,   260,    24,     0,
       0,     0,    13,     0,    14,    15,    16,    17,     0,     0,
       0,     0,    12,    64,    18,    19,     0,     0,     0,     0,
       0,    13,     0,    14,    15,   213,    17,     0,     0,    20,
       0,    21,    64,    18,    19,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,    23,    12,    24,    20,     0,
      21,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,     0,    22,     0,    23,    12,    24,    18,    19,     0,
       0,     0,     0,     0,    13,     0,    14,    15,    16,    17,
     147,     0,    20,     0,    21,     0,    18,    19,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,    23,    12,
      24,    20,     0,    21,     0,     0,     0,     0,    13,     0,
      14,    15,   124,    17,     0,    22,     0,    23,    12,    24,
      18,    19,     0,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,     0,     0,    20,     0,    21,     0,    18,
      19,     0,     0,     0,     0,     0,     0,     0,     0,    22,
       0,    23,    12,    24,    20,     0,    21,     0,     0,     0,
       0,    13,     0,    14,    15,   213,    17,     0,   225,     0,
      23,    78,    24,    18,    19,    79,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    20,     0,
      21,   105,   106,   107,   108,   109,   110,   -69,   -69,     0,
     111,     0,    22,     0,    23,     0,    24,     0,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
       0,     0,     0,     0,    80,   105,   106,   107,   108,   109,
     110,     0,     0,     0,   111,     0,     0,     0,     0,     0,
       0,     0,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,     0,     0,     0,     0,     0,   105,
     106,   107,   108,   109,   110,     0,     0,     0,   111,     0,
       0,     0,     0,     0,     0,   199,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,     0,     0,
       0,     0,     0,   105,   106,   107,   108,   109,   110,     0,
       0,     0,   111,     0,     0,     0,   100,   101,     0,   202,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,     0,     0,     0,     0,   102,     0,   132,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,   103,
       0,     0,     0,   210,   105,   106,   107,   108,   109,   110,
     -69,   -69,     0,   111,     0,     0,     0,   250,     0,     0,
       0,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   105,   106,   107,   108,   109,   110,     0,
       0,     0,   111,     0,     0,     0,     0,     0,     0,     0,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   105,   106,   107,   108,   109,   110,     0,     0,
       0,   111,     0,     0,     0,     0,     0,     0,     0,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,     0,     0,     0,     0,     0,     0,     0,   277,   105,
     106,   107,   108,   109,   110,   -67,   -67,     0,   111,     0,
       0,     0,     0,     0,     0,     0,   112,   113,   212,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   105,   106,
     107,   108,   109,   110,     0,     0,     0,   111,     0,     0,
       0,     0,     0,     0,     0,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   105,   106,   107,
     108,   109,   110,     0,     0,     0,   111,     0,     0,     0,
       0,     0,     0,     0,   112,   113,   114,   115,   116,   117,
     118,   119,   198,   121,   122,   123,   105,   106,   107,   108,
     109,   110,     0,     0,     0,   111,     0,     0,     0,     0,
       0,     0,     0,   112,   113,   114,   115,   116,   117,   118,
     119,   201,   121,   122,   123,   105,   106,   107,   108,   109,
     110,     0,     0,     0,   111,     0,     0,     0,     0,     0,
       0,     0,   112,   113,   114,   115,   116,   117,   118,   119,
     209,   121,   122,   123,   105,   106,   107,   108,   109,   110,
       0,     0,     0,   111,     0,     0,     0,     0,     0,     0,
       0,   112,   113,   114,   115,   116,   117,   118,   119,   211,
     121,   122,   123,   105,   106,   107,   108,   109,   110,     0,
       0,     0,   111,     0,     0,     0,     0,     0,     0,     0,
     112,   113,   212,   115,   116,   117,   118,   119,   120,   121,
     122,   123
};

static const yytype_int16 yycheck[] =
{
       0,     0,    55,     4,    57,     4,    26,    27,     0,    85,
      35,   145,     4,    36,     0,   244,     0,    53,     4,    35,
      36,     0,    22,    54,   100,     4,    26,    26,    27,   205,
      55,   207,    55,    86,    70,   264,    26,    27,    32,    55,
      24,    72,    26,    43,    69,    68,    54,    54,    68,   278,
      50,     6,    68,     8,    53,    55,     4,    57,    48,    43,
      56,    53,    54,     0,    72,    72,    50,    53,    54,    68,
      54,   205,    68,   207,    53,    54,    53,    53,     0,     0,
      56,    53,    55,    55,    56,    85,    86,    53,    53,    26,
      56,    56,    55,    70,    70,    55,    69,    69,    16,    19,
      21,    85,    68,    55,    53,    26,    69,    56,    28,    69,
      30,    31,    32,    33,    53,    53,   100,    56,    56,    35,
      40,    41,    43,    68,   124,    53,   126,    56,    56,    50,
     130,   184,     8,    53,    35,    55,    32,    57,    14,    15,
     124,    53,   126,   143,    56,    53,   130,    53,    56,    69,
      56,    71,    72,    73,    55,    76,    77,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    45,    69,    32,
      48,    71,   225,    26,    27,    26,    27,   230,    71,   232,
      26,    27,    26,    32,   184,   185,   186,   188,     0,   126,
      55,    55,   192,   130,    53,   195,    26,    27,    32,    29,
       6,    26,   186,   124,   245,   126,   243,   143,   192,   130,
      -1,   195,    35,    -1,    26,    -1,    46,    47,    48,   219,
     220,   221,    -1,   223,   224,   225,    38,   227,    -1,    -1,
     230,    43,   232,    -1,    -1,   219,   220,   221,    50,   223,
     224,    -1,   242,   227,   244,   245,   246,   247,    -1,   186,
     250,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   242,    -1,
     244,   245,   246,   247,   264,   186,   250,    -1,    -1,   269,
     270,   192,    -1,    85,   195,   275,   276,    -1,   278,    -1,
     264,    -1,    -1,    -1,    -1,   269,   270,   224,   100,    -1,
     227,   275,   276,    -1,   278,    -1,    -1,    -1,   219,   220,
     221,    -1,   223,   224,    -1,    -1,   227,   244,   245,   246,
     247,    -1,   124,    -1,   126,    -1,    26,    27,   130,    29,
      -1,   242,    -1,   244,   245,   246,   247,   264,    -1,   250,
      -1,    -1,    -1,   270,    -1,    -1,    46,    47,    48,   276,
      -1,   278,    -1,   264,     3,     4,    56,    -1,   269,   270,
      -1,    -1,    -1,    -1,   275,   276,    -1,   278,    -1,    -1,
      -1,    20,    50,    22,    -1,    -1,     3,     4,    -1,    26,
      27,     8,    29,    -1,   186,    -1,    -1,    -1,    -1,    -1,
     192,    18,    19,   195,    26,    27,    -1,    29,    -1,    46,
      47,    48,    -1,    -1,    -1,    -1,    55,    -1,    57,    56,
      -1,    -1,    -1,    -1,    46,    47,    48,   219,   220,   221,
      -1,   223,   224,    -1,    56,   227,    -1,    -1,    55,    56,
      57,    58,    -1,    -1,    -1,    -1,    85,    86,    -1,    -1,
     242,    -1,   244,   245,   246,   247,   124,    -1,   250,    -1,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,   264,    -1,    -1,    -1,    -1,   269,   270,    -1,
      97,    -1,    -1,   275,   276,   102,   278,    -1,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    20,     1,    22,
      23,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    -1,    -1,    -1,   192,    18,    19,   195,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    30,    31,    32,
      33,    -1,    55,    -1,    57,   184,   185,    40,    41,   188,
      -1,   219,   220,   221,    -1,   223,    -1,    -1,    71,    -1,
      -1,    -1,    55,    -1,    57,    -1,    -1,   184,   185,    -1,
      -1,   188,    85,    86,   242,    -1,    69,   194,    71,    -1,
      73,   198,   250,    -1,   201,    98,   225,   100,    -1,    -1,
      -1,   230,   209,   232,   211,   212,    -1,    -1,    -1,    -1,
      -1,   269,    -1,    -1,   243,    -1,    -1,   275,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    19,    -1,    -1,   131,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    30,    31,    32,    33,
     143,    -1,   145,    -1,   147,   252,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     0,     1,    -1,    -1,     4,
       5,    55,    -1,    57,     9,    10,    11,    12,    13,    -1,
      -1,    16,    17,    18,    19,    69,    -1,    71,    72,    73,
      -1,   184,   185,    28,    -1,    30,    31,    32,    33,    -1,
      -1,    20,    -1,   196,    -1,    40,    41,    26,    27,    -1,
      29,   204,   205,    -1,   207,    -1,    -1,    -1,    -1,    -1,
      55,    -1,    57,    42,    43,    44,    45,    46,    47,    48,
      -1,    -1,   225,    -1,    69,    -1,    71,   230,    73,   232,
      -1,   234,     0,     1,   237,    -1,     4,     5,    -1,    -1,
     243,     9,    10,    11,    12,    13,    -1,    -1,    16,    17,
      18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    30,    31,    32,    33,     0,     1,    -1,    -1,
       4,     5,    40,    41,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    16,    17,    18,    19,    -1,    55,    -1,    57,
      26,    27,    -1,    29,    28,    -1,    30,    31,    32,    33,
      -1,    69,    -1,    71,    -1,    73,    40,    41,    -1,    -1,
      46,    47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    71,     1,    73,
      -1,     4,     5,    -1,    -1,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    -1,    -1,    -1,
      -1,    -1,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    69,    -1,    71,     1,
      73,    -1,     4,     5,    -1,    -1,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    -1,    -1,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,     1,    -1,
      -1,     4,     5,    55,    -1,    57,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    18,    19,    69,    -1,    71,
      -1,    73,    -1,    26,    27,    28,    -1,    30,    31,    32,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    40,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      53,    -1,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    71,     1,
      73,    -1,     4,     5,    -1,    -1,     8,     9,    10,    11,
      12,    13,    14,    15,    -1,    -1,    18,    19,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    30,    31,
      32,    33,    -1,     1,    -1,    -1,     4,     5,    40,    41,
       8,     9,    10,    11,    12,    13,    14,    15,    -1,    -1,
      18,    19,    -1,    55,    -1,    57,    -1,    -1,    -1,    -1,
      28,    -1,    30,    31,    32,    33,    -1,    69,    -1,    71,
      -1,    73,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    71,     1,    73,    -1,     4,     5,     6,
      -1,     8,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    30,    31,    32,    33,    -1,     1,    -1,
      -1,     4,     5,    40,    41,     8,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    18,    19,    -1,    55,    -1,
      57,    -1,    -1,    -1,    -1,    28,    -1,    30,    31,    32,
      33,    -1,    69,    -1,    71,    -1,    73,    40,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    71,     1,
      73,    -1,     4,     5,    -1,    -1,     8,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    18,    19,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    30,    31,
      32,    33,    -1,     1,    -1,    -1,     4,     5,    40,    41,
       8,     9,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      18,    19,    -1,    55,    -1,    57,    -1,    -1,    -1,    -1,
      28,    -1,    30,    31,    32,    33,    -1,    69,    -1,    71,
      -1,    73,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    71,     1,    73,    -1,     4,     5,    -1,
      -1,     8,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    30,    31,    32,    33,    -1,     1,    -1,
      -1,     4,     5,    40,    41,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    18,    19,    -1,    55,    -1,
      57,    -1,    -1,    -1,    -1,    28,    -1,    30,    31,    32,
      33,    -1,    69,    -1,    71,    -1,    73,    40,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,
       4,     5,    55,    -1,    57,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    18,    19,    69,    -1,    71,    -1,
      73,    19,    -1,    -1,    28,    -1,    30,    31,    32,    33,
      28,    -1,    30,    31,    32,    33,    40,    41,    -1,    -1,
      -1,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    57,    -1,    -1,    -1,    55,    56,    57,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    71,    19,    73,
      -1,    69,    -1,    71,    19,    73,    -1,    28,    -1,    30,
      31,    32,    33,    28,    -1,    30,    31,    32,    33,    40,
      41,    -1,    -1,    -1,    -1,    40,    41,    -1,    -1,    -1,
      -1,    -1,    53,    -1,    55,    -1,    57,    -1,    53,    -1,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      71,    72,    73,    19,    69,    -1,    71,    72,    73,    -1,
      -1,    -1,    28,    -1,    30,    31,    32,    33,    -1,    -1,
      -1,    -1,    19,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    30,    31,    32,    33,    -1,    -1,    55,
      -1,    57,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    71,    19,    73,    55,    -1,
      57,    -1,    -1,    -1,    -1,    28,    -1,    30,    31,    32,
      33,    -1,    69,    -1,    71,    19,    73,    40,    41,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    30,    31,    32,    33,
      53,    -1,    55,    -1,    57,    -1,    40,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    71,    19,
      73,    55,    -1,    57,    -1,    -1,    -1,    -1,    28,    -1,
      30,    31,    32,    33,    -1,    69,    -1,    71,    19,    73,
      40,    41,    -1,    -1,    -1,    -1,    -1,    28,    -1,    30,
      31,    32,    33,    -1,    -1,    55,    -1,    57,    -1,    40,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    71,    19,    73,    55,    -1,    57,    -1,    -1,    -1,
      -1,    28,    -1,    30,    31,    32,    33,    -1,    69,    -1,
      71,     0,    73,    40,    41,     4,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      57,    20,    21,    22,    23,    24,    25,    26,    27,    -1,
      29,    -1,    69,    -1,    71,    -1,    73,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    -1,    -1,    -1,    53,    20,    21,    22,    23,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    -1,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    -1,
      -1,    -1,    -1,    20,    21,    22,    23,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    35,    36,    -1,    70,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    -1,    -1,    -1,    55,    -1,     4,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    -1,    -1,    70,    20,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,    -1,    -1,    -1,     4,    -1,    -1,
      -1,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    20,    21,    22,    23,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    20,    21,    22,    23,    24,    25,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    20,
      21,    22,    23,    24,    25,    26,    27,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    20,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    20,    21,    22,
      23,    24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    20,    21,    22,    23,
      24,    25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    20,    21,    22,    23,    24,
      25,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    20,    21,    22,    23,    24,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    20,    21,    22,    23,    24,    25,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     4,     5,     9,    10,    11,    12,    13,    16,
      17,    18,    19,    28,    30,    31,    32,    33,    40,    41,
      55,    57,    69,    71,    73,    75,    76,    78,    79,    85,
      86,    87,    93,    94,    97,    98,    99,   100,   101,   102,
     104,   105,     4,    83,    85,    87,    97,    98,   100,   105,
      83,    32,   100,    16,    55,    55,    69,    55,    69,   100,
     100,    87,    94,    85,    39,    87,    89,    90,    94,   105,
      91,    92,    94,    98,     0,    79,    26,    27,     0,     4,
      53,    54,    80,    81,    68,    35,    55,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    69,    95,    96,
      35,    36,    55,    68,    96,    20,    21,    22,    23,    24,
      25,    29,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    32,    97,    77,    78,    80,    80,
      77,    35,     4,   103,    98,    90,   100,   100,    90,   100,
     100,    56,    56,    53,    70,    54,    72,    53,    72,    94,
      85,    85,    32,    87,    94,    99,   105,    56,    90,   100,
     100,    94,    99,   100,    32,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100,   100,    55,    69,    77,    35,     7,    78,
      84,     8,    94,     8,    14,    15,    35,    56,    45,    70,
      56,    45,    70,    89,    92,    71,    94,    71,    56,    45,
      70,    45,    39,    32,    87,    88,    94,     0,    82,    32,
      83,     6,     8,    53,    77,    69,   100,    77,    94,   100,
      55,   100,    55,    91,    92,    72,    91,    92,   100,   100,
     100,    53,    56,    55,    77,    77,    77,    77,     8,    90,
       4,     8,    53,    56,    90,    56,    90,    72,    72,    72,
      72,    56,    56,    32,    77,    88,    82,    84,     8,    70,
      77,   100,    56,    56,    82,    56,    77,    56,    77,    82
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
      case 31: /* "\"string\"" */

/* Line 1000 of yacc.c  */
#line 96 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding string \"%s\"\n", (yyvaluep->str));
#endif
  free((yyvaluep->str));
};

/* Line 1000 of yacc.c  */
#line 1769 "psycon.tab.c"
	break;
      case 32: /* "\"identifier\"" */

/* Line 1000 of yacc.c  */
#line 96 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding string \"%s\"\n", (yyvaluep->str));
#endif
  free((yyvaluep->str));
};

/* Line 1000 of yacc.c  */
#line 1783 "psycon.tab.c"
	break;
      case 76: /* "block_func" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1797 "psycon.tab.c"
	break;
      case 77: /* "block" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1811 "psycon.tab.c"
	break;
      case 78: /* "line" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1825 "psycon.tab.c"
	break;
      case 79: /* "line_func" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1839 "psycon.tab.c"
	break;
      case 83: /* "conditional" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1853 "psycon.tab.c"
	break;
      case 84: /* "elseif_list" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1867 "psycon.tab.c"
	break;
      case 85: /* "expcondition" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1881 "psycon.tab.c"
	break;
      case 86: /* "stmt" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1895 "psycon.tab.c"
	break;
      case 87: /* "condition" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1909 "psycon.tab.c"
	break;
      case 88: /* "id_list" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1923 "psycon.tab.c"
	break;
      case 89: /* "arg" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1937 "psycon.tab.c"
	break;
      case 90: /* "arg_list" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1951 "psycon.tab.c"
	break;
      case 91: /* "matrix" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1965 "psycon.tab.c"
	break;
      case 92: /* "vector" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1979 "psycon.tab.c"
	break;
      case 93: /* "range" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 1993 "psycon.tab.c"
	break;
      case 94: /* "exp_range" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2007 "psycon.tab.c"
	break;
      case 95: /* "compop" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2021 "psycon.tab.c"
	break;
      case 96: /* "assign2this" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2035 "psycon.tab.c"
	break;
      case 97: /* "varblock" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2049 "psycon.tab.c"
	break;
      case 98: /* "tid" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2063 "psycon.tab.c"
	break;
      case 99: /* "assign" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2077 "psycon.tab.c"
	break;
      case 100: /* "exp" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2091 "psycon.tab.c"
	break;
      case 101: /* "func_static_or_not" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2105 "psycon.tab.c"
	break;
      case 102: /* "funcdef" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2119 "psycon.tab.c"
	break;
      case 103: /* "case_list" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2133 "psycon.tab.c"
	break;
      case 104: /* "csig" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2147 "psycon.tab.c"
	break;
      case 105: /* "initcell" */

/* Line 1000 of yacc.c  */
#line 89 "psycon.y"
	{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName((yyvaluep->pnode)));
#endif
  yydeleteAstNode((yyvaluep->pnode), 0);
};

/* Line 1000 of yacc.c  */
#line 2161 "psycon.tab.c"
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
#line 80 "psycon.y"
{
  if (ErrorMsg) {
	free(ErrorMsg);
	ErrorMsg = NULL;
  }
  *errmsg = NULL;
}

/* Line 1242 of yacc.c  */
#line 2322 "psycon.tab.c"

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
#line 123 "psycon.y"
    { *pproot = NULL;;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 125 "psycon.y"
    { *pproot = (yyvsp[(1) - (1)].pnode);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 129 "psycon.y"
    { (yyval.pnode) = (yyvsp[(1) - (1)].pnode); ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 131 "psycon.y"
    { //yyn=5
		if ((yyvsp[(2) - (2)].pnode)) {
			if ((yyvsp[(1) - (2)].pnode) == NULL)
				(yyval.pnode) = (yyvsp[(2) - (2)].pnode);
			else if ((yyvsp[(1) - (2)].pnode)->type == N_BLOCK) 
			{
//				$$ = $1;
				(yyvsp[(1) - (2)].pnode)->tail->next = (yyvsp[(2) - (2)].pnode);
				(yyvsp[(1) - (2)].pnode)->tail = (yyvsp[(2) - (2)].pnode);
			} 
			else 
			{ // a=1; b=2; ==> $1->type is '='. So first, a N_BLOCK tree should be made.
				(yyval.pnode) = newAstNode(N_BLOCK, (yyloc));
				(yyval.pnode)->next = (yyvsp[(1) - (2)].pnode);
				(yyvsp[(1) - (2)].pnode)->next = (yyval.pnode)->tail = (yyvsp[(2) - (2)].pnode);
#ifdef _DEBUG				
				(yyval.pnode)->str = (char*)malloc(32);
				strcpy_s ((yyval.pnode)->str, 32, "block_begins,yyn=5");
#endif
			}
		} else
			(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
	;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 157 "psycon.y"
    {
		if ((yyvsp[(1) - (1)].pnode)) // if cond1, x=1, end ==> x=1 comes here.
			(yyval.pnode) = (yyvsp[(1) - (1)].pnode);
		else
			(yyval.pnode) = newAstNode(N_BLOCK, (yyloc));
	;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 164 "psycon.y"
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
#ifdef _DEBUG				
				(yyval.pnode)->str = (char*)malloc(32);
				strcpy_s ((yyval.pnode)->str, 32, "block_begins,yyn=7");
#endif				
			}
		}
		else // only "block" is given
			(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
	;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 190 "psycon.y"
    { (yyval.pnode) = NULL;;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 192 "psycon.y"
    { //yyn=9
		(yyval.pnode) = NULL;
		yyerrok;
	;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 197 "psycon.y"
    { (yyval.pnode) = (yyvsp[(1) - (2)].pnode);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 199 "psycon.y"
    { (yyval.pnode) = (yyvsp[(1) - (2)].pnode); (yyval.pnode)->suppress=1;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 203 "psycon.y"
    { (yyval.pnode) = (yyvsp[(1) - (1)].pnode);;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 205 "psycon.y"
    { (yyval.pnode) = (yyvsp[(1) - (1)].pnode);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 221 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_IF, (yyloc));
	;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 225 "psycon.y"
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

  case 26:

/* Line 1455 of yacc.c  */
#line 238 "psycon.y"
    { //yyn=26; 
		if ((yyvsp[(1) - (3)].pnode)->child==NULL) // in this case $1 is T_IF created as /*empty*/ 
		{  
			yydeleteAstNode((yyvsp[(1) - (3)].pnode), 1);
			(yyval.pnode) = (yyvsp[(3) - (3)].pnode);
		}
		else
		{
			(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
			AstNode *p = (yyvsp[(3) - (3)].pnode);
			if (p->type!=N_BLOCK)
			{
				p = newAstNode(N_BLOCK, (yylsp[(3) - (3)]));
				p->next = (yyvsp[(3) - (3)].pnode);
			}
			(yyvsp[(1) - (3)].pnode)->alt = p;
		}
	;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 266 "psycon.y"
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
#line 292 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(3) - (4)].pnode);
		(yyvsp[(2) - (4)].pnode)->next = (yyvsp[(3) - (4)].pnode)->child;
		(yyval.pnode)->child = (yyvsp[(2) - (4)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 300 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(3) - (6)].pnode);
		(yyvsp[(2) - (6)].pnode)->next = (yyvsp[(3) - (6)].pnode)->child;
		(yyvsp[(3) - (6)].pnode)->tail->next = (yyvsp[(5) - (6)].pnode);
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 309 "psycon.y"
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

  case 36:

/* Line 1455 of yacc.c  */
#line 321 "psycon.y"
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

  case 37:

/* Line 1455 of yacc.c  */
#line 335 "psycon.y"
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

  case 38:

/* Line 1455 of yacc.c  */
#line 349 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_RETURN, (yyloc));
	;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 353 "psycon.y"
    { (yyval.pnode) = newAstNode(T_BREAK, (yyloc));;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 355 "psycon.y"
    { (yyval.pnode) = newAstNode(T_CONTINUE, (yyloc));;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 361 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('<', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 363 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('>', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 365 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_EQ, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 367 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_NE, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 369 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_GE, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 371 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_LE, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 373 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 379 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_LOGIC_NOT, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 384 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_AND, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 386 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_LOGIC_OR, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 390 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_IDLIST, (yyloc));
	;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 394 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_IDLIST, (yyloc));
		(yyval.pnode)->child = (yyval.pnode)->tail = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->tail->str = (yyvsp[(1) - (1)].str);
	;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 400 "psycon.y"
    {
		(yyvsp[(1) - (3)].pnode)->tail = (yyvsp[(1) - (3)].pnode)->tail->next = newAstNode(T_ID, (yylsp[(3) - (3)]));
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		(yyval.pnode)->tail->str = (yyvsp[(3) - (3)].str);
	;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 408 "psycon.y"
    {	(yyval.pnode) = newAstNode(T_FULLRANGE, (yyloc)); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 414 "psycon.y"
    { //yyn=57
		(yyval.pnode) = newAstNode(N_ARGS, (yyloc));
		(yyval.pnode)->tail = (yyval.pnode)->child = (yyvsp[(1) - (1)].pnode);
	;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 419 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		if ((yyval.pnode)->tail)
			(yyval.pnode)->tail = (yyval.pnode)->tail->next = (yyvsp[(3) - (3)].pnode);
		else
			(yyval.pnode)->tail = (yyval.pnode)->next = (yyvsp[(3) - (3)].pnode);
	;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 430 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_MATRIX, (yyloc));
		(yyval.pnode)->tail = (yyval.pnode)->child = newAstNode(N_VECTOR, (yyloc));
	;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 435 "psycon.y"
    { //yyn=62
		(yyval.pnode) = newAstNode(N_MATRIX, (yyloc));
		(yyval.pnode)->child = (yyval.pnode)->tail = (yyvsp[(1) - (1)].pnode);
	;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 440 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		(yyvsp[(1) - (3)].pnode)->tail = (yyvsp[(1) - (3)].pnode)->tail->next = (yyvsp[(3) - (3)].pnode);		
	;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 448 "psycon.y"
    { //yyn=65
		(yyval.pnode) = newAstNode(N_VECTOR, (yyloc));
		(yyval.pnode)->child = (yyval.pnode)->tail = (yyvsp[(1) - (1)].pnode);
	;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 453 "psycon.y"
    {
		(yyvsp[(1) - (2)].pnode)->tail = (yyvsp[(1) - (2)].pnode)->tail->next = (yyvsp[(2) - (2)].pnode);
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
	;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 458 "psycon.y"
    {
		(yyvsp[(1) - (3)].pnode)->tail = (yyvsp[(1) - (3)].pnode)->tail->next = (yyvsp[(3) - (3)].pnode);
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
	;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 465 "psycon.y"
    {
		(yyval.pnode) = makeFunctionCall(":", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));
	;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 469 "psycon.y"
    {//69
		(yyval.pnode) = makeFunctionCall(":", (yyvsp[(1) - (5)].pnode), (yyvsp[(5) - (5)].pnode), (yyloc));
		(yyvsp[(5) - (5)].pnode)->next = (yyvsp[(3) - (5)].pnode);
	;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 478 "psycon.y"
    { 	
		(yyval.pnode) = newAstNode('+', (yyloc));
	;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 482 "psycon.y"
    { 		(yyval.pnode) = newAstNode('-', (yyloc));	;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 484 "psycon.y"
    { 		(yyval.pnode) = newAstNode('*', (yyloc));	;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 486 "psycon.y"
    { 		(yyval.pnode) = newAstNode('/', (yyloc));	;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 488 "psycon.y"
    { 		(yyval.pnode) = newAstNode('@', (yyloc));	;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 490 "psycon.y"
    {
		(yyval.pnode) = newAstNode('@', (yyloc));	
		(yyval.pnode)->child = newAstNode('@', (yyloc));
	;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 495 "psycon.y"
    { 		(yyval.pnode) = newAstNode(T_OP_CONCAT, (yyloc));	;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 497 "psycon.y"
    { 		(yyval.pnode) = newAstNode(T_OP_SHIFT, (yyloc));	;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 499 "psycon.y"
    { 		(yyval.pnode) = newAstNode('%', (yyloc));	;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 501 "psycon.y"
    { 		
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = strdup("movespec");
		(yyval.pnode)->tail = (yyval.pnode)->child = newAstNode(N_ARGS, (yyloc));
	;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 509 "psycon.y"
    { //82
		(yyval.pnode) = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 513 "psycon.y"
    { 
		(yyval.pnode) = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 517 "psycon.y"
    { 
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
		if ((yyval.pnode)->child) // compop should be "@@=" and $$->child->type should be '@'  (64)
		{
			(yyval.pnode)->child->child = newAstNode(T_REPLICA, (yyloc));
			(yyval.pnode)->child->tail = (yyval.pnode)->child->child->next = newAstNode(T_REPLICA, (yyloc));
		}
		else
		{
			(yyval.pnode)->child = newAstNode(T_REPLICA, (yylsp[(2) - (2)]));
		}
		(yyval.pnode)->tail = (yyval.pnode)->child->next = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 533 "psycon.y"
    { //85
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (1)].str);
	;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 538 "psycon.y"
    {//86
		(yyval.pnode) = (yyvsp[(1) - (3)].pnode);
		AstNode *p = newAstNode(N_STRUCT, (yyloc));
		p->str = (yyvsp[(3) - (3)].str);
		if ((yyval.pnode)->type==N_CELL)
		{
			(yyval.pnode)->alt->alt = p; //always initiating
			(yyval.pnode)->tail = p; // so that next concatenation can point to p, even thought p is "hidden" underneath $$->alt
		}
		else
		{
			// When tid is N_VECTOR, it needs to nullify tail; otherwise, it may create a conflict down the road: for example 90 to 91.
			if ((yyval.pnode)->tail!=NULL && ((yyval.pnode)->type==N_VECTOR || (yyval.pnode)->type==N_MATRIX) )
				(yyval.pnode)->tail=NULL;
			//Concatenated dot expressions go through $$->tail
			if ((yyval.pnode)->tail!=NULL)
				(yyval.pnode)->tail->alt = p; // update $$->tail
			else
				(yyval.pnode)->alt = p; // initiate $$->tail
			(yyval.pnode)->tail = p;
		}
	;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 561 "psycon.y"
    {
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

  case 87:

/* Line 1455 of yacc.c  */
#line 573 "psycon.y"
    {//88
		(yyval.pnode) = (yyvsp[(1) - (4)].pnode);
		(yyval.pnode)->tail = (yyval.pnode)->alt->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->tail->child = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 579 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (6)].pnode);
		(yyval.pnode)->tail = (yyval.pnode)->tail->alt = newAstNode(N_TIME_EXTRACT, (yylsp[(2) - (6)]));
		(yyval.pnode)->tail->child = (yyvsp[(3) - (6)].pnode);
		(yyval.pnode)->tail->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 586 "psycon.y"
    {//tid-vector 90
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
	;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 593 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (4)].str);
		(yyval.pnode)->tail = (yyval.pnode)->alt = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 599 "psycon.y"
    { //92
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (4)].str);
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->alt->child = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 606 "psycon.y"
    { //93
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (7)].str);
		(yyval.pnode)->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->alt->child = (yyvsp[(3) - (7)].pnode);
		(yyval.pnode)->tail = (yyval.pnode)->alt->alt = (yyvsp[(6) - (7)].pnode);
	;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 614 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ID, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (6)].str);
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_TIME_EXTRACT, (yylsp[(2) - (6)]));
		(yyval.pnode)->tail->child = (yyvsp[(3) - (6)].pnode);
		(yyvsp[(3) - (6)].pnode)->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 622 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (4)].pnode);
		if ((yyval.pnode)->tail)
			(yyval.pnode)->tail = (yyval.pnode)->tail->alt = (yyvsp[(3) - (4)].pnode); // just remember--- this is different from $$->tail->alt = $$->tail = $3; 8/20/2018
		else
			(yyval.pnode)->tail = (yyval.pnode)->alt = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 630 "psycon.y"
    { // 95
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

  case 97:

/* Line 1455 of yacc.c  */
#line 642 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
	;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 646 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
		(yyval.pnode)->tail = (yyval.pnode)->alt = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 651 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->alt->child = (yyvsp[(3) - (4)].pnode);
		(yyval.pnode)->tail = (yyval.pnode)->alt = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 658 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
		(yyval.pnode)->alt = newAstNode(N_CELL, (yyloc));
		(yyval.pnode)->alt->child = (yyvsp[(3) - (7)].pnode);
		(yyval.pnode)->tail = (yyval.pnode)->alt->alt = (yyvsp[(6) - (7)].pnode);
	;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 665 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_REPLICA, (yyloc));
		(yyval.pnode)->tail = (yyval.pnode)->alt = newAstNode(N_TIME_EXTRACT, (yylsp[(2) - (6)]));
		(yyval.pnode)->tail->child = (yyvsp[(3) - (6)].pnode);
		(yyvsp[(3) - (6)].pnode)->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 672 "psycon.y"
    { 	
		(yyval.pnode) = newAstNode(T_TRANSPOSE, (yyloc));
 		(yyval.pnode)->child = (yyvsp[(1) - (2)].pnode);
	;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 683 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (5)].pnode);
	;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 688 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 694 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 700 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->str = malloc(8);
		strcpy((yyval.pnode)->str, "R");
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 708 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_TSEQ, (yyloc));
		(yyval.pnode)->str = malloc(8);
		strcpy((yyval.pnode)->str, "R");
		(yyval.pnode)->child = (yyvsp[(2) - (6)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
	;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 716 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
	;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 720 "psycon.y"
    {
		(yyval.pnode) = newAstNode(N_HOOK, (yyloc));
		(yyval.pnode)->str = strdup((yyvsp[(2) - (2)].pnode)->str);
	;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 725 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 734 "psycon.y"
    { //111
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
		(yyval.pnode)->child = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 739 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (2)].pnode);
		//If LHS is vector, move the child node (the vector content) to alt. That's OK because the vector on LHS cannot have index or arg_list. Note that this is not really a vector, but only a list of variable list to store the results of RHS. 9/13/2018
		if ((yyvsp[(1) - (2)].pnode)->type==N_VECTOR)
		{ 
			if ((yyval.pnode)->alt) yydeleteAstNode((yyval.pnode)->alt, 1); // $$->alt shouldn't have a meaningful node, but instead of overwriting, let's clean it properly.
			(yyval.pnode)->alt = (yyval.pnode)->child;
		}
		(yyval.pnode)->child = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 750 "psycon.y"
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

  case 114:

/* Line 1455 of yacc.c  */
#line 765 "psycon.y"
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

  case 115:

/* Line 1455 of yacc.c  */
#line 780 "psycon.y"
    { // x={"bjk",noise(300), 4.5555}
		(yyval.pnode)->str = getT_ID_str((yyvsp[(1) - (3)].pnode));
		(yyval.pnode)->child = (yyvsp[(3) - (3)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 796 "psycon.y"
    { // 116
		(yyval.pnode) = newAstNode(T_NUMBER, (yyloc));
		(yyval.pnode)->dval = (yyvsp[(1) - (1)].dval);
	;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 801 "psycon.y"
    { // 117
		(yyval.pnode) = newAstNode(T_STRING, (yyloc));
		(yyval.pnode)->str = (yyvsp[(1) - (1)].str);
	;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 806 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_ENDPOINT, (yyloc));
	;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 811 "psycon.y"
    {//120
		(yyval.pnode) = (yyvsp[(1) - (1)].pnode);
	;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 815 "psycon.y"
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

  case 122:

/* Line 1455 of yacc.c  */
#line 831 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_NEGATIVE, (yyloc));
		(yyval.pnode)->child = (yyvsp[(2) - (2)].pnode);
	;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 836 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (2)].pnode);
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 842 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_SIGMA, (yyloc));
		(yyval.pnode)->child = newAstNode(T_ID, (yylsp[(3) - (8)]));
		(yyval.pnode)->child->str = getT_ID_str((yyvsp[(3) - (8)].pnode));
		(yyval.pnode)->child->child = (yyvsp[(5) - (8)].pnode);
		(yyval.pnode)->child->next = (yyvsp[(7) - (8)].pnode);
	;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 850 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('+', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 852 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('-', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 854 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('*', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 856 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('/', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 858 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_MATRIXMULT, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 860 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("^", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 862 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("mod", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 864 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("interp", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 866 "psycon.y"
    { (yyval.pnode) = makeFunctionCall("movespec", (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 868 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode('@', (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 870 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_OP_SHIFT, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 872 "psycon.y"
    { (yyval.pnode) = makeBinaryOpNode(T_OP_CONCAT, (yyvsp[(1) - (3)].pnode), (yyvsp[(3) - (3)].pnode), (yyloc));;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 876 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_FUNCTION, (yyloc));
		(yyval.pnode)->suppress = 2;
	;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 881 "psycon.y"
    {
		(yyval.pnode) = newAstNode(T_FUNCTION, (yyloc));
		(yyval.pnode)->suppress = 3;
	;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 888 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (4)].pnode);
		(yyval.pnode)->str = (yyvsp[(2) - (4)].str);
		(yyval.pnode)->child = newAstNode(N_IDLIST, (yyloc));
		(yyval.pnode)->child->next = (yyvsp[(3) - (4)].pnode);
	;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 895 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (6)].pnode);
		(yyval.pnode)->str = (yyvsp[(4) - (6)].str);
		(yyval.pnode)->child = newAstNode(N_IDLIST, (yyloc));
		(yyval.pnode)->child->next = (yyvsp[(5) - (6)].pnode);
		if ((yyvsp[(2) - (6)].pnode)->type!=N_VECTOR)
		{
			AstNode *p = newAstNode(N_VECTOR, (yylsp[(2) - (6)]));
			p->child = p->tail = (yyvsp[(2) - (6)].pnode);
			(yyval.pnode)->alt = p;
		}
		else
		{
			(yyval.pnode)->alt = (yyvsp[(2) - (6)].pnode);
		}
	;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 912 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (7)].pnode);
		(yyval.pnode)->str = (yyvsp[(2) - (7)].str);
		(yyval.pnode)->child = (yyvsp[(4) - (7)].pnode);
		(yyvsp[(4) - (7)].pnode)->next = (yyvsp[(6) - (7)].pnode);
	;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 919 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(1) - (9)].pnode);
		(yyval.pnode)->str = (yyvsp[(4) - (9)].str);
		(yyval.pnode)->child = (yyvsp[(6) - (9)].pnode);
		(yyvsp[(6) - (9)].pnode)->next = (yyvsp[(8) - (9)].pnode);
		if ((yyvsp[(2) - (9)].pnode)->type!=N_VECTOR)
		{
			AstNode *p = newAstNode(N_VECTOR, (yylsp[(2) - (9)]));
			p->child = p->tail = (yyvsp[(2) - (9)].pnode);
			(yyval.pnode)->alt = p;
		}
		else
		{
			(yyval.pnode)->alt = (yyvsp[(2) - (9)].pnode);
		}
	;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 938 "psycon.y"
    { (yyval.pnode) = newAstNode(T_SWITCH, (yyloc));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 940 "psycon.y"
    { (yyval.pnode) = newAstNode(T_SWITCH, (yyloc));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 942 "psycon.y"
    {
		if ((yyvsp[(1) - (5)].pnode)->child)
			(yyvsp[(1) - (5)].pnode)->tail->next = (yyvsp[(3) - (5)].pnode);
		else
			(yyvsp[(1) - (5)].pnode)->child = (yyvsp[(3) - (5)].pnode);
		(yyvsp[(3) - (5)].pnode)->next = (yyvsp[(5) - (5)].pnode);
		(yyvsp[(1) - (5)].pnode)->tail = (yyvsp[(5) - (5)].pnode);
		(yyval.pnode) = (yyvsp[(1) - (5)].pnode);
	;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 952 "psycon.y"
    {
		if ((yyvsp[(1) - (6)].pnode)->child)
			(yyvsp[(1) - (6)].pnode)->tail->next = (yyvsp[(4) - (6)].pnode);
		else
			(yyvsp[(1) - (6)].pnode)->child = (yyvsp[(4) - (6)].pnode);
		(yyvsp[(4) - (6)].pnode)->next = (yyvsp[(6) - (6)].pnode);
		(yyvsp[(1) - (6)].pnode)->tail = (yyvsp[(6) - (6)].pnode);
		(yyval.pnode) = (yyvsp[(1) - (6)].pnode);
	;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 967 "psycon.y"
    {
		(yyval.pnode) = (yyvsp[(2) - (3)].pnode);
		(yyval.pnode)->type = N_INITCELL;
		(yyval.pnode)->line = (yyloc).first_line;
		(yyval.pnode)->col = (yyloc).first_column;
	;}
    break;



/* Line 1455 of yacc.c  */
#line 3883 "psycon.tab.c"
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
#line 989 "psycon.y"


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
