#include <stdio.h>
#include <thread>

#include "CycleTimer.h"

typedef struct {
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int numThreads;
} WorkerArgs;


extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);


//
// workerThreadStart --
//
// Thread entrypoint.
void workerThreadStart(WorkerArgs * const args) {

    // DONE FOR CS149 STUDENTS: Implement the body of the worker
    // thread here. Each thread should make a call to mandelbrotSerial()
    // to compute a part of the output image.  For example, in a
    // program that uses two threads, thread 0 could compute the top
    // half of the image and thread 1 could compute the bottom half.
    double startTime = CycleTimer::currentSeconds();
    
    /* Unbalanced Workload
    unsigned int block_height = (args->height + args->numThreads - 1) / args->numThreads;
    unsigned int start_row = args->threadId * block_height;
    unsigned int row_count =  std::min<unsigned>(block_height, args->height - start_row);
    mandelbrotSerial(args->x0, args->y0, args->x1, args->y1, args->width, args->height, start_row, row_count, args->maxIterations, args->output);
    */

    /* Balanced Workload
    */
    unsigned int n_squared = args->numThreads * args->numThreads;
    unsigned int block_height = (args->height + n_squared - 1) / n_squared;
    unsigned int n_block_height = (args->height + args->numThreads - 1) / args->numThreads;
    for (int i = 0; i < args->numThreads; i++) {
        unsigned int start_row = n_block_height * i + block_height * args->threadId;
        unsigned int row_count = std::min<unsigned>(block_height, args->height - start_row);
        mandelbrotSerial(args->x0, args->y0, args->x1, args->y1, args->width, args->height, start_row, row_count, args->maxIterations, args->output);
    }

    double endTime = CycleTimer::currentSeconds();
    // printf("Hello world from thread %d\n", args->threadId);
    // printf("Total rows: %d\n", args->height);
    // printf("Thread %d Start Row: %d Row Count: %d\n", args->threadId, start_row, row_count);
    printf("[thread %d]:\t\t[%.3f] ms\n", args->threadId, (endTime - startTime) * 1000);
}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Threads of execution are created by spawning std::threads.
void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    static constexpr int MAX_THREADS = 32;

    if (numThreads > MAX_THREADS)
    {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    // Creates thread objects that do not yet represent a thread.
    std::thread workers[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    for (int i=0; i<numThreads; i++) {
      
        // DONE FOR CS149 STUDENTS: You may or may not wish to modify
        // the per-thread arguments here.  The code below copies the
        // same arguments for each thread
        args[i].x0 = x0;
        args[i].y0 = y0;
        args[i].x1 = x1;
        args[i].y1 = y1;
        args[i].width = width;
        args[i].height = height;
        args[i].maxIterations = maxIterations;
        args[i].numThreads = numThreads;
        args[i].output = output;
      
        args[i].threadId = i;
    }

    // Spawn the worker threads.  Note that only numThreads-1 std::threads
    // are created and the main application thread is used as a worker
    // as well.
    for (int i=1; i<numThreads; i++) {
        workers[i] = std::thread(workerThreadStart, &args[i]);
    }
    
    workerThreadStart(&args[0]);

    // join worker threads
    for (int i=1; i<numThreads; i++) {
        workers[i].join();
    }
}

