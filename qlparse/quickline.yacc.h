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

extern int qqdebug;

int qqPrintf(const char *msg, AstNode *p);
int getTokenID(const char *str);
char *getAstNodeName(AstNode *p);
int qqparse(AstNode **pproot, char **errmsg);
int qqdeleteAstNode(AstNode *p, int fSkipNext);

int qqsetNewStringToScan(const char *source);

#ifdef __cplusplus
}
#endif 