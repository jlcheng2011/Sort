#include <iostream>
#include "SortFactory.h"

#define ELEMENT_COUNT 100

// 生成随机数
int rand(int L, int R) {
    int RND = rand(); // 使用 rand() 生成随机数即可

    return RND % (R - L + 1) + L;
}

int main() {
    int count= ELEMENT_COUNT;
    int array[count], array2[count];
    for(int i = 0; i < count; i ++) {
        array[i] =  rand(1, count); // 输入被排序的序列
        array2[i] =  array[i]; /// 输入被排序的序列
    }

//    selection_sort(array, count);
    merge_sort(array, 0, count);
//    quick_sort(array, 0, count - 1);
//    shell_sort(array, count);
//    radix_sort(array, count);


    for(int i = 0; i < count; i ++) {
        printf("%d\n", array[i]);
    }

    return 0;
}
