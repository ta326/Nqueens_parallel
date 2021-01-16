#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#define BILLION 1000000000L
/* A utility function to print solution */
int static no_sol = 0;
int static maxSum = 0;
int **maxBoardPtr;

struct timespec ts1, ts2;
double timeInit;

typedef struct
{
    int ***board;
    int *col, p, n, pid;
    int **ld;
    int **rd;
    int **cl;
    int row_start;
    int row_end;
    int total_board;
    int *sum;
} GM;

GM **gm;

pthread_mutex_t lock;
pthread_barrier_t barrier;

/*Prints visual representation of a board 
  Takes pointer to (2D) board of dimension n. 
*/
void printSolution(int n)
{
    printf("Maximum Value Solution:\n");
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
}

int static no_board = 0;
int static curr_board = 0;

// used to calculate all sub boards when p > n
void *
solve_sub(GM *gm, int **board, int *ld, int *rd, int *cl, int col, int n, int size, int type)
{
    if (col >= 2 && type == 1)
    {
        for (int j = 0; j < n; j++)
        {
            //board[i][j] = (int *) malloc(n * sizeof(int ));
            for (int k = 0; k < n; k++)
            {
                if (board[j][k] == 1)
                {
                    gm->board[curr_board][j][k] = 1;
                    gm->sum[curr_board] = gm->sum[curr_board] + abs(j - k);
                }
            }
        }
        for (int i = 0; i < size; i++)
        {
            if (ld[i] == 1)
                gm->ld[curr_board][i] = 1;
            if (rd[i] == 1)
                gm->rd[curr_board][i] = 1;
        }
        for (int j = 0; j < n; j++)
        {
            if (cl[j] == 1)
                gm->cl[curr_board][j] = 1;
        }
        //printSolution(gm->board[no_board1], n);
        curr_board += 1;
        //printSolution(board,n);

        return NULL;
    }

    //Type 0 means only solve total number of boards
    //(number of possible solutions in first two columns)
    if (col >= 2 && type == 0)
    {
        no_board += 1;
        //printSolution(board,n);

        return NULL;
    }

    for (int i = 0; i < n; i++)
    {
        if ((ld[i - col + n - 1] != 1 &&
             rd[i + col] != 1) &&
            cl[i] != 1)
        {
            /* Place this queen in board[i][col] */
            board[i][col] = 1;
            ld[i - col + n - 1] =
                rd[i + col] = cl[i] = 1;
            int incr = abs(i - col);
            col += 1;
            solve_sub(gm, board, ld, rd, cl, col, n, size, type);
            col -= 1;
            board[i][col] = 0; // BACKTRACK
            ld[i - col + n - 1] = 0;
            rd[i + col] = 0;
            cl[i] = 0;
        }
    }
    return NULL;
}

void *
Nqueen(int **board, int *ld, int *rd, int *cl, int col, int n, int sum)
{
    if (col >= n)
    {
        //pthread_mutex_lock(&lock);
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
        pthread_mutex_lock(&lock);
        no_sol++;
        pthread_mutex_unlock(&lock);
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
        //for ( int row = row_start; row <= row_end; row++) {
        if ((ld[i - col + n - 1] != 1 &&
             rd[i + col] != 1) &&
            cl[i] != 1)
        {
            /* Place this queen in board[i][col] */
            int incr = abs(i - col);
            board[i][col] = 1;
            ld[i - col + n - 1] =
                rd[i + col] = cl[i] = 1;

            /* recur to place rest of the queens */
            //if (solveNQUtil(board, col + 1)) {
            //    return true;
            //}
            //res = solveNQUtil(board, col + 1, n, ld, rd, cl) || res;
            col += 1;
            Nqueen(board, ld, rd, cl, col, n, sum + incr);
            col -= 1;
            /* If placing queen in board[i][col] 
               doesn't lead to a solution, then 
               remove queen from board[i][col] */
            board[i][col] = 0; // BACKTRACK
            ld[i - col + n - 1] = 0;
            rd[i + col] = 0;
            cl[i] = 0;
        }
        //}
    }
    return NULL;
}

void *Parallel(void *varg)
{
    pthread_barrier_wait(&barrier);
    GM *arg = varg;
    register int ***board;
    int *col, n;
    int **ld;
    int **rd;
    int **cl;
    int total = arg->total_board;
    int pid = arg->pid;
    int *sum = arg->sum;
    board = arg->board;
    col = arg->col;
    cl = arg->cl;
    rd = arg->rd;
    ld = arg->ld;
    //p = arg->p;
    n = arg->n;
    // pid = arg->pid;

    if (pid == 0)
    {
        clock_gettime(CLOCK_MONOTONIC, &ts2);
        timeInit =
            BILLION * (ts2.tv_sec - ts1.tv_sec) + (ts2.tv_nsec - ts1.tv_nsec);
        timeInit = timeInit / BILLION;
    }

    int row_start = arg->row_start;
    int row_end = arg->row_end;
    for (int i = arg->row_start; i <= arg->row_end; i++)
    {
        // printf("SUM FOR : %d is %d\n", i, sum[i]);
        if (n > total)
            Nqueen(board[i], ld[i], rd[i], cl[i], 1, n, sum[i]);
        else
            Nqueen(board[i], ld[i], rd[i], cl[i], 2, n, sum[i]);
    }
    return NULL;
}

