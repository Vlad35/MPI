#include "mpi.h"
#include "stdio.h"
#include  <time.h>
#include <math.h>
const double a = 0.0;//Нижний предел
const double b = 1.0;//Верхний предел
const double h = 0.000001;//Шаг интегрирования


using namespace std;

double fnc(double x)//Интегрируемая функция
{
	return 4 / (1 + x * x);
}


int main(int argc, char** argv)
{
	int myrank, ranksize, i;
	clock_t start, finish;

	MPI_Init(&argc, &argv);//Инициализация MPI
//Определяем свой номер в группе:
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	//Определяем размер группы:
	MPI_Comm_size(MPI_COMM_WORLD, &ranksize);
	double cur_a, cur_b, d_ba, cur_h;
	double* sbuf = NULL;
	if (!myrank)
	{//Это процесс-мастер
		//Определяем размер диапазона для каждого процесса:
		d_ba = (b - a) / ranksize;
		sbuf = new double[ranksize * 3];
		cur_a = a;
		cur_h = h;
		for (i = 0; i < ranksize; i++)
		{
			cur_b = cur_a + d_ba - h;
			sbuf[i * 3] = cur_a;
			sbuf[i * 3 + 1] = cur_b;
			sbuf[i * 3 + 2] = h;
			cur_a += d_ba;
		}
	}
	double rbuf[3];

	start = clock();
	//Рассылка всем процессам, включая процесс-мастер
	//начальных данных для расчета:
	MPI_Scatter(sbuf, 3, MPI_DOUBLE, rbuf, 3, MPI_DOUBLE, 0,
		MPI_COMM_WORLD);
	if (sbuf) delete[]sbuf;
	cur_a = rbuf[0]; cur_b = rbuf[1]; cur_h = rbuf[2];
	//Расчет интеграла в своем диапазоне, выполняют все 
	//процессы:
	double s = 0;
	printf("Process %d. A=%.4f B=%.4f h=%.10f\n",
		myrank, cur_a, cur_b, cur_h);
	for (cur_a += cur_h; cur_a <= cur_b; cur_a += cur_h)
		s += cur_h * fnc(cur_a);
	rbuf[0] = s;
	if (!myrank) sbuf = new double[ranksize];
	//Собираем значения интегралов от процессов:
	MPI_Gather(rbuf, 1, MPI_DOUBLE, sbuf, 1, MPI_DOUBLE, 0,
		MPI_COMM_WORLD);
	if (!myrank)
	{//Это процесс-мастер
		//Суммирование интегралов, полученных каждым 
		//процессом:
		for (i = 0, s = 0; i < ranksize; i++) s += sbuf[i];
		finish = clock();
		//Печать результата:
		printf("Integral value: %.4f\n", s);
		printf("Time: %.4f\n", (double)(finish - start) / CLOCKS_PER_SEC);

		delete[]sbuf;
	}
	MPI_Finalize();//Завершение работы с MPI
	getchar();
	return 0;
}
