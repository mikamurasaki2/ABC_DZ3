#include<cmath>
#include<pthread.h>
#include<cstdint>
#include<fstream>
#include<iostream>

struct inh_share {
    double ratio;       // Соотношение.
    int sum;            // Значение наследства для одного человека.
};

// Структура куска массива на обработку.
struct inh_task {
    int total_sum;      // Сумма наследства.
    int n;              // Размер куска массива.
    inh_share *part;    // Адрес начала куска.
};

int check_record(int total, const inh_share *ps)
{
    // Происходит проверка на "честность" адвоката, то есть правильно ли произошло деление наследства.
    // Если деление наследства произошла правильно, то вернется значение наследства для одного человека, иначе -1.
    return (fabs(ps->ratio*total-ps->sum) < 1)?ps->sum:-1;
}

void *task_proc(void *arg)
{
    inh_task *ptask = (inh_task *)arg;

    // Если размер куска - одна запись, то он честно проверяет ее корректность и возвращает результат, 
    // приведенный к типу void*.
    if (ptask->n == 1)
        return (void *)(intptr_t)check_record(ptask->total_sum, ptask->part);
    else {

        // Если же в куске больше элементов, то создаем 2 куска примерно одной длины: левый и правый. И запускаем нити на обработку.
        int nleft = ptask->n>>1;
        inh_task task_left = { ptask->total_sum, nleft, ptask->part };
        inh_task task_right = { ptask->total_sum, ptask->n - nleft, ptask->part+nleft };
        pthread_t thread_left, thread_right;
        void *rv_left, *rv_right;
        int res_left, res_right;

        // Содаем два новых потока: левый и правый.
        pthread_create(&thread_left, NULL, task_proc, &task_left);
        pthread_create(&thread_right, NULL, task_proc, &task_right);

        // Откладывание выполнение вызывающего данную функцию потока до тех пор, пока не будет выполнен поток thread.
        pthread_join(thread_left, &rv_left);
        pthread_join(thread_right, &rv_right);

        // Результаты работы потоков в целочисленном значении.
        res_left = (int)(intptr_t)rv_left;
        res_right = (int)(intptr_t)rv_right;

        // Возвращает сумму работы двух потоков.
        // Если кто-то из них вернул -1, то это означает, что где-то доля наследства была неверной. И вернется -1 приведенный к void *
        // В противном случае мы вернем сумму результатов, приведенную к void*
        return (void *)(intptr_t)((res_left != -1 && res_right !=-1)?res_left+res_right:-1);
    }
}

int main() {

    // Чтение из файла.
    std::ifstream f("input.txt");
    inh_task t;
    void *rv;
    int res, i;

    // Количество наследников и сумма наследства, считанных из файла.
    f>>t.n>>t.total_sum;

    // Доля.
    t.part = new inh_share[t.n*sizeof(inh_share)];

    // Заполняем массив наследств и структуру для первого вызова. Вызываем. 
    // Если результат совпал с суммой наследства, то все довольны, и адвокат ничего не присвоил себе.
    for  (i=0; i<t.n; i++) 
        f>>t.part[i].ratio>>t.part[i].sum;

    rv = task_proc(&t);
    res = (int)(intptr_t)rv;
    delete [] t.part;

    // Проверка итога работы. Если все правильно, то программа возвращает 0, иначе 1.
    if (res == t.total_sum) {
        std::cout<<"The inheritance shared correctly\n";
        return 0;
    } else {
        std::cout<<"The inheritance shared incorrectly\n";
        return 1;
    }
}

// Комментарий 1: Есть проблема с ловлей ошибки округления. Когда не удается поделить сумму поровну так, 
// чтобы при сложении всех кусков всегда получалась изначальная сумма.
// Комментарий 2: Предполагается, что у дона Энрике сумма наследства меньше, чем 2^31.
