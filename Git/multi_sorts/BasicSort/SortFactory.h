//
// Created by chengjinliang2 on 2022/11/14.
//

#ifndef MULTI_SORTS_SORTFACTORY_H
#define MULTI_SORTS_SORTFACTORY_H


class SortFactory {

};

/**
 * 1.选择排序
 * */
void selection_sort(int a[], int n);

/**
 * 2.归并排序
 * */
void merge_sort(int array[], int start, int end);

/**
 * 3.快速排序
 * */
void quick_sort(int s[], int l, int r);

/**
 * 4.希尔排序
 *
 * */
void shell_sort(int a[], int n);

/**
 * 5.基数排序
 * */
void radix_sort(int arr[], int length);



#endif //MULTI_SORTS_SORTFACTORY_H
