#ifndef LIST_H
#define LIST_H

typedef struct node
{
	char *name;
	struct node *next;
}List;

List* search(List *h,char *x);
int GetLength(List* h);
void Delet(List* head,char* ch);
void AppendId(List *head,char* buf);
List* CreateList(); /*����������ĺ���*/

#endif
