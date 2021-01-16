#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#define BILLION 1000000000L
/* A utility function to print solution */
int static no_sol = 0;
int static maxSum = 0;
int static arrLen = 0;
int **maxBoardPtr;
pthread_t *threads;

pthread_mutex_t lock;
pthread_mutex_t lock1;

// pthread_mutex_t lock;
// void printSolution(int ***board, int n, int p)
// {
//     //int static no_sol = 1;
//     pthread_mutex_lock(&lock);
//     no_sol++;
//     int sum = 0;
//     //for (int i = 0; i < n; i++) {
//     for (int j = 0; j < n; j++)
//     {
//         for (int k = 0; k < n; k++)
//         {
//             if (board[p][j][k] == 1)
//                 sum += abs(j - k);
//             printf(" %d ", board[p][j][k]);
//         }
//         printf("\n");
//     }
//     //}
//     printf(" sum is %d \n", sum);
//     pthread_mutex_unlock(&lock);
// }

/*Struct for N-Qeens Algorithm: Holds
    (2D int array pointer) NxN board,
    (int) column value, # processors, board dimension, pid,
    (int array pointer) left-diagonal conflict array,
    (int array pointer) right-diagonal conflict array,
    (int array pointer) column conflict array,
    (int) sum
*/
typedef struct
{
  int *freeProc;
  int ***board;
  int col, p, n, pid;
  int **ld;
  int **rd;
  int **cl;
  int sum;
} GM;

/*Prints visual representation of a board 
  Takes pointer to (2D) board of dimension n. 
*/
void printSolution(int n)
{
  printf("Maximum Value Solution:\n");
  //int static no_sol = 1;
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      // int x = *(*(maxBoardPtr + i) + j);
      int x = maxBoardPtr[i][j];
      printf(" %d ", x);
    }
    printf("\n");
  }
  printf("Solution Sum: %d \n", maxSum);
  printf("Total Solutions Found: %d\n", no_sol);
  printf("Time Data:\n");
}

void *Nqueen(void *varg)
{
  GM *arg = varg;
  register int ***board;
  int col, n, p, pid;
  int **ld;
  int **rd;
  int **cl;
  int *freeProc;
  int sum;

  pid = arg->pid;
  board = arg->board;
  col = arg->col;
  cl = arg->cl;
  rd = arg->rd;
  ld = arg->ld;
  n = arg->n;
  p = arg->p;
  freeProc = arg->freeProc;
  sum = arg->sum;

  if (col >= n)
  {
    // printf("Get Here 0");
    pthread_mutex_lock(&lock);
    if (sum > maxSum)
    {
      maxSum = sum;
      for (int i = 0; i < n; i++)
      {
        for (int j = 0; j < n; j++)
        {
          maxBoardPtr[i][j] = board[pid][i][j];
        }
      }
    }
    no_sol++;
    pthread_mutex_unlock(&lock);
    pthread_mutex_lock(&lock1);
    free(&threads[pid]);
    pthread_mutex_unlock(&lock1);
    return NULL;
  }

  for (int i = 0; i < n; i++)
  {
    // printf("  Get Here 1");
    /* Check if the queen can be placed on 
          board[i][col] */
    /* A check if a queen can be placed on  
           board[row][col].We just need to check 
           ld[row-col+n-1] and rd[row+coln] where 
           ld and rd are for left and right  
           diagonal respectively*/
    if ((ld[pid][i - col + n - 1] != 1 &&
         rd[pid][i + col] != 1) &&
        cl[pid][i] != 1)
    {
      /* Place this queen in board[i][col] */
      board[pid][i][col] = 1;
      ld[pid][i - col + n - 1] =
          rd[pid][i + col] = cl[pid][i] = 1;
      int incr = abs(i - col);

      /* recur to place rest of the queens */
      int newCol = arg->col + 1;
      int newSum = arg->sum + incr;
      int sent = 0;
      for (int i = 0; i < p; i++)
      {
        // printf("    Get Here 2");
        pthread_mutex_lock(&lock1);
        if (sent == 0 && freeProc[i] == 0)
        {
          freeProc[i] == 1;
          int sent = 1;

          for (int k = 0; k < n; k++)
          {
            for (int j = 0; j < n; j++)
            {
              board[i][k][j] = board[pid][i][j];
            }
          }

          for (int j = 0; j < arrLen; j++)
          {
            ld[i][j] = ld[pid][j];
            rd[i][j] = ld[pid][j];
            cl[i][j] = ld[pid][j];
          }

          GM *arg = malloc(sizeof(*arg));
          arg->freeProc = freeProc;
          arg->board = board;
          arg->col = newCol;
          arg->cl = cl;
          arg->rd = rd;
          arg->ld = ld;
          arg->n = n;
          arg->p = p;
          arg->pid = i;
          arg->sum = newSum;
          pthread_create(&threads[i], NULL, Nqueen, arg);
          break;
        }
        pthread_mutex_unlock(&lock1);
      }

      if (sent == 0)
      {
        // printf("      Get Here 3");
        arg->col = newCol;
        arg->sum = newSum;
        Nqueen(arg);
      }
      /* If placing queen in board[i][col] 
               doesn't lead to a solution, then 
               remove queen from board[i][col] 
               BACKTRACK*/
      board[pid][i][col] = 0;
      ld[pid][i - col + n - 1] =
          rd[pid][i + col] = cl[pid][i] = 0;
    }
  }
  pthread_mutex_lock(&lock1);
  free(&threads[pid]);
  pthread_mutex_unlock(&lock1);
  return NULL;
}

