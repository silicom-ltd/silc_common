/*
 * silc_list.h
 *
 *  Created on: Nov 23, 2010
 *      Author: jeff_zheng
 *   Copyright: NetPerform Technology
 */

#ifndef __SILC_LIST_H
#define __SILC_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct silc_list_node_s {
	struct silc_list_node_s *next;
	struct silc_list_node_s *prev;
}silc_list_node, silc_list;



#define SILC_LIST_INIT(list)	silc_list_init(&(list))

static inline void silc_list_init(silc_list* p_list)
{
	p_list->next = (silc_list_node*)p_list;
	p_list->prev = (silc_list_node*)p_list;
}

/**
 * silc_list_add - add a new entry
 * @new_node: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified node. Good for stacks
 */
static inline void silc_list_add(silc_list_node *new_node, silc_list_node *p_node)
{
	silc_list_node* p_next = p_node->next;

	p_next->prev = new_node;
	new_node->next = p_next;
	new_node->prev = p_node;
	p_node->next = new_node;
}

/**
 * silc_list_add_tail - add a new_node entry
 * @new_node: new_node entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void silc_list_add_tail(silc_list_node *new_node, silc_list *p_list)
{
	silc_list_add(new_node, p_list->prev);
}

static inline void silc_list_add_head(silc_list_node *new_node, silc_list *p_list)
{
	silc_list_add(new_node, (silc_list_node*)p_list);
}

/*
 * Delete a list entry from node, but don't zero out the original node,
 * internal use only for slightly better performance
 */
static inline void __silc_list_del(silc_list_node *prev, silc_list_node *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * silc_list_del - deletes entry from list.
 * p_node: the element to delete from the list.
 */
static inline void silc_list_del(silc_list_node *p_node)
{
	__silc_list_del(p_node->prev, p_node->next);
	p_node->next = (silc_list_node *) NULL;
	p_node->prev = (silc_list_node *) NULL;
}

#if 0
/**
 * silc_list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void silc_list_del_init(silc_list_node *entry)
{
	__silc_list_del(entry->prev, entry->next);
	SILC_INIT_LIST_HEAD(entry);
}
#endif
/**
 * silc_list_move - delete a node from one list and add as another's head
 * @node: the entry to move
 * @p_dest_node: the head that will precede our entry
 */
static inline void silc_list_move_node(silc_list_node *node, silc_list_node *p_dest_node)
{
	__silc_list_del(node->prev, node->next);
	silc_list_add(node, p_dest_node);
}

/**
 * silc_list_move_tail - delete from one list and add as another's tail
 * @node: the entry to move
 * @p_list: the head that will follow our entry
 */
static inline void silc_list_move_node_tail(silc_list_node *node, silc_list *p_list)
{
	__silc_list_del(node->prev, node->next);
	silc_list_add_tail(node, p_list);
}

/**
 * silc_list_empty - test whether a list is empty or not
 * @p_list: the list to test.
 */
static inline int silc_list_empty(silc_list *p_list)
{
	return p_list->next == (silc_list_node*)p_list;
}

/**
 * silc_list_join - join two lists together, all the elements in plist
 *                will be put onto the second list, and p_list will be reinitialized
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void silc_list_join(silc_list *p_list, silc_list_node *head)
{
	if (silc_list_empty(p_list))
		return;

	silc_list_node *begin = p_list->next;
	silc_list_node *end = p_list->prev;
	silc_list_node *next_node = head->next;

	begin->prev = head;
	head->next = begin;

	end->next = next_node;
	next_node->prev = end;

	silc_list_init(p_list);

}


/**
 * silc_list_entry - get the struct for this entry
 * @ptr:	the &silc_list_node pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define silc_list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * silc_list_for_each	-	iterate over a list
 * @pos:	the &silc_list_node to use as a loop counter.
 * @head:	the head for your list.
 */
#define silc_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (silc_list_node*)(head); \
			pos = pos->next)
/**
 * silc_list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &silc_list_node to use as a loop counter.
 * @head:	the head for your list.
 */
