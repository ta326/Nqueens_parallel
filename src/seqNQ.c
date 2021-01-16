#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define BILLION 1000000000L
/* A utility function to print solution */
int static no_sol = 0;
int static maxSum = 0;
int **maxBoardPtr;

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
  int **board;
  int col, n;
  int *ld;
  int *rd;
  int *cl;
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
  register int **board;
  int col, n;
  int *ld;
  int *rd;
  int *cl;
  int sum;

  board = arg->board;
  col = arg->col;
  cl = arg->cl;
  rd = arg->rd;
  ld = arg->ld;
  n = arg->n;
  sum = arg->sum;

  if (col >= n)
  {
    if (sum > maxSum)
    {
      maxSum = sum;
      for (int i = 0; i < n; i++)
      {
        for (int j = 0; j < n; j++)
        {
          maxBoardPtr[i][j] = board[i][j];
        }
      }
    }
    no_sol++;
    return NULL;
  }
  for (int i = 0; i < n; i++)
  {
    /* Check if the queen can be placed on 
          board[i][col] */
    /* A check if a queen can be placed on  
           board[row][col].We just need to check 
           ld[row-col+n-1] and rd[row+coln] where 
           ld and rd are for left and right  
           diagonal respectively*/
    if ((ld[i - col + n - 1] != 1 &&
         rd[i + col] != 1) &&
        cl[i] != 1)
    {
      /* Place this queen in board[i][col] */
      board[i][col] = 1;
      ld[i - col + n - 1] =
          rd[i + col] = cl[i] = 1;
      int incr = abs(i - col);

      /* recur to place rest of the queens */
      //if (solveNQUtil(board, col + 1)) {
      //    return true;
      //}
      //res = solveNQUtil(board, col + 1, n, ld, rd, cl) || res;
      arg->col = arg->col + 1;
      arg->sum = arg->sum + incr;
      Nqueen(arg);
      arg->col = arg->col - 1;
      arg->sum = arg->sum - incr;
      /* If placing queen in board[i][col] 
               doesn't lead to a solution, then 
               remove queen from board[i][col] */
      board[i][col] = 0; // BACKTRACK
      ld[i - col + n - 1] = 0;
      rd[i + col] = 0;
      cl[i] = 0;
    }
  }
  return NULL;
}

// driver program to test above function
int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("Usage: seq_NQ1 n\nAborting...\n");
    exit(0);
  }
  int n = atoi(argv[1]);
  int **board;
  struct timespec start, end;
  double timeInit, timeCompute, timeFinish;
  /* ld is an array where its indices indicate row-col+N-1 
    (N-1) is for shifting the difference to store negative  
     indices */
  /* rd is an array where its indices indicate row+col 
    and used to check whether a queen can be placed on  
    right diagonal or not*/
  int ld[n - 1 + n - 1 + 1];
  int rd[n - 1 + n - 1 + 1];
  int cl[n]; // need N-1 + N-1 + 1 for right diagonal
  /*column array where its indices indicates column and  
    used to check whether a queen can be placed in that 
    row or not*/
  for (int i = 0; i < n; i++)
  {
    cl[i] = 0;
  }
  int size = n - 1 + n - 1 + 1;
  for (int i = 0; i < size; i++)
  {
    ld[i] = 0;
    rd[i] = 0;
  }
  board = (int **)malloc(n * sizeof(int *));
  maxBoardPtr = (int **)malloc(n * sizeof(int *));
  for (int i = 0; i < n; i++)
  {
    board[i] = (int *)malloc(n * sizeof(int));
    maxBoardPtr[i] = (int *)malloc(n * sizeof(int));
    for (int j = 0; j < n; j++)
    {
      board[i][j] = 0;
      maxBoardPtr[i][j] = 0;
    }
  }

  //Start Init Time
  clock_gettime(CLOCK_MONOTONIC, &start);

  GM *arg = malloc(sizeof(*arg));
  arg->board = board;
  arg->col = 0;
  arg->cl = cl;
  arg->rd = rd;
  arg->ld = ld;
  arg->n = n;
  arg->sum = 0;

  //End Init Time
  clock_gettime(CLOCK_MONOTONIC, &end);
  timeInit =
      BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
  timeInit = timeInit / BILLION;

  //Start Compute Time
  clock_gettime(CLOCK_MONOTONIC, &start);

  Nqueen(arg);

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