// driver program to test above function
int main(int argc, char **argv)
{
  if (argc != 3)
  {
    printf("Usage: pbksb n p\nAborting...\n");
    exit(0);
  }
  int n = atoi(argv[1]);
  int p = atoi(argv[2]);

  struct timespec start, end;
  double timeInit, timeCompute, timeFinish;
  /* ld is an array where its indices indicate row-col+N-1 
    (N-1) is for shifting the difference to store negative  
     indices */
  /* rd is an array where its indices indicate row+col 
    and used to check whether a queen can be placed on  
    right diagonal or not*/
  int ***board;
  int **ld;
  int **rd;
  int **cl;
  arrLen = (2 * n) - 1;
  // need N-1 + N-1 + 1 for right diagonal
  /*column array where its indices indicates column and  
    used to check whether a queen can be placed in that 
    row or not*/
  maxBoardPtr = (int **)malloc(n * sizeof(int *));

  ld = (int **)malloc(p * sizeof(int *));
  rd = (int **)malloc(p * sizeof(int *));
  cl = (int **)malloc(p * sizeof(int *));
  board = (int ***)malloc(p * sizeof(int **));
  for (int j = 0; j < p; j++)
  {
    ld[j] = calloc(arrLen, sizeof(int));
    rd[j] = calloc(arrLen, sizeof(int));
    cl[j] = calloc(n, sizeof(int));
    board[j] = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
      board[j][i] = calloc(n, sizeof(int));
      maxBoardPtr[i] = calloc(n, sizeof(int));
    }
  }

  //Start Init Time
  clock_gettime(CLOCK_MONOTONIC, &start);

  int halfCol = (n + 1) / 2;
  int startRows = halfCol <= p ? halfCol : p;

  int *freeProc = calloc(p, sizeof(int));

  threads = malloc(p * sizeof(threads));
  for (int i = 0; i < startRows; i++)
  {
    freeProc[i] = 1;
    board[i][0][i] = 1;
    ld[0][i - 0 + n - 1] =
        rd[0][i + 0] =
            cl[0][i] = 1;

    GM *arg = malloc(sizeof(*arg));
    arg->freeProc = freeProc;
    arg->board = board;
    arg->col = 1;
    arg->cl = cl;
    arg->rd = rd;
    arg->ld = ld;
    arg->n = n;
    arg->p = p;
    arg->pid = i;
    arg->sum = 0;
    pthread_create(&threads[i], NULL, Nqueen, arg);
  }

  //End Init Time
  clock_gettime(CLOCK_MONOTONIC, &end);
  timeInit =
      BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
  timeInit = timeInit / BILLION;

  //Start Compute Time
  clock_gettime(CLOCK_MONOTONIC, &start);

  for (int i = 0; i < p; i++)
    pthread_join(threads[i], NULL);

  //End Compute Time
  clock_gettime(CLOCK_MONOTONIC, &end);
  timeCompute =
      BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
  timeCompute = timeCompute / BILLION;

  //Start Finish Time
  clock_gettime(CLOCK_MONOTONIC, &start);

  //Print maximum solution
  printSolution(n);

  //End Compute Time
  clock_gettime(CLOCK_MONOTONIC, &end);
  timeFinish =
      BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
  timeFinish = timeFinish / BILLION;

  printf("  Initialization Time: %lf seconds\n", timeInit);
  printf("  Computation Time: %lf seconds\n", timeCompute);
  printf("  Finish Time: %lf seconds\n", timeFinish);
  printf("  Total Elapsed Time: %lf seconds\n",
         timeInit + timeCompute + timeFinish);
  return 0;
}