#define silc_list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (silc_list_node*)(head); \
			pos = pos->prev)

/**
 * silc_list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &silc_list_node to use as a loop counter.
 * @n:		another &silc_list_node to use as temporary storage
 * @head:	the head for your list.
 */
#define silc_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (silc_list_node*)(head); \
		pos = n, n = pos->next)

/**
 * silc_list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop counter.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define silc_list_for_each_entry(pos, head, member)				\
	for (pos = silc_list_entry((head)->next, __typeof__(*pos), member);	\
		 pos != NULL && &pos->member != (silc_list_node*)(head); 					\
		 pos = silc_list_entry(pos->member.next, __typeof__(*pos), member))

/**
 * silc_list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop counter.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define silc_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = silc_list_entry((head)->next, __typeof__(*pos), member),	\
		n = silc_list_entry(pos->member.next, __typeof__(*pos), member);	\
		 &pos->member != (silc_list_node*)(head); 					\
		 pos = n, n = silc_list_entry(n->member.next, __typeof__(*n), member))

#define silc_list_dup(head_dst, head_src, member, type, dup_func, success_ret) \
	do{\
	type * _loop_src, *dup_dst; \
	success_ret = silc_true; \
	silc_list_for_each_entry(_loop_src, head_src, member) \
	{ \
		dup_dst = dup_func(_loop_src); \
		if(dup_dst==NULL) \
		{ \
			success_ret = silc_false; \
			break; \
		} \
		silc_list_add_tail(&dup_dst->member, head_dst); \
	}\
	}while(0);

typedef int (*silc_list_node_cmp_cbf_t)(silc_list_node* p_node1, silc_list_node* p_node2);

static inline void silc_list_qsort_inner(silc_list* p_list, silc_list_node_cmp_cbf_t p_cbf)
{
	silc_list    list_less, list_greater;
	silc_list_node    *p_node,*p_node_middle,*p_tmp_node;

	int cmp_ret;

	/*if we have 1 or less than 1 element, we return*/
	if(silc_list_empty(p_list))
		return;
	if(p_list->next->next==(silc_list_node*)p_list)
		return;

	silc_list_init(&list_less);
	silc_list_init(&list_greater);

	/* select a pivot value pivot from list*/
	p_node_middle = p_list->next;
	silc_list_for_each_safe(p_node,p_tmp_node,p_list)
	{
		cmp_ret = p_cbf(p_node,p_node_middle);
		if(cmp_ret<0)
		{
			silc_list_del(p_node);
			silc_list_add_tail(p_node,&list_less);
		}
		else if(cmp_ret>0)
		{
			silc_list_del(p_node);
			silc_list_add_tail(p_node,&list_greater);
		}
	}

	silc_list_qsort_inner(&(list_less),p_cbf);
	silc_list_qsort_inner(&(list_greater),p_cbf);

	silc_list_join(&(list_greater),p_list->prev);
	silc_list_join(&(list_less),(silc_list_node*)p_list);
	return;
}

static inline void silc_list_qsort(silc_list* p_list_head, silc_list_node_cmp_cbf_t p_cbf)
{
	if(p_list_head==NULL)
		return;

	silc_list_qsort_inner(p_list_head, p_cbf);
	return;
}


/*
 * Simple sinle linked list implementation.
 * this uses less space than the double linked one. However the silc_slist_head size is the same as a silc_list_head
 * del_tail is not available, as it is quite expensive.
 */

typedef struct silc_slist_node_s {
	struct silc_slist_node_s *next;
}silc_slist_node;

typedef struct silc_slist_s {
	silc_slist_node*	first;
	silc_slist_node* 	last;
}silc_slist;


#define SILC_SLIST_INIT(list) silc_slist_init(&(list))

static inline void silc_slist_init(silc_slist* slist)
{
	slist->first = NULL;
	slist->last = NULL;
}

