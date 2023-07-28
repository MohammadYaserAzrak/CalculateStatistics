# Calculate Statistics
Two applications, each calculates the min, max, and average of a collection of data. One using multiple processes (statFork.cpp), and the other using multiple threads (statThread.cpp).

## Prerequisites
- Qt 6 library 
  
## How to Run

1. Clone this repository:
```
git clone <repo-link>
```
2. Run the project:
```
qmake6 statThread.pro
make
./statThread <numOfThreads/Processes> data.txt
```
