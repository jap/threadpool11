#include "threadpool.hh"

#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <condition_variable>

#include <iostream>
#include <thread>
#include <chrono>



fifo<int> f1(50000), f2(40000);

const int N(1680000);
const int num_readers(2);
const int num_pushers(2);
const int num_writers(2);

std::mutex io_mutex;

int sleep_some()
{
    static __thread int slept;
    static const std::chrono::milliseconds du(1);
    std::this_thread::sleep_for(du);
    return ++slept;
}

void reader(const int k) {
    int z;
    //std::cout << "r+";
    for (int p=0; p<(N/k); ++p) {
        while(!f2.pop(z))
            sleep_some();
        /* if (!(p%1000)) {
          std::unique_lock<std::mutex> iolock(io_mutex);
          std::cout <<"read " << p << std::endl;
        } */

    }
    //std::unique_lock<std::mutex> iolock(io_mutex);
    //std::cout << "r-" << z << " " << (sleep_some()-1) << std::endl;
}

void pusher(const int k) {
    int z;
    //std::cout << "p+" << std::endl;
    for (int p=0; p<(N/k); ++p) {
        while (!f1.pop(z))
            sleep_some();
        while (!f2.push(z))
            sleep_some();
    }
    //std::unique_lock<std::mutex> iolock(io_mutex);
    //std::cout << "p-" << " " << (sleep_some()-1) << std::endl;
}

void writer(const int k) {
    std::cout << "w+" << std::endl;
    for (int p=0; p<(N/k); ++p) {
        while (!f1.push(p))
            sleep_some();
        /* if (!(p%1000)) {
            std::unique_lock<std::mutex> iolock(io_mutex);
            std::cout <<"wrote " << p << std::endl;
        } */
    }
    //std::unique_lock<std::mutex> iolock(io_mutex);
    //std::cout << "w-" << " " << (sleep_some()-1) << std::endl;
}

int main()
{
    using namespace std::chrono;
  try {
    std::cout << "S";
    auto start = steady_clock::now();
    std::vector<std::thread> Ts;
    for (int i=0; i<num_writers; ++i) {
        Ts.push_back(std::thread(writer, num_writers));
    }
    for (int i=0; i<num_pushers; ++i) {
        Ts.push_back(std::thread(pusher, num_pushers));
    }
    for (int i=0; i<num_readers; ++i) {
        Ts.push_back(std::thread(reader, num_readers));
    }
    std::cout << "W" << std::endl;
    std::for_each(Ts.begin(), Ts.end(), [](std::thread& t){ t.join(); });
    auto end = steady_clock::now();
    std::cout << "this took "
              << duration_cast<microseconds>(end - start).count()
              << "ns" << std::endl;
  } catch (...) {
    std::cerr << "caught something" << std::endl;
  }
}
