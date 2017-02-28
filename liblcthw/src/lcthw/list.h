#ifndef lcthw_List_h
#define lcthw_List_h

#include <stdlib.h>

struct ListNode;

// element in the linked list
typedef struct ListNode {
	struct ListNode* next;
	struct ListNode* prev;
	void* value;
} ListNode;

// container for linked ListNode structs
typedef struct List {
	int count;			// cannot be < 0
	ListNode* first;	// cannot be NULL when count > 0
	ListNode* last;
} List;

List* List_create();
void List_destroy(List* list);
void List_clear(List* list);
void List_clear_destroy(List* list);

#define List_count(A) ((A)->count >= 0 ? (A)->count : NULL)		// count invariant ?
#define List_first(A) ((A)->first != NULL && (A)->count > 0 ? (A)->first->value : NULL)
#define List_last(A) ((A)->last != NULL ? (A)->last->value : NULL)

void List_push(List* list, void* value);
void* List_pop(List* list);
void List_unshift(List* list, void* value);
void* List_shift(List* list);
void* List_remove(List* list, ListNode* node);
List* List_duplicate(List* src);
List* List_join(List* src, List* dst);
List** List_split(List* list, char* sentinel);
void List_reverse(List* list);
void List_print(List* list);

#define LIST_FOREACH(L, S, M, V) ListNode* _node = NULL;\
												   ListNode* V = NULL;\
for(V = _node = L->S; _node != NULL; V = _node = _node->M)

#endif