/**
 * silc_slist_add - add a new entry
 * @new_node: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void silc_slist_add_after(silc_slist_node *new_node, silc_slist_node *add_after_node, silc_slist* p_list_head)
{
	silc_slist_node* p_tmp;
	p_tmp = add_after_node->next;
	add_after_node->next = new_node;
	new_node->next = p_tmp;
}

/**
 * silc_slist_add_head - add a new entry to the front of the list
 * @new_node: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void silc_slist_add_head(silc_slist_node *p_new_node, silc_slist* p_list_head)
{
	silc_slist_node* p_tmp;
	if(p_list_head->first)
	{
		p_tmp = p_list_head->first;
		p_list_head->first = p_new_node;
		p_new_node->next = p_tmp;
	}
	else
	{
		p_list_head->first = p_new_node;
		p_list_head->last = p_new_node;
		p_new_node->next = NULL;
	}
}

/**
 * silc_slist_add_tail - add a new entry to the end of the list
 * @new_node: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void silc_slist_add_tail(silc_slist_node *p_new_node, silc_slist* p_list_head)
{
/*    silc_slist_node* p_tmp;*/
	if(p_list_head->last)
	{
		p_list_head->last->next = p_new_node;
		p_list_head->last = p_new_node;
		p_new_node->next = NULL;
	}
	else
	{
		p_list_head->first = p_new_node;
		p_list_head->last = p_new_node;
		p_new_node->next = NULL;
	}
}


/**
 * silc_slist_del_head - Delete the first entry from a list and return it
 * @head: list head to delete from
 *
 * NULL is returned if the list is empty
 */
static inline silc_slist_node* silc_slist_del_head(silc_slist* p_list_head)
{
	silc_slist_node* p_tmp;

	p_tmp = p_list_head->first;
	if(p_tmp)
	{
		p_list_head->first = p_tmp->next;
		if(p_tmp->next==NULL)
		{
			p_list_head->last = NULL;
		}

		p_tmp->next = NULL;
	}

	return p_tmp;
}

/**
 * silc_slist_del - deletes entry from list.
 * @entry_del: the element to delete from the list.
 * @prev_node: prev_node of entry_del in the list, should be NULL if entry_del is the first node of the list.
 * @p_list_head: list head to delete from
 * Note: silc_slist_empty on entry does not return true after this, the entry is in an undefined state.
 */
static inline void silc_slist_del(silc_slist_node *entry_del, silc_slist_node* prev_node, silc_slist* p_list_head)
{
	if(prev_node)
	{/*not delete from head*/
		prev_node->next = entry_del->next;
	}
	else
	{/*deleting from head*/
		p_list_head->first = entry_del->next;
	}
	entry_del->next = NULL;

	if(entry_del==p_list_head->last)
	{/*deleting the last node*/
		p_list_head->last=prev_node;
	}
}

/**
 * silc_slist_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int silc_slist_empty(silc_slist *head)
{
	return head->first == NULL;
}

/**
 * silc_slist_splice_after - join two lists together, the first list will be append to the second, and it will be emptied
 * @list_to_append: the new list to add.
 * @dest: the place to add it in the first list.
 */
static inline void silc_slist_splice_after(silc_slist *list_to_append, silc_slist *dest)
{
	if(!silc_slist_empty(list_to_append))
	{
		if(dest->last==NULL)
		{
			dest->first = list_to_append->first;
			dest->last = list_to_append->last;
		}
		else
		{
			dest->last->next = list_to_append->first;
			dest->last = list_to_append->last;
		}
		list_to_append->first = NULL;
		list_to_append->last = NULL;
	}
}

/**
 * silc_slist_splice_before - join two lists together, the first list will be inserted at the front of the second, and it will be emptied
 * @list_to_insert: the new list to add.
 * @dest: the place to add it in the first list.
 */
static inline void silc_slist_splice_before(silc_slist *list_to_insert, silc_slist *dest)
{
	if(!silc_slist_empty(list_to_insert))
	{
		if(dest->first==NULL)
		{
			dest->first = list_to_insert->first;
			dest->last = list_to_insert->last;
		}
		else
		{
			list_to_insert->last->next = dest->first;
			dest->first = list_to_insert->first;
		   // dest->last = list_to_insert->last;
		}
		list_to_insert->first = NULL;
		list_to_insert->last = NULL;
	}
}


