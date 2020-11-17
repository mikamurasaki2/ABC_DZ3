#include<cmath>
#include<pthread.h>
#include<cstdint>
#include<fstream>
#include<iostream>

struct inh_share {
    double ratio;       // �����������.
    int sum;            // �������� ���������� ��� ������ ��������.
};

// ��������� ����� ������� �� ���������.
struct inh_task {
    int total_sum;      // ����� ����������.
    int n;              // ������ ����� �������.
    inh_share *part;    // ����� ������ �����.
};

int check_record(int total, const inh_share *ps)
{
    // ���������� �������� �� "���������" ��������, �� ���� ��������� �� ��������� ������� ����������.
    // ���� ������� ���������� ��������� ���������, �� �������� �������� ���������� ��� ������ ��������, ����� -1.
    return (fabs(ps->ratio*total-ps->sum) < 1)?ps->sum:-1;
}

void *task_proc(void *arg)
{
    inh_task *ptask = (inh_task *)arg;

    // ���� ������ ����� - ���� ������, �� �� ������ ��������� �� ������������ � ���������� ���������, 
    // ����������� � ���� void*.
    if (ptask->n == 1)
        return (void *)(intptr_t)check_record(ptask->total_sum, ptask->part);
    else {

        // ���� �� � ����� ������ ���������, �� ������� 2 ����� �������� ����� �����: ����� � ������. � ��������� ���� �� ���������.
        int nleft = ptask->n>>1;
        inh_task task_left = { ptask->total_sum, nleft, ptask->part };
        inh_task task_right = { ptask->total_sum, ptask->n - nleft, ptask->part+nleft };
        pthread_t thread_left, thread_right;
        void *rv_left, *rv_right;
        int res_left, res_right;

        // ������ ��� ����� ������: ����� � ������.
        pthread_create(&thread_left, NULL, task_proc, &task_left);
        pthread_create(&thread_right, NULL, task_proc, &task_right);

        // ������������ ���������� ����������� ������ ������� ������ �� ��� ���, ���� �� ����� �������� ����� thread.
        pthread_join(thread_left, &rv_left);
        pthread_join(thread_right, &rv_right);

        // ���������� ������ ������� � ������������� ��������.
        res_left = (int)(intptr_t)rv_left;
        res_right = (int)(intptr_t)rv_right;

        // ���������� ����� ������ ���� �������.
        // ���� ���-�� �� ��� ������ -1, �� ��� ��������, ��� ���-�� ���� ���������� ���� ��������. � �������� -1 ����������� � void *
        // � ��������� ������ �� ������ ����� �����������, ����������� � void*
        return (void *)(intptr_t)((res_left != -1 && res_right !=-1)?res_left+res_right:-1);
    }
}

int main() {

    // ������ �� �����.
    std::ifstream f("input.txt");
    inh_task t;
    void *rv;
    int res, i;

    // ���������� ����������� � ����� ����������, ��������� �� �����.
    f>>t.n>>t.total_sum;

    // ����.
    t.part = new inh_share[t.n*sizeof(inh_share)];

    // ��������� ������ ��������� � ��������� ��� ������� ������. ��������. 
    // ���� ��������� ������ � ������ ����������, �� ��� ��������, � ������� ������ �� �������� ����.
    for  (i=0; i<t.n; i++) 
        f>>t.part[i].ratio>>t.part[i].sum;

    rv = task_proc(&t);
    res = (int)(intptr_t)rv;
    delete [] t.part;

    // �������� ����� ������. ���� ��� ���������, �� ��������� ���������� 0, ����� 1.
    if (res == t.total_sum) {
        std::cout<<"The inheritance shared correctly\n";
        return 0;
    } else {
        std::cout<<"The inheritance shared incorrectly\n";
        return 1;
    }
}

// ����������� 1: ���� �������� � ������ ������ ����������. ����� �� ������� �������� ����� ������� ���, 
// ����� ��� �������� ���� ������ ������ ���������� ����������� �����.
// ����������� 2: ��������������, ��� � ���� ������ ����� ���������� ������, ��� 2^31.
