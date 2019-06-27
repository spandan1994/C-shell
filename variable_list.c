
#include "variable_list.h"


LIST *CreateLIST(void)
{
	LIST *newlist = (LIST *)malloc(sizeof(LIST));
	newlist->head = NULL;
	return newlist;
}

NODE *MakeNODE(char **name)
{
	NODE *newnode = (NODE *)malloc(sizeof(NODE));
	int i;
	for(i = 0 ; i < 40 ; i++) newnode->variable[i] = NULL;
	for(i = 0 ; name[i] != NULL ; i++)
	{
		newnode->variable[i] = strdup(name[i]);
	}
	newnode->next = NULL;
	return newnode;
}

void pushFRONT(LIST *l, NODE *n)
{
	n->next = l->head;
	l->head = n;	
}

NODE *Search_by_var(LIST *l, char *name)
{
	NODE *temp = l->head;
	while(temp != NULL)
	{	
		if( strcmp( (temp->variable[0]) , name ) == 0 )
		{
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}

int Search_and_replace(LIST *l, char **name)
{
	int i;
	NODE *temp = l->head;
	while(temp != NULL)
	{
		if( strcmp( (temp->variable[0]) , name[0] ) == 0 )
		{
			for(i = 0 ; i < 40 ; i++) temp->variable[i] = NULL;
			for(i = 0 ; name[i] != NULL ; i++)
			{
				temp->variable[i] = strdup(name[i]);
			}
			return 1;
		}
		temp = temp->next;
	}
	return 0;
}

void Free_LIST(LIST *l)
{
	NODE *temp = l->head;
	while(temp != NULL)
	{
		l->head = temp->next;
		free(temp);
		temp = l->head;
	}
	free(l);
}
