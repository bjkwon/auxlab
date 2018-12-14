#ifndef LQSYNYACC
#define LQSYNYACC
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef struct AstNode_t {
  int line, type;
  unsigned col;
  double dval;
  char *str;
  struct AstNode_t *tail;
  struct AstNode_t *next;
} AstNode;

extern int yydebug;

int yyPrintf(const char *msg, AstNode *p);
int getTokenID(const char *str);
char *getAstNodeName(AstNode *p);
int yyparse(AstNode **pproot, char **errmsg);
int yydeleteAstNode(AstNode *p, int fSkipNext);

int yysetNewStringToScan(const char *source);

#ifdef __cplusplus
}
#endif 