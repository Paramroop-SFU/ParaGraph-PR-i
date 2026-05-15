#include "core/graph.h"
#include "core/utils.h"
#include <future>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <vector>

using namespace std;
struct return_val {
    long triangle;
    double time;
};

long countTriangles(uintV *array1, uintE len1, uintV *array2, uintE len2,
                     uintV u, uintV v) {

  uintE i = 0, j = 0; // indexes for array1 and array2
  long count = 0;

  if (u == v)
    return count;

  while ((i < len1) && (j < len2)) {
    if (array1[i] == array2[j]) {
      if ((array1[i] != u) && (array1[i] != v)) {
        count++;
      }
      i++;
      j++;
    } else if (array1[i] < array2[j]) {
      i++;
    } else {
      j++;
    }
  }
  return count;
}

return_val thread_func(Graph &g,uintV start, uintV end){
  timer t1;
  t1.start();
  
  long triangle_count = 0;
  for (uintV u = start; u < end; u++) {
    // For each outNeighbor v, find the intersection of inNeighbor(u) and
    // outNeighbor(v)
    uintE out_degree = g.vertices_[u].getOutDegree();
    for (uintE i = 0; i < out_degree; i++) {
      uintV v = g.vertices_[u].getOutNeighbor(i);
      triangle_count += countTriangles(g.vertices_[u].getInNeighbors(),
                                       g.vertices_[u].getInDegree(),
                                       g.vertices_[v].getOutNeighbors(),
                                       g.vertices_[v].getOutDegree(), u, v);
    }
  }
  double time_taken = t1.stop();
  return_val r = {triangle_count,time_taken};
  return r;
}

void triangleCountSerial(Graph &g,uint workers) {
  uintV n = g.n_;
  long triangle_count = 0;
  double time_taken = 0.0;
  timer t1;

  // The outNghs and inNghs for a given vertex are already sorted

  // Create threads and distribute the work across T threads
  vector<thread> threads; 
  vector<return_val> result(workers);
  uint nums_per_thread = n/workers;
  

  

  // -------------------------------------------------------------------
  t1.start();
  for (uint i =0; i < workers-1; i++){
    uintV start = i*nums_per_thread;
    uintV end = start + nums_per_thread;
    threads.push_back(thread([&,i,start,end](){
        result[i] = thread_func(g,start,end);
    }));
  }
  uint remainder = n%workers;
  int m = workers-1;
  uintV start = m*nums_per_thread;
  uintV end = start + nums_per_thread + remainder;
  
  threads.push_back(thread([&,m,start,end](){
        result[m] = thread_func(g,start,end);
    }));


  for (auto &i : threads){
     i.join();
    }
  for (auto i :result){
    triangle_count += i.triangle;
  }





  
  time_taken = t1.stop();
  // -------------------------------------------------------------------
  // Here, you can just print the number of non-unique triangles counted by each
  cout << "thread_id, triangle_count, time_taken\n"; 
  for (uint i =0; i < workers; i++){
    cout << i << ", " << result[i].triangle << ", " << result[i].time << endl;
  }
  
  // above statistics for each thread Example output for 2 threads: thread_id,
  // triangle_count, time_taken 1, 102, 0.12 0, 100, 0.12

  // Print the overall statistics
  std::cout << "Number of triangles : " << triangle_count << "\n";
  std::cout << "Number of unique triangles : " << triangle_count / 3 << "\n";
  std::cout << "Time taken (in seconds) : " << std::setprecision(TIME_PRECISION)
            << time_taken << "\n";
}

int main(int argc, char *argv[]) {
  cxxopts::Options options(
      "triangle_counting_serial",
      "Count the number of triangles using serial and parallel execution");
  options.add_options(
      "custom",
      {
          {"nWorkers", "Number of workers",
           cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_WORKERS)},
          {"inputFile", "Input graph file path",
           cxxopts::value<std::string>()->default_value(
               "/scratch/input_graphs/roadNet-CA")},
      });

  auto cl_options = options.parse(argc, argv);
  uint n_workers = cl_options["nWorkers"].as<uint>();
  std::string input_file_path = cl_options["inputFile"].as<std::string>();
  std::cout << std::fixed;
  std::cout << "Number of workers : " << n_workers << "\n";

  Graph g;
  std::cout << "Reading graph\n";
  g.readGraphFromBinary<int>(input_file_path);
  std::cout << "Created graph\n";

  triangleCountSerial(g,n_workers);

  return 0;
}
