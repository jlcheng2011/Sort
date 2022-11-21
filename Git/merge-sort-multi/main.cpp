#include <iostream>
#include <ctime>
#include <thread> /// C++ 11 标准的多线程写法
#include <algorithm>
#include <vector>
#include <list>
#include <omp.h>
#include <time.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

using namespace std;

int NOT_THREAD = (980); /// 多大长度一下不开启新线程
#define ELEMENT_COUNT 10000000
#define SORT_CHUNK_SIZE 30000

const int maxn = ELEMENT_COUNT + 7;
int A[maxn], B[maxn];

inline long long now() {
    timespec timestamp;
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    timestamp.tv_sec = mts.tv_sec;
    timestamp.tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, &timestamp);
#endif
    return (long long)timestamp.tv_sec * 1000000000LL + timestamp.tv_nsec;
}

template<class T>
void Sort(std::vector<T> & data)
{
    const intptr_t size = data.size();
    intptr_t stride = 20480;

    //从stride = 1开始归并排序非常缓慢
    //因此这里对data进行初排序
    //从一个较大的stride开始归并排序
    if (stride != 1) {
#pragma omp parallel for schedule(dynamic, 20480 / stride + 1)
        for (intptr_t i = 0; i < size; i += stride) {
//            printf("i=%d stride=%d\n", i, stride);
            auto left = data.begin() + i;
            auto right = i + stride < size ? data.begin() + i + stride : data.end();
            std::sort(left, right);
        }
    }

    //并行归并排序
#pragma omp parallel
    {
        intptr_t _stride = stride;
        do
        {
//            printf("_stride=%d\n", _stride);
            _stride *= 2;

#pragma omp for schedule(dynamic, 20480 / _stride + 1)
            for (intptr_t i = 0; i < size; i += _stride) {
                //对[i, i+_stride/2)和[i+_stride/2, i+_stride)进行归并
                auto left = data.begin() + i;
                auto mid = (i + i + _stride) / 2 < size ? data.begin() + (i + i + _stride) / 2 : data.end();
                auto right = i + _stride < size ? data.begin() + i + _stride : data.end();
                inplace_merge(left, mid, right);
            }
        } while (_stride < size);
    }
}

//快排获取中轴点
template<class T>
std::pair<intptr_t, intptr_t> QuickSortPartion(std::vector<T>& data, intptr_t left, intptr_t right) {
    //从count个样本中预测中值x
    intptr_t count = (right - left) * 0.001; if (count < 11) count = 11;
    intptr_t stride = (right - left) / count;
    std::vector<T> sample(count);
#pragma omp parallel for schedule(dynamic, SORT_CHUNK_SIZE)
    for (intptr_t i = 0; i < count; i++) {
        sample[i] = data[left + stride * i];
    }
    Sort(sample);
    const auto x = sample[count / 2];

    //并行分界
    //时间复杂度：O(n)
    //空间复杂度：O(1)
    const intptr_t mSize = SORT_CHUNK_SIZE;
    intptr_t iRead = left, jRead = right;
    intptr_t iWrite = left, jWrite = right;
    std::pair<intptr_t, intptr_t> retval;
#pragma omp parallel
    {
        //根据x分界
        std::vector<T> _data;
        std::vector<T> _dataSmaller;
        std::vector<T> _dataLarger;
        _data.reserve(mSize);
        _dataSmaller.reserve(mSize * 1.5);
        _dataLarger.reserve(mSize * 1.5);
        for (;;)
        {
            //第一步
            //线程安全地从data中取出一部分元素到_data中
            _data.clear();
#pragma omp critical
            {
#pragma omp flush(iRead)
#pragma omp flush(jRead)
#pragma omp flush(iWrite)
#pragma omp flush(jWrite)
                //最多分配mSize个元素
                const intptr_t size = jRead - iRead + 1 >= mSize ? mSize : jRead - iRead + 1;
                _data.resize(size);

                if (iRead - iWrite <= jWrite - jRead) {
                    //从左侧iRead处开始分配size个元素
                    std::copy(data.begin() + iRead, data.begin() + iRead + size, _data.begin());
                    iRead += size;
                }
                else {
                    //从右侧jRead处开始分配size个元素
                    std::copy(data.begin() + jRead - size + 1, data.begin() + jRead + 1, _data.begin());
                    jRead -= size;
                }
            }

            //第二步
            //在_data中与x比较，分为2组
            for (auto & val : _data) {
                if (val < x) {
                    _dataSmaller.push_back(val);
                }
                else if (val > x) {
                    _dataLarger.push_back(val);
                }
            }

            //第三步
            //线程安全地获取data中的写回下标，及各自的写回长度_iSize和_jSize
            intptr_t _iWrite = -1;
            intptr_t _jWrite = -1;
            intptr_t _iSize = 0;
            intptr_t _jSize = 0;
#pragma omp critical
            {
#pragma omp flush(iRead)
#pragma omp flush(jRead)
#pragma omp flush(iWrite)
#pragma omp flush(jWrite)
                bool isReadComplete = iRead > jRead;
                if (_dataSmaller.size() != 0) {
                    _iWrite = iWrite;

                    //当isReadComplete为假时，iWrite保证不超过iRead
                    //当isReadComplete为真时，iWrite才可以超过iRead
                    if (iWrite + _dataSmaller.size() <= iRead || isReadComplete) {
                        //可以全部写入
                        _iSize = _dataSmaller.size();
                    }
                    else {
                        //只允许部分写入
                        _iSize = iRead - iWrite;
                    }

                    //更新iWrite
                    iWrite += _iSize;
                }
                if (_dataLarger.size() != 0) {
                    _jWrite = jWrite;

                    //当isReadComplete为假时，jWrite保证不超过jRead
                    //当isReadComplete为真时，jWrite才可以超过jRead
                    if (jWrite - _dataLarger.size() >= jRead || isReadComplete) {
                        //可以全部写入
                        _jSize = _dataLarger.size();
                    }
                    else {
                        //只允许部分写入
                        _jSize = jWrite - jRead;
                    }

                    //更新iWrite
                    jWrite -= _jSize;
                }
            }

            //第四步
            //写回分组后的数据到data中
            if (_iWrite != -1) {
                //从左侧_iWrite处写回_dataSmaller中末端_iSize个元素
                std::copy(_dataSmaller.end() - _iSize, _dataSmaller.end(), data.begin() + _iWrite);
                _dataSmaller.resize(_dataSmaller.size() - _iSize);
            }
            if (_jWrite != -1) {
                //从右侧_jWrite处写回_dataLarger中末端_jSize个元素
                std::copy(_dataLarger.end() - _jSize, _dataLarger.end(), data.begin() + _jWrite - _jSize + 1);
                _dataLarger.resize(_dataLarger.size() - _jSize);
            }

            //最后
            //判断退出
            if (iRead > jRead) {
                if (_dataSmaller.size() == 0 && _dataLarger.size() == 0) break;
            }
        }

        //设置栅障
#pragma omp barrier

        //写回相等值
#pragma omp flush(iWrite)
#pragma omp flush(jWrite)
#pragma omp for schedule(dynamic, SORT_CHUNK_SIZE)
        for (intptr_t i = iWrite; i <= jWrite; i++) {
            data[i] = x;
        }
#pragma omp single
        {
            retval.first = iWrite;
            retval.second = jWrite;
        }
    }

    return retval;
}

