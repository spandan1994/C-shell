
#include "process_list.h"


list *Createlist(void)
{
	list *newlist = (list *)malloc(sizeof(list));
	newlist->head = NULL;
	return newlist;
}

node *Makenode(char *name, pid_t pid, int jid)
{
	node *newnode = (node *)malloc(sizeof(node));
	strcpy(newnode->p_name,name);
	newnode->status = 0;
	newnode->p_pid = pid;
	newnode->job_id = jid;
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

int Search_by_name(list *l, char *name, int situ)
{
	node *temp = l->head;
	while(temp != NULL)
	{
		if( strcmp( (temp->p_name) , name ) == 0 && temp->status <= 0 )
		{
			if(situ) temp->status = 1;   //status=1 will remove process from the process list while updation.
			return (temp->p_pid);        //There should be a choice whether to change the status, e.g. bg and fg.
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
			return (temp->job_id);
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

void free_list(list *l)
{
        node *temp = l->head;
        while(temp != NULL)
        {
                l->head = temp->next;
                free(temp);
                temp = l->head;
        }
        free(l);
}

int change_status(list *l, int pid, int value)   //changes status of all processes having the job_id pid
{
	node *temp = l->head;
	int state = 0;
        while(temp != NULL)
        {
                if( temp->job_id == pid )
                {
			if( temp->status != value ) temp->status = value;
			state = 1;
                }
		else
		{
			if( state != 0 ) return 0;  //avoids unnecessary traversal of linked list
		}
                temp = temp->next;
        }
        return 0;
}