// driver program to test above function
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: nqueen n p\nAborting...\n");
        exit(0);
    }
    int n = atoi(argv[1]);
    int p = atoi(argv[2]);

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
    int *sum;
    int *col;
    int size = n - 1 + n - 1 + 1;
    // // setting up left and right doaginal
    // if p > n find all possble combination of queens in the first 2 columns
    int **board1;
    int *ld1;
    int *rd1;
    int *cl1;
    ld1 = (int *)malloc(size * sizeof(int));
    rd1 = (int *)malloc(size * sizeof(int));
    board1 = (int **)malloc(n * sizeof(int *));
    for (int j = 0; j < n; j++)
    {
        board1[j] = (int *)malloc(n * sizeof(int));
        for (int k = 0; k < n; k++)
        {
            board1[j][k] = 0;
        }
    }

    for (int i = 0; i < size; i++)
    {
        ld1[i] = 0;
        rd1[i] = 0;
    }

    cl1 = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++)
    {
        cl1[i] = 0;
    }
    if (p > n)
    {
        GM *arg = malloc(sizeof(*arg));
        solve_sub(arg, board1, ld1, rd1, cl1, 0, n, size, 0);
    }
    int max = (n >= no_board) ? n : no_board;

    ld = (int **)malloc(max * sizeof(int *));
    rd = (int **)malloc(max * sizeof(int *));
    for (int j = 0; j < max; j++)
    {
        ld[j] = (int *)malloc(size * sizeof(int));
        rd[j] = (int *)malloc(size * sizeof(int));
        for (int i = 0; i < size; i++)
        {
            ld[j][i] = 0;
            rd[j][i] = 0;
        }
    }

    // setting up rows to be checked
    cl = (int **)malloc(max * sizeof(int *));
    for (int i = 0; i < max; i++)
    {
        cl[i] = (int *)malloc(n * sizeof(int));
        for (int j = 0; j < n; j++)
        {
            cl[i][j] = 0;
        }
    }
    sum = (int *)malloc(max * sizeof(int));
    for (int j = 0; j < max; j++)
    {
        sum[j] = 0;
    }

    // // setting up columns for each board
    col = (int *)malloc(max * sizeof(int));
    for (int i = 0; i < max; i++)
    {
        col[i] = 0;
    }

    board = (int ***)malloc(max * sizeof(int **));
    for (int i = 0; i < max; i++)
    {
        board[i] = (int **)malloc(n * sizeof(int *));
        for (int j = 0; j < n; j++)
        {
            board[i][j] = (int *)malloc(n * sizeof(int));
            for (int k = 0; k < n; k++)
            {
                board[i][j][k] = 0;
            }
        }
    }

    maxBoardPtr = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
        maxBoardPtr[i] = (int *)malloc(n * sizeof(int));
        for (int j = 0; j < n; j++)
        {
            maxBoardPtr[i][j] = 0;
        }
    }

    //  setting all board
    if (max == no_board)
    {
        for (int j = 0; j < n; j++)
        {
            for (int k = 0; k < n; k++)
            {
                board1[j][k] = 0;
            }
        }

        for (int i = 0; i < size; i++)
        {
            ld1[i] = 0;
            rd1[i] = 0;
        }

        for (int i = 0; i < n; i++)
        {
            cl1[i] = 0;
        }

        for (int i = 0; i < max; i++)
        {
            col[i] = 2;
        }
        GM *arg = malloc(sizeof(*arg));
        arg->board = board;
        arg->ld = ld;
        arg->rd = rd;
        arg->cl = cl;
        arg->col = col;
        arg->sum = sum;

        solve_sub(arg, board1, ld1, rd1, cl1, 0, n, size, 1);
    }
    // setting all board done

    struct timespec ts3, ts4;
    double timeCompute, timeFinish;

    pthread_barrier_init(&barrier, NULL, p);

    pthread_t *threads = malloc(p * sizeof(threads));
    int block = max / p;
    int remain = max % p; // if p % 2 != 0 the remainder
    int extra_added = 0;  // used to count row_start;

    //Start Init Time
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    for (int i = 0; i < p; i++)
    {
        GM *arg = malloc(sizeof(*arg));
        arg->row_start = block * i + extra_added;
        // printf(" row start is %d \n", arg->row_start);
        arg->row_end = arg->row_start + block - 1;
        if (remain != 0)
        {
            arg->row_end += 1;
            remain -= 1;
            extra_added += 1;
        }
        // printf(" row end is %d \n", arg->row_end);
        if (max == n)
        {
            for (int j = arg->row_start; j <= arg->row_end; j++)
            {
                board[j][j][0] = 1;
                cl[j][j] = 1;
                rd[j][j] = 1;
                ld[j][j + n - 1] = 1;
                col[j] = 1;
                sum[j] = j; //Set sum to abs(j-0);
            }
        }
        arg->board = board;
        arg->col = col; 
        arg->cl = cl;
        arg->rd = rd;
        arg->ld = ld;
        arg->n = n;
        arg->pid = i;
        arg->sum = sum;
        arg->total_board = no_board;
        pthread_create(&threads[i], NULL, Parallel, arg);
    }
    for (int i = 0; i < p; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &ts3);
    timeCompute =
        BILLION * (ts3.tv_sec - ts2.tv_sec) + (ts3.tv_nsec - ts2.tv_nsec);
    timeCompute = timeCompute / BILLION;

    printSolution(n);

    clock_gettime(CLOCK_MONOTONIC, &ts4);
    timeFinish =
        BILLION * (ts4.tv_sec - ts3.tv_sec) + (ts4.tv_nsec - ts3.tv_nsec);
    timeFinish = timeFinish / BILLION;

    printf("Time Data:\n");
    printf("  Initialization Time: %lf seconds\n", timeInit);
    printf("  Computation Time: %lf seconds\n", timeCompute);
    printf("  Finish Time: %lf seconds\n", timeFinish);
    printf("  Total Elapsed Time: %lf seconds\n",
           timeInit + timeCompute + timeFinish);
    return 0;
}