//并行快速排序
template<class T>
void QuickSort(std::vector<T>& data)
{
    intptr_t threads = omp_get_max_threads();
    std::list<std::pair<intptr_t, intptr_t>> partions;
    partions.push_back(std::make_pair<intptr_t, intptr_t>(0, data.size() - 1));

    //分区
    for (int i = 0; i < threads * 1; i++) {
        std::pair<intptr_t, intptr_t> * largest = &partions.front();
        for (auto & p : partions) {
            if (p.second - p.first > largest->second - largest->first) largest = &p;
        }
        if (largest->second - largest->first < SORT_CHUNK_SIZE) break;

        auto mid = QuickSortPartion(data, largest->first, largest->second);
        partions.push_back(std::make_pair(largest->first, mid.first - 1));
        partions.push_back(std::make_pair(mid.second + 1, largest->second));
        partions.remove(*largest);
    }

    //排序
    std::vector<std::pair<intptr_t, intptr_t>> partions2(partions.begin(), partions.end());
#pragma omp parallel for schedule(dynamic)
    for (intptr_t i = 0; i < partions2.size(); i++) {
        auto & p = partions2[i];
        std::sort(data.begin() + p.first, data.begin() + p.second + 1);
    }
}

int rand(int L, int R) {
    int RND = rand(); /// linux 下直接使用 rand() 生成随机数即可

    return RND % (R - L + 1) + L;
}

void normalQuickSort(int s[], int l, int r)
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
        normalQuickSort(s, l, i - 1); // 递归调用
        normalQuickSort(s, i + 1, r);
    }
}

int main() {
    int n = ELEMENT_COUNT;
    for(int i = 1; i <= n; i ++) A[i] = rand(1, 1000000); /// 输入被排序的序列

    std::vector<int> vec(A, A+n-1);

    clock_t start,end;

    std::vector<int> vec1(vec);
    start=now();
    Sort(vec1);
    end=now();
    cout<<"F1运行时间"<<(double)(end-start)/(CLOCKS_PER_SEC*1000)<<endl;

//    std::vector<int> vec2(vec);
//    start=now();
//    QuickSort(vec2);
//    end=now();
//    cout<<"F2运行时间"<<(double)(end-start)/(CLOCKS_PER_SEC*1000)<<endl;

//    std::vector<int> vec3(vec);
//    start=now();
//    std::sort(vec3.begin(), vec3.begin() + n-1);
//    end=now();
//    cout<<"F3运行时间"<<(double)(end-start)/(CLOCKS_PER_SEC*1000)<<endl;
//
//    start=now();
//    normalQuickSort(A, 0 , n - 1);
//    end=now();
//    cout<<"F4运行时间"<<(double)(end-start)/(CLOCKS_PER_SEC*1000)<<endl;

    //输出 myvector 容器中的元素
//    for (std::vector<int>::iterator it = vec.begin(); it != vec.end(); ++it) {
//        std::cout << *it << ' ';
//    }
    return 0;

}



