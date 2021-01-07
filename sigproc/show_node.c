#include <stdio.h>
//#include "psycon.yacc.h"
#include "psycon.tab.h"

void print_node_offset(FILE *fp, int offset)
{
	for (int k=0; k<offset; k++)
		fprintf(fp," ");
}

void print_node(FILE *fp, AstNode *p)
{
	if (!p) return;
	fprintf(fp,"[%d]", p->type);
	switch(p->type)
	{
	case T_ID:
	case N_STRUCT:
	case T_STRING:
		fprintf(fp,"%s", p->str);
		break;
	case T_NUMBER:
		fprintf(fp,"%f", p->dval);
		break;
	}
}

void print_node1(FILE *fp, AstNode *p)
{ // child node
	if (p->child)
	{
		fprintf(fp, "(C)");
		if (p->child->type==N_ARGS)
			fprintf(fp,"N_ARGS ");
	}
	print_node(fp, p->child);
	fprintf(fp,"\n");	
}

void print_node2(FILE *fp, AstNode *p)
{ // alt node
	fprintf(fp,"\n");	
	if (p->alt)
	{
		fprintf(fp, "(A)");
		if (p->alt->type==N_ARGS)
			fprintf(fp,"N_ARGS ");
	}
	print_node(fp, p->alt);
	fprintf(fp,"\n");	
}

void print_node3(FILE *fp, AstNode *p)
{ // next node
	fprintf(fp,"\n");	
	if (p->next)
	{
		fprintf(fp, "(N)");
		if (p->next->type==N_ARGS)
			fprintf(fp,"N_ARGS ");
	}
	print_node(fp, p->next);
	fprintf(fp,"\n");	
}

void show_node(const char *fname, AstNode *pnode)
{
	FILE *fp = fopen(fname, "at");
	print_node(fp,pnode);
	print_node1(fp,pnode);
	print_node2(fp,pnode);
	print_node3(fp,pnode);
	fclose(fp);
}