#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#include "uthread.h"

static int seq_threshold = 100;

struct array {
	int *arr;
	int len;
};

void selection_sort(struct array *A)
{
	int *arr = A->arr;
	int length = A->len;

	int i, j, min, temp;
	for (i = 0; i < length - 1; ++i) {

		min = i;

		for (j = i + 1; j < length; ++j) {
			if (arr[j] < arr[min]) {
				min = j;
			}
		}

		temp = arr[i];
		arr[i] = arr[min];
		arr[min] = temp;
	}
}

void merge(struct array *A, struct array *B)
{
	int *arr1 = A->arr;
	int l1 = A->len;

	int *arr2 = B->arr;
	int l2 = B->len;

	int *result = malloc(sizeof(int) * (l1 + l2));

	int i = 0, j = 0, k = 0;

	while (i < l1 && j < l2) {
		if (arr1[i] < arr2[j]) {
			result[k++] = arr1[i++];
		} else {
			result[k++] = arr2[j++];
		}

		uthread_yield();
	}

	if (i >= l1) {
		memcpy(result + k, arr2 + j, sizeof(int) * (l2 - j));
	} else if (j >= l2) {
		memcpy(result + k, arr1 + i, sizeof(int) * (l1 - i));
	}

	memcpy(arr1, result, sizeof(int) * (l1 + l2));

	free(result);
}

int par_mergesort(void *arg)
{
	struct array *A = (struct array *)arg;

	if (A->len <= seq_threshold) {
		selection_sort(A);
	}

	else {
		struct array left_half, right_half;

		if (A->len % 2 == 0) {
			left_half.len = right_half.len = A->len / 2;
		} else {
			left_half.len = A->len / 2;
			right_half.len = A->len / 2 + 1;
		}

		left_half.arr = A->arr;
		right_half.arr = A->arr + left_half.len;

		struct uthread_attr a = UT_ATTR(UT_DEFER, 1);
		tid_t left_t = uthread_create(a, par_mergesort, &left_half);
		tid_t right_t = uthread_create(a, par_mergesort, &right_half);

		assert(left_t);
		assert(right_t);

		assert(uthread_join(left_t, NULL) == 0);
		assert(uthread_join(right_t, NULL) == 0);

		merge(&left_half, &right_half);
	}

	return 0;
}

struct array *rand_array(int size)
{
	struct array *result = malloc(sizeof(struct array));
	result->arr = malloc(sizeof(int) * size);
	result->len = size;

	int i;
	srand(time(NULL));
	for (i = 0; i < size; ++i) {
		result->arr[i] = rand() % size;
	}
	return result;
}

const char *check_sort(struct array *A)
{
	int i, is_sorted = 1;
	for (i = 0; i < A->len - 1; ++i) {
		if (A->arr[i] > A->arr[i + 1]) {
			is_sorted = 0;
			break;
		}
	}
	return is_sorted ? "sorted!" : "not sorted!";
}

int main(int argc, char *argv[])
{
	int n;
	struct array *A;

	if (argc < 2)
		n = 200;
	else
		n = atoi(argv[1]);

	A = rand_array(n);

	printf("before sort: %s\n", check_sort(A));
	par_mergesort(A);
	printf("after sort: %s\n", check_sort(A));

	return 0;
}
