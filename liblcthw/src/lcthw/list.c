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
	check(node, "node can't be NULL");

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

