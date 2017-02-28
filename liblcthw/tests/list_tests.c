#include "minunit.h"
#include <lcthw/list.h>
#include <assert.h>

static List* list = NULL;
char* test1 = "test1 data";
char* test2 = "test2 data";
char* test3 = "test3 data";
char* test4 = "test4 data";
char* test5 = "test5 data";
char* test6 = "test6 data";

char* test_create()
{
	list = List_create();
	mu_assert(list != NULL, "Failed to create list.");
	return NULL;
}

char* test_destroy()
{
	List_clear_destroy(list);
	return NULL;
}

char* test_push_pop()
{
	List_push(list, test1);
	mu_assert(List_last(list) == test1, "Wrong last value.");

	List_push(list, test2);
	mu_assert(List_last(list) == test2, "Wrong last value.");

	List_push(list, test3);
	mu_assert(List_last(list) == test3, "Wrong last value.");

	mu_assert(List_count(list) == 3, "Wrong count on push.");

	char* val = List_pop(list);
	mu_assert(val == test3, "Wrong value on pop.");

	val = List_pop(list);
	mu_assert(val == test2, "Wrong value on pop.");

	val = List_pop(list);
	mu_assert(val == test1, "Wrong value on pop.");
	mu_assert(List_count(list) == 0, "Wrong count after pop.");

	return NULL;
}

char* test_unshift()
{
	List_unshift(list, test1);
	mu_assert(List_first(list) == test1, "Wrong first value.");

	List_unshift(list, test2);
	mu_assert(List_first(list) == test2, "Wrong first value");

	List_unshift(list, test3);
	mu_assert(List_first(list) == test3, "Wrong last value.");
	mu_assert(List_count(list) == 3, "Wrong count on unshift.");

	return NULL;
}

char* test_remove()
{
	// we only need to test the middle remove case since push/shift
	// already tests the other cases
	
	char* val = List_remove(list, list->first->next);
	mu_assert(val == test2, "Wrong removed element.");
	mu_assert(List_count(list) == 2, "Wrong count after remove.");
	mu_assert(List_first(list) == test3, "Wrong first after remove.");
	mu_assert(List_last(list) == test1, "Wrong last after remove.");

	return NULL;
}

char* test_shift()
{
	mu_assert(List_count(list) != 0, "Wrong count before shift.");

	char* val = List_shift(list);
	mu_assert(val == test3, "Wrong value on shift.");

	val = List_shift(list);
	mu_assert(val == test1, "Wrong value on shift.");
	mu_assert(List_count(list) == 0, "Wrong count after shift.");

	return NULL;
}

char* test_print()
{
	// test print empty list
	List_print(list);

	// test print full list
	List_push(list, test1);
	List_push(list, test2);
	List_push(list, test3);
	List_print(list);

	return NULL;
}

char* test_duplicate()
{
	List_push(list, test1);
	List_push(list, test2);
	List_push(list, test3);

	List* dup = List_duplicate(list);

	mu_assert((int)List_count(list) == (int)List_count(dup), "Mismatched list lengths.");
	mu_assert(list->first->value == dup->first->value, "First valueue doesn't match.");
	mu_assert(list->first->next->value == dup->first->next->value, "Second value doesn't match.");
	mu_assert(list->last->value == dup->last->value, "Last valueue doesn't match.");

	List_clear_destroy(dup);

	return NULL;
}

char* test_join()
{
	List* tail = List_create();
	List_push(tail, test4);
	List_push(tail, test5);
	List_push(tail, test6);

	List* result = List_join(tail, list);

	int sum = (int)List_count(tail) + (int)List_count(list);

	mu_assert((int)List_count(result) == sum, "Mismatched lengths");

	List_clear_destroy(tail);
	List_clear_destroy(result);

	return NULL;
}

char* all_tests()
{
	mu_suite_start();

	mu_run_test(test_create);
	mu_run_test(test_push_pop);
	mu_run_test(test_unshift);
	mu_run_test(test_remove);
	mu_run_test(test_shift);
	mu_run_test(test_duplicate);
	mu_run_test(test_print);
	mu_run_test(test_join);
	mu_run_test(test_destroy);

	return NULL;
}

RUN_TESTS(all_tests);
