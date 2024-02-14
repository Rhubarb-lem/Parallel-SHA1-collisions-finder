#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <mpi.h>

#define STR_LENGTH 5

void gen(int lb, int ub, const char *searchString,  int *localCount)
{
    char buffer[STR_LENGTH + 1];
    buffer[STR_LENGTH] = '\0';

    unsigned char hash[SHA_DIGEST_LENGTH];
    int count = 0;

    void generate(int index, char prevChar)
    {
        for (char j = prevChar + 1; j <= 122; j++)
        {
            buffer[index] = (char)j;
            if (index == STR_LENGTH - 1)
            {
                SHA1((const unsigned char *)buffer, STR_LENGTH, hash);
                char hex_hash[2 * SHA_DIGEST_LENGTH + 1];
                for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
                {
                    sprintf(hex_hash + (2 * i), "%02x", hash[i]);
                }

                if (strcmp(hex_hash, searchString) == 0)
                {
                    printf("Find collision!:%s\n", buffer);
                    count++;
                }
            }
            else
            {
                generate(index + 1, j);
            }
        }
    }

    for (char c = lb; c <= ub; c++)
    {
        buffer[0] = (char)c;
        generate(1, 47);
    }
    *localCount = count;
}

int main(int argc, char *argv[])
{
    int rank, num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    double t = MPI_Wtime();

    char searchHash[2 * SHA_DIGEST_LENGTH + 1];

    char inputString[] = "abcde";
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char *)inputString, strlen(inputString), hash);
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        sprintf(searchHash + (2 * i), "%02x", hash[i]);
    }

    int chunk = (122 - 48) / num_procs;
    int lb = 48 + rank * chunk;
    int ub = lb + chunk;
    if (rank < (122 - 48) % num_procs)
    {
        lb++;
    }

  
    int localCount;
    printf("P = %d\n", num_procs);
    gen(lb, ub, searchHash, &localCount);
    printf("(%d)\n", localCount);

    t = MPI_Wtime() - t;
    printf(":%f:\n", t);

    MPI_Finalize();
    return 0;
}
