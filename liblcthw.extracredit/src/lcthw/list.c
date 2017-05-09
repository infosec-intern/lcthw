#include <lcthw/list.h>
#include <lcthw/dbg.h>

List* List_create()
{
	return calloc(1, sizeof(List));
}

void List_destroy(List* list)
{
	LIST_FOREACH(list, first, next, cur) {
		if (cur->prev) {
			free(cur->prev);
		}
	}
	free(list->last);
	free(list);
}

void List_clear(List* list)
{
	LIST_FOREACH(list, first, next, cur) {
		free(cur->value);
	}
}

void List_clear_destroy(List* list)
{
	List_clear(list);
	List_destroy(list);
}

void List_push(List* list, void* value)
{
	check(list, "Can't push to a NULL list");
	check(value != NULL, "List_push: value cannot be NULL");

	// set node to the last element in the list
	ListNode* node = calloc(1, sizeof(ListNode));
	check_mem(node);

	node->value = value;

	if (list->last == NULL) {
		// if length of list is 0, create first entry in list
		list->first = node;
		list->last = node;
	} else {
		// first link end of list to node, then node to end of list, then set node to last
		list->last->next = node;
		node->prev = list->last;
		list->last = node;
	}

	list->count++;

	// fallthrough
error:
	return;
}

void* List_pop(List* list)
{
	// remove last element in list
	ListNode* node = list->last;
	return node != NULL ? List_remove(list, node) : NULL;
}

void List_unshift(List* list, void* value)
{
	check(list, "Can't unshift a NULL list");
	check(value != NULL, "List_unshift: value cannot be NULL");

	// set node to the first element in the list
	ListNode* node = calloc(1, sizeof(ListNode));
	check_mem(node);

	node->value = value;

	if (list->first == NULL) {
		// if length of list is 0, create first entry in list
		list->first = node;
		list->last = node;
	} else {
		// first link node to list, then list to node, then set node to first
		node->next = list->first;
		list->first->prev = node;
		list->first = node;
	}

	list->count++;

	// fallthrough
error:
	return;
}

void* List_shift(List* list)
{
	// remove first element in list
	ListNode* node = list->first;
	return node != NULL ? List_remove(list, node) : NULL;
}

void* List_remove(List* list, ListNode* node)
{
	void* result = NULL;

	check(list->first && list->last, "List is empty.");
	check(node, "List_remove: node can't be NULL");

	if (node == list->first && node == list->last) {
		// if node is only element in list, zero it out
		list->first = NULL;
		list->last = NULL;
	} else if (node == list->first) {
		// if node is first element in list, move beginning up one
		list->first = node->next;
		check(list->first != NULL, "Invalid list, somehow got a first that is NULL.");
		list->first->prev = NULL;
	} else if (node == list->last) {
		// if node is last element in list, move end down one
		list->last = node->prev;
		check(list->last != NULL, "Invalid list, somehow got a next that is NULL.");
		list->last->next = NULL;
	} else {
		// else link surrounding nodes together
		ListNode* after = node->next;
		ListNode* before = node->prev;
		after->prev = before;
		before->next = after;
	}

	list->count--;
	result = node->value;
	free(node);

	// fallthrough
error:
	return result;
}

/*-- CUSTOM FUNCTIONS BELOW --*/

List* List_duplicate(List* src)
{
	// Copy src list into dst list
	check(src, "Can't duplicate a NULL list");
	
	List* dst = List_create();

	LIST_FOREACH(src, first, next, cur) {
		List_push(dst, cur->value);
	}
	return dst;

error:
	return NULL;
}

List* List_join(List* src, List* dst)
{
	// Join two lists together with src becoming the tail of dst
	check(src->first->value != NULL, "Can't join NULL to list");
	check(dst->first->value != NULL, "Can't join list to NULL");

	List* head = List_duplicate(dst);
	check(head, "Error duplicating list head");
	List* tail = List_duplicate(src);
	check(tail, "Error duplicating list tail");

	head->last->next = tail->first;
	tail->first->prev = head->last;

	// manually bump list length up
	int i = 0;
	for(i = 0; i < List_count(tail); i++)
		head->count++;

	return head;
error:
	return NULL;
}

List** List_split(List* list, char* sentinel)
{
	// Split the list into several lists at value
	// Converts sentinel into an integer 
	check(list, "Can't split a NULL list");
	List** set;
	int i = 0;
	ListNode* node = calloc(1, sizeof(ListNode));

	LIST_FOREACH(list, first, next, cur) {
		// still not safe - will fix later
		if (strncmp(cur->value, sentinel, 20) == 0) {
			// generate a new list
			*(set + i) = List_create();
			// set the current node as the last link in our list
			// sentinel is the end of our split
			(*(set + i))->last = cur;
			// set beginning of current list to temporary node
			(*(set + i))->first = node;
			// use a temporary node to save the beginning of our next list
			node = cur->next;
			// modify list count and set count
			(*(set + i))->count++;
			i++;
		}
	}
	
	int j = 0;
	for (j = 0; j < i; j++) {
		printf("List %d: ", i);
		List_print(*(set + i));
	}

	return set;
error:
	return NULL;
}

void List_reverse(List* list)
{
	// Reverse the list in-place
	return NULL;
}

void List_print(List* list)
{
	// Print a diagram of the linked list
	if (list) {
		LIST_FOREACH(list, first, next, cur) {
			printf("[%s] -> ", cur->value);
		}
	}
	// end on NULL or print only NULL for empty
	printf("[NULL]\n");
}
