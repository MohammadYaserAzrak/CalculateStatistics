#include <iostream>
#include <unistd.h>
#include <fstream>
#include <cmath>
#include <cfloat>
#include <sys/wait.h>
#include <fcntl.h>
#include <iomanip>

char *fileName;
int children;

// API
struct context
{
	float min = FLT_MAX;
	float max = 0;
	float sum = 0;
	float avg = 0;

	void
	processData(int beg, int end, float *data, int dataNum)
	{
		int tmax = data[beg];
		int tmin = data[beg];
		for (int i = beg; i < end; i++)
		{
			if (data[i] < tmin)
				tmin = data[i];
			else if (data[i] > tmax)
				tmax = data[i];

			sum += data[i];
			avg = sum / dataNum;
		}
		if (tmin < min)
			min = tmin;
		if (tmax > max)
			max = tmax;
	}
	void
	updateFromBuffer(context *buff, int dataNum)
	{
		if (min > buff->min)
			min = buff->min;
		if (max < buff->max)
			max = buff->max;

		sum += buff->sum;
		avg = sum / dataNum;
	}
};

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cout << "Usage: <children> <file>" << std::endl;
		exit(-1);
	}

	children = atoi(argv[1]);
	fileName = argv[2];

	std::ifstream file;
	clock_t *start;
	clock_t *end;
	context fdata;
	double time;
	context *buff = new context;
	int dataNum;
	int div;
	float *data;

	// attempt to open <file>
	file.open(fileName);
	if (!file)
	{
		std::cout << "Error Opening file: " << fileName << std::endl;
		exit(-1);
	}
	else
	{
		file >> dataNum;
		if (dataNum < children)
		{
			std::cout << "Work can't be divided to given data" << std::endl;
			exit(-1);
		}
		data = new float[dataNum];
		for (int i = 0; i < dataNum; i++)
		{
			file >> data[i];
		}
	}

	file.close();

	start = new clock_t[children + 1];
	end = new clock_t[children + 1];
	int pipefds[2 * children];
	div = ceil((float)dataNum / (children + 1));

	// Fork children and process all data in parallel
	for (int i = 0; i < children; i++)
	{
		pipe(pipefds + i * 2);
		if (fork() == 0)
		{
			context cdata;
			start[i] = clock();
			cdata.processData(i * div, (i + 1) * (div), data, dataNum);
			end[i] = clock();

			write(pipefds[i * 2 + 1], &start[i], sizeof(clock_t));
			write(pipefds[i * 2 + 1], &end[i], sizeof(clock_t));
			write(pipefds[i * 2 + 1], &cdata, sizeof(context));
			close(pipefds[i * 2 + 1]);
			exit(0);
		}
	}

	if (div != 1)
	{
		start[children] = clock();
		fdata.processData(children * div, dataNum, data, dataNum);
		end[children] = clock();
	}

	for (int i = 0; i < children; i++)
	{
		wait(NULL);
		read(pipefds[i * 2], &start[i], sizeof(clock_t));
		read(pipefds[i * 2], &end[i], sizeof(clock_t));
		read(pipefds[i * 2], buff, sizeof(context));
		fdata.updateFromBuffer(buff, dataNum);
		close(pipefds[i * 2]);
	}

	for (int i = 0; i < children + 1; i++)
	{
		clock_t tstart = start[i];
		clock_t tend = end[i];

		if (start[i] < tstart)
			tstart = start[i];
		else if (end[i] > tend)
			tend = end[i];

		time = double(tend - tstart) / double(CLOCKS_PER_SEC);
	}

	std::cout << "Min: " << fdata.min << std::endl;
	std::cout << "Max: " << fdata.max << std::endl;
	std::cout << "Avg: " << fdata.avg << std::endl;
	std::cout << "Execution Time: " << std::fixed << time << std::setprecision(5) << " Sec" << std::endl;

	delete buff;
	delete[] data;

	return 0;
}
