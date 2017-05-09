#include "list_algos.h"
#include <string.h>

List* List_merge_sort(List* list)
{
	return list;
}

int List_bubble_sort(List* list)
{
	LIST_FOREACH(list, first, next, cur) {
		if (cur->next && strcmp(cur->value, cur->next->value) > 0) {
			// if current > next
			// switch list nodes around
		}
	}
	return 0;
}
