#ifndef PSYCONYACC
#define PSYCONYACC
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define N_BLOCK		10000
#define N_ARGS		10001
#define N_MATRIX		10002
#define N_VECTOR		10003
#define N_CALL		10004
#define N_STRUCT		10005
#define N_IDLIST		10006
#define N_TIME_EXTRACT	10007
#define N_CELLASSIGN	10008
#define N_IXASSIGN	10009
#define N_INITCELL	10012
#define N_CELL		10015
#define N_HOOK		10016
#define N_TSEQ		10017

typedef struct AstNode_t {
	int line, type;
	unsigned col;
	double dval;
	char *str;
	int suppress;
	struct AstNode_t* child;
	struct AstNode_t* alt;
	struct AstNode_t* tail;
	struct AstNode_t* next;
} AstNode;

extern int yydebug;

int yyPrintf(const char *msg, AstNode *p);
int getTokenID(const char *str);
char *getAstNodeName(AstNode *p);
int yyparse(AstNode **pproot, char **errmsg);
int yydeleteAstNode(AstNode *p, int fSkipNext);

int yysetNewStringToScan(const char *source);
int yysetNewFileToScan(FILE *source);

#ifdef __cplusplus
}
#endif 