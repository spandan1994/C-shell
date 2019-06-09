
#include "process_list.h"


list *Createlist(void)
{
	list *newlist = (list *)malloc(sizeof(list));
	newlist->head = NULL;
	return newlist;
}

node *Makenode(char *name, pid_t pid)
{
	node *newnode = (node *)malloc(sizeof(node));
	strcpy(newnode->p_name,name);
	newnode->status = 0;
	newnode->p_pid = pid;
	newnode->next = NULL;
	return newnode;
}

void pushfront(list *l, node *n)
{
	if(l->head == NULL) l->head = n;
	else
	{
		n->next = l->head;
		l->head = n;
	}
}

int Search_by_name(list *l, char *name)
{
	node *temp = l->head;
	while(temp != NULL)
	{
		if( strcmp( (temp->p_name) , name ) == 0 && temp->status == 0 )
		{
			temp->status = 1;
			return (temp->p_pid);
		}
		temp = temp->next;
	}
	return -1;
}

int Search_by_pid(list *l, pid_t pid)
{
	node *temp = l->head;
	while(temp != NULL)
	{
		if( (temp->p_pid) == pid )
		{
			return 0;
		}
		temp = temp->next;
	}
	return -1;
}

void Update(list *l)
{
	node *temp;
	while(l->head != NULL && l->head->status == 1)
	{
		temp = l->head;
		l->head = l->head->next;
		free(temp);
	}

	temp = l->head;
	while(temp != NULL)
	{
		while(temp->next != NULL && temp->next->status == 1)
		{
			node *temp1 = temp->next;
			temp->next = temp1->next;
			free(temp1);
		}
		temp = temp->next;
	}

}