/**
 * silc_slist_entry - get the struct for this entry
 * @ptr:    the &silc_slist pointer.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 */
#define silc_slist_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * silc_slist_for_each    -   iterate over a list
 * @pos:    the &silc_slist to use as a loop counter.
 * @head:   the head for your list.
 */
#define silc_slist_for_each(pos, head) \
	for (pos = (head)->first; pos != NULL; pos = pos->next)

/**
 * silc_slist_for_each_safe   -   iterate over a list safe against removal of list entry
 * @pos:    the &silc_slist to use as a loop counter.
 * @n:      another &silc_slist to use as temporary storage
 * @head:   the head for your list.
 */
#define silc_slist_for_each_safe(pos, n, head) \
	for (pos = (head)->first, n = pos?pos->next:NULL; pos != NULL; \
		pos = n, n = pos?pos->next:NULL)

/**
 * silc_slist_for_each_entry  -   iterate over list of given type
 * @pos:    the type * to use as a loop counter.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define silc_slist_for_each_entry(pos, head, member)				\
	for ((pos) = ((head)->first)?(silc_slist_entry((head)->first, __typeof__(*pos), member)):NULL;  \
		 (pos) != NULL;                                                                       \
		 (pos) = (pos)->member.next?(silc_slist_entry((pos)->member.next, __typeof__(*pos), member)):NULL)

/**
 * silc_slist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop counter.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define silc_slist_for_each_entry_safe(pos, n, head, member)			\
	for (pos = ((head)->first)?(silc_slist_entry((head)->next, __typeof__(*(pos)), member)):NULL,  \
		n = ((pos)==NULL||(pos)->member.next==NULL)?NULL:silc_slist_entry((pos)->member.next, __typeof__(*(pos)), member); \
		 (pos) != NULL;                    \
		 (pos) = n, n = ((pos)==NULL||(pos)->member.next==NULL)?NULL:(silc_slist_entry(n->member.next, __typeof__(*n), member)))

typedef int (*silc_slist_node_cmp_cbf_t)(silc_slist_node* p_node1, silc_slist_node* p_node2);


static inline void silc_slist_qsort_inner(silc_slist* p_list, silc_slist_node_cmp_cbf_t p_cbf)
{
	silc_slist    list_less, list_greater;
	silc_slist_node    *p_node,*p_node_middle,*p_tmp_node, *p_tmp_prev_node;

	int cmp_ret;

	/*if we have 1 or less than 1 element, we return*/
	if(silc_slist_empty(p_list))
		return;
	if(p_list->first->next==NULL)
		return;

	silc_slist_init(&list_less);
	silc_slist_init(&list_greater);

	/* select a pivot value pivot from list*/
	p_node_middle = p_list->first;

	p_tmp_prev_node = NULL;
	silc_slist_for_each_safe(p_node,p_tmp_node,p_list)
	{
		cmp_ret = p_cbf(p_node,p_node_middle);
		if(cmp_ret<0)
		{
			silc_slist_del(p_node, p_tmp_prev_node, p_list);
			silc_slist_add_tail(p_node,&list_less);
			continue;
		}
		else if(cmp_ret>0)
		{
			silc_slist_del(p_node, p_tmp_prev_node, p_list);
			silc_slist_add_tail(p_node,&list_greater);
			continue;
		}
		p_tmp_prev_node = p_node;
	}

	silc_slist_qsort_inner(&(list_less),p_cbf);
	silc_slist_qsort_inner(&(list_greater),p_cbf);

	silc_slist_splice_before(&(list_greater), p_list);
	silc_slist_splice_after(&(list_less),p_list);
	return;
}

static inline void silc_slist_qsort(silc_slist* p_list_head, silc_slist_node_cmp_cbf_t p_cbf)
{
	if(p_list_head==NULL)
		return;

	silc_slist_qsort_inner(p_list_head, p_cbf);
	return;
}





#ifdef __cplusplus
}
#endif






#endif


