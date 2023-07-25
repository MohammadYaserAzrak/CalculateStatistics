#include <QThread>
#include <QTime>
#include <QDebug>
#include <QElapsedTimer>
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;

class statThread : public QThread
{
public:
    int ID;
    double *ARR;
    int B, E;
    double localMin, localMax, localSum;
    statThread(int id, double *arr, int b, int e) : ID(id), ARR(arr), B(b), E(e)
    {
    }
    void run()
    {
        localMin = ARR[B];
        localMax = ARR[B];
        localSum = ARR[B];
        for (int i = B; i < E; i++)
        {
            if (ARR[i] < localMin)
                localMin = ARR[i];
            else if (ARR[i] > localMax)
                localMax = ARR[i];
            localSum += ARR[i];
        }
    }
};

int main(int argc, char *argv[])
{
    int N;
    double Min, Max, Sum, Average;
    if (argc != 3)
    {
        cout << "Invalid # of parameters" << endl;
        return 0;
    }

    string filename = argv[2];
    ifstream fin;
    fin.open(filename);
    if (fin.fail())
    {
        cout << "File failed to open" << endl;
        return 0;
    }
    fin >> N;
    double *text = new double[N];
    for (int i = 0; i < N; i++)
    {
        fin >> text[i];
    }
    fin.close();
    int numOfThreads = atoi(argv[1]);
    statThread *thread[numOfThreads];
    int partLen = ceil(1.0 * N / numOfThreads);

    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < numOfThreads; i++)
    {
        int b = i * partLen;
        int e = (i + 1) * partLen;
        thread[i] = new statThread{i, text, b, e};
        thread[i]->start();
    }
    thread[numOfThreads] = new statThread{numOfThreads - 1, text, (numOfThreads - 1) * partLen, N - 1};
    thread[numOfThreads]->start();

    for (int i = 0; i < numOfThreads + 1; i++)
    {
        thread[i]->wait();
    }

    Min = thread[0]->localMin;
    Max = thread[0]->localMax;
    Sum = thread[0]->localSum;
    for (int i = 1; i < numOfThreads; i++)
    {
        if (thread[i]->localMin < Min)
            Min = thread[i]->localMin;
        else if (thread[i]->localMax > Max)
            Max = thread[i]->localMax;
        Sum = Sum + thread[i]->localSum;
    }
    Average = (float)Sum / N;

    cout << "Min: " << setprecision(4) << Min << endl;
    cout << "Max: " << setprecision(4) << Max << endl;
    cout << "Average: " << setprecision(4) << Average << endl;

    qint64 timeElapsed = timer.elapsed();
    cout << "Time elapsed: " << timeElapsed << " ms" << endl;

    for (int i = 0; i < numOfThreads; i++)
    {
        delete thread[i];
    }
    delete[] text;
    return 0;
}
