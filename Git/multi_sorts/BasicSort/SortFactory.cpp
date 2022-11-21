//
// Created by chengjinliang2 on 2022/11/14.
//
#include <vector>
#include "SortFactory.h"

/**
 * 1.选择排序
 * */
void selection_sort(int a[], int n) {
    for (int i = 1; i < n; ++i) {
        int ith = i;
        for (int j = i + 1; j <= n; ++j) {
            if (a[j] < a[ith]) {
                ith = j;
            }
        }
        std::swap(a[i], a[ith]);
    }
}

/**
 * 2.归并排序
 * */
void merge(int array[], int start, int mid, int end);
void merge_sort(int array[], int start, int end)
{
    int mid = 0;
    if(start < end)
    {
        mid = (start + end) / 2;
        merge_sort(array, start, mid);
//        cout<<"\n start="<<start <<" mid="<<mid << " ";
//        for(int i = 0; i < mid; i++) cout<<"array="<<array[i]<<" ";
        merge_sort(array, mid + 1, end);
//        cout<<"\n mid+1="<<mid+1 <<" end="<<end << " ";
//        for(int i = mid + 1; i < end; i++) cout<<"array="<<array[i]<<" ";
        merge(array, start, mid, end);
//        cout<<"\n start="<<start <<" end="<<end << " ";
//        for(int i = start; i < end; i++) cout<<"array="<<array[i]<<" ";
    }
}

void merge(int array[], int start, int mid, int end) // start, mid, end all are Array's index
{
    int l1 = mid - start + 1;//有序数组的长度
    int l2 = end - mid;//有序数组的长度

    //申请新数组
    int left[l1];
    int right[l2];

    //为申请的新数组赋值
    for(int i = 0; i < l1; ++i)
    {
        left[i] = array[start + i];
    }
    for(int j = 0; j < l2; ++j)
    {
        right[j] = array[mid + 1 + j];
    }

    //merge:the most important area
    int i = 0, j = 0;
    //先完成其中一个数组到新有序数组的转移
    while(i < l1 && j < l2)
    {
        if(left[i] <= right[j])
        {
            array[start++] = left[i++];
        }else{
            array[start++] = right[j++];
        }
    }

    //将有剩下的数组，一次性转移到新数组
    while(i < l1)
    {
        array[start++] = left[i++];
    }

    while(j < l2)
    {
        array[start++] = right[j++];
    }
}

/**
 * 3.快速排序
 */
void quick_sort(int s[], int l, int r)
{
    if (l< r)
    {
        int i = l, j = r, x = s[l];
        while (i < j)
        {
            while(i < j && s[j]>= x) // 从右向左找第一个小于x的数
                j--;
            if(i < j)
                s[i++] = s[j];
            while(i < j && s[i]< x) // 从左向右找第一个大于等于x的数
                i++;
            if(i < j)
                s[j--] = s[i];
        }
        s[i] = x;
        quick_sort(s, l, i - 1); // 递归调用
        quick_sort(s, i + 1, r);
    }
}

/**
 * 4.希尔排序
 *
 * 参数说明：
 *     a -- 待排序的数组
 *     n -- 数组的长度
 */
void shell_sort(int a[], int n)
{
    int i,j,gap;

    // gap为步长，每次减为原来的一半。
    for (gap = n / 2; gap > 0; gap /= 2)
    {
        // 共gap个组，对每一组都执行直接插入排序
        for (i = 0 ;i < gap; i++)
        {
            for (j = i + gap; j < n; j += gap)
            {
                // 如果a[j] < a[j-gap]，则寻找a[j]位置，并将后面数据的位置都后移。
                if (a[j] < a[j - gap])
                {
                    int tmp = a[j];
                    int k = j - gap;
                    while (k >= 0 && a[k] > tmp)
                    {
                        a[k + gap] = a[k];
                        k -= gap;
                    }
                    a[k + gap] = tmp;
                }
            }
        }

    }
}

/**
 * 5.基数排序
 * */
void radix_sort(int arr[], int length) {
    if (length <= 1) return;
    int max_item = arr[1];
    for (int i = 1; i < length; i++) {
        max_item = max_item > arr[i] ? max_item : arr[i];
    }
    int len = 1;
    while (max_item /= 10)
    {
        len++;   // 计算最大的数的位数
    }

    std::vector<std::vector<int> > buckets(10);
    int tmp = len;
    while (tmp--)   // 对每一个位进行桶排序，从个位开始
    {
        for (int i = 0; i < length; i++) {
            // arr[]中的数，按照各个位数进行排序
            int tmp_item = arr[i];
            int r;
            for (int j = 0; j < len - tmp; j++) {
                r = tmp_item % 10;
                tmp_item /= 10;
            }
            buckets[r].push_back(std::move(arr[i]));  // 每个桶里面可以放多个数
        }
        int idx = 0;
        for (std::vector<int>& bucket : buckets) {
            for (int& x : bucket) {
                arr[idx++] = std::move(x);
            }
            bucket.clear();
        }  // 按照桶排序的顺序拿出来，这里是升序，降序就反过来拿出来
    }
}


