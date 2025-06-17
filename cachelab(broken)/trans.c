/* 
 * name: Qianhe Xiao
 * loginID: qianhe
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int k, i, j;
    int a, b, c, d, e, f, g, h;
    
    if (M == 32 && N == 32) {
        // use 8x8 block size for 32x32 matrix
        for(i = 0; i < 32; i += 8) {
            for(j = 0; j < 32; j += 8) {
                for(k = i; k < i + 8; k++) {
                    // read 8 elements from A
                    a = A[k][j];   b = A[k][j+1];
                    c = A[k][j+2]; d = A[k][j+3];
                    e = A[k][j+4]; f = A[k][j+5];
                    g = A[k][j+6]; h = A[k][j+7];

                    // write 8 elements to B
                    B[j][k] = a;   B[j+1][k] = b;
                    B[j+2][k] = c; B[j+3][k] = d;
                    B[j+4][k] = e; B[j+5][k] = f;
                    B[j+6][k] = g; B[j+7][k] = h;
                }
            }
        }
    }

    if (M == 64 && N == 64) {
        for (i = 0; i < 64; i += 8) {
            for (j = 0; j < 64; j += 8) {
                // Step 1: Handle top-left 4×4 and temporarily store bottom-left 4×4 in top-right
                for (k = i; k < i + 4; k++) {
                    a = A[k][j];     b = A[k][j+1];   c = A[k][j+2];   d = A[k][j+3];
                    e = A[k][j+4];   f = A[k][j+5];   g = A[k][j+6];   h = A[k][j+7];
                    
                    // Transpose top-left 4×4 to correct position
                    B[j][k] = a;     B[j+1][k] = b;   B[j+2][k] = c;   B[j+3][k] = d;
                    
                    // Temporarily store transposed bottom-right 4×4 in top-right position
                    B[j][k+4] = e;   B[j+1][k+4] = f; B[j+2][k+4] = g; B[j+3][k+4] = h;
                }
                
                // Step 2: Move bottom-left 4×4 from A and swap with misplaced data in B
                for (k = 0; k < 4; k++) {
                    // Read bottom-left 4×4 from A (column-wise)
                    a = A[i+4][j+k];   b = A[i+5][j+k];   c = A[i+6][j+k];   d = A[i+7][j+k];
                    
                    // Read misplaced data from B's top-right
                    e = B[j+k][i+4];   f = B[j+k][i+5];   g = B[j+k][i+6];   h = B[j+k][i+7];
                    
                    // Write bottom-left to correct position in B (top-right)
                    B[j+k][i+4] = a;   B[j+k][i+5] = b;   B[j+k][i+6] = c;   B[j+k][i+7] = d;
                    
                    // Write misplaced data to correct position (bottom-left)
                    B[j+k+4][i] = e;   B[j+k+4][i+1] = f; B[j+k+4][i+2] = g; B[j+k+4][i+3] = h;
                }
                
                // Step 3: Handle bottom-right 4×4 directly
                for (k = i + 4; k < i + 8; k++) {
                    a = A[k][j+4];   b = A[k][j+5];   c = A[k][j+6];   d = A[k][j+7];
                    B[j+4][k] = a;   B[j+5][k] = b;   B[j+6][k] = c;   B[j+7][k] = d;
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

