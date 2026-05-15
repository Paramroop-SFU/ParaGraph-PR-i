#include "core/graph.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <vector>
#include <mutex>

#ifdef USE_INT
#define INIT_PAGE_RANK 100000
#define EPSILON 1000
#define PAGE_RANK(x) (15000 + (5 * x) / 6)
#define CHANGE_IN_PAGE_RANK(x, y) std::abs(x - y)
typedef int64_t PageRankType;
#else
#define INIT_PAGE_RANK 1.0
#define EPSILON 0.01
#define DAMPING 0.85
#define PAGE_RANK(x) (1 - DAMPING + DAMPING * x)
#define CHANGE_IN_PAGE_RANK(x, y) std::fabs(x - y)
typedef float PageRankType;
#endif
using namespace std;

double thread_fuc(uintV n,PageRankType *pr_curr,PageRankType* pr_next,Graph &g,uint maxs,mutex &mux,CustomBarrier& my_barrier){
  timer t1;
  t1.start();
  for (int j =0; j < maxs; j++){ 
    for (uintV u = 0; u < n; u++) {
      uintE out_degree = g.vertices_[u].getOutDegree();
      for (uintE i = 0; i < out_degree; i++) {
        uintV v = g.vertices_[u].getOutNeighbor(i);
        mux.lock();
        pr_next[v] += (pr_curr[u] / out_degree);
        mux.unlock();
      }
    }
    my_barrier.wait();
    for (uintV v = 0; v < n; v++) {
     
      pr_next[v] = PAGE_RANK(pr_next[v]);

      // reset pr_curr for the next iteration
      pr_curr[v] = pr_next[v];
      pr_next[v] = 0.0;
      
    }
    my_barrier.wait();
  }
    return  t1.stop();
}

void pageRankSerial(Graph &g, int max_iters, uint workers) {
  uintV n = g.n_;

  PageRankType *pr_curr = new PageRankType[n];
  PageRankType *pr_next = new PageRankType[n];

  for (uintV i = 0; i < n; i++) {
    pr_curr[i] = INIT_PAGE_RANK;
    pr_next[i] = 0.0;
  }
  vector<double> times(workers);
  // Push based pagerank
  timer t1;
  double time_taken = 0.0;
  // Create threads and distribute the work across T threads
  vector<thread> threads;
  uint n_per_worker =   max_iters/workers;
  uint remainder =  max_iters%workers;
  mutex mux;
  CustomBarrier my_barrier(workers);

  // -------------------------------------------------------------------
  t1.start();
  for (uint i =1; i < workers; i++)
  {
    threads.push_back(thread([&, i](){
        times[i] = thread_fuc(n,pr_curr,pr_next,g,n_per_worker,mux,my_barrier);
    }));

  }
  uint is = 0;
  threads.push_back(thread([&,is](){
      times[is] = thread_fuc(n,pr_curr,pr_next,g,n_per_worker+remainder,mux,my_barrier);
    }));



  for (auto &i : threads) {
    i.join();
  }
  time_taken = t1.stop();
  // -------------------------------------------------------------------
  std::cout << "thread_id, time_taken\n";
  for (int h =0; h < workers;h++ )
  {
    cout << h << ", " << times[h] << endl;
  }
  // Print the above statistics for each thread
  // Example output for 2 threads:
  // thread_id, time_taken
  // 0, 0.12
  // 1, 0.12

  PageRankType sum_of_page_ranks = 0;
  for (uintV u = 0; u < n; u++) {
    sum_of_page_ranks += pr_curr[u];
  }
  std::cout << "Sum of page rank : " << sum_of_page_ranks << "\n";
  std::cout << "Time taken (in seconds) : " << time_taken << "\n";
  delete[] pr_curr;
  delete[] pr_next;
}

int main(int argc, char *argv[]) {
  cxxopts::Options options(
      "page_rank_push",
      "Calculate page_rank using serial and parallel execution");
  options.add_options(
      "",
      {
          {"nWorkers", "Number of workers",
           cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_WORKERS)},
          {"nIterations", "Maximum number of iterations",
           cxxopts::value<uint>()->default_value(DEFAULT_MAX_ITER)},
          {"inputFile", "Input graph file path",
           cxxopts::value<std::string>()->default_value(
               "/scratch/input_graphs/roadNet-CA")},
      });

  auto cl_options = options.parse(argc, argv);
  uint n_workers = cl_options["nWorkers"].as<uint>();
  uint max_iterations = cl_options["nIterations"].as<uint>();
  std::string input_file_path = cl_options["inputFile"].as<std::string>();

#ifdef USE_INT
  std::cout << "Using INT\n";
#else
  std::cout << "Using FLOAT\n";
#endif
  std::cout << std::fixed;
  std::cout << "Number of workers : " << n_workers << "\n";

  Graph g;
  std::cout << "Reading graph\n";
  g.readGraphFromBinary<int>(input_file_path);
  std::cout << "Created graph\n";
  pageRankSerial(g, max_iterations,n_workers);

  return 0;
}
