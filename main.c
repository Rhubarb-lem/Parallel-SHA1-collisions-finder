#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include "mpi.h"

void computeSHA1(const char *input, unsigned char *output) {
    SHA1((const unsigned char *)input, strlen(input), output);
}

void generateAndCompare(int length, const char *searchString, char startChar, char endChar) {
    char buffer[length + 1];
    buffer[length] = '\0';
    unsigned char targetHash[SHA_DIGEST_LENGTH];
    computeSHA1(searchString, targetHash);

    
    void generate(int index, char prevChar) {
        for (char c = prevChar + 1; c <= endChar; c++) {
            buffer[index] = c;

            if (index == length - 1) {
                unsigned char hash[SHA_DIGEST_LENGTH];
                computeSHA1(buffer, hash);
                if (memcmp(hash, targetHash, SHA_DIGEST_LENGTH) == 0) {
                    printf("Match found: %s\n", buffer);
                    
                    MPI_Send(buffer, length + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
                }
            } else {
                generate(index + 1, c);  
            }
        }
    }

    generate(0, startChar - 1);  
}

int main(int argc, char *argv[]) {
    int length = 4;  
    const char *searchString = "test"; 

    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int chunkSize = ('z' - 'a' + 1) / world_size;
    char startChar = 'a' + world_rank * chunkSize;
    char endChar = startChar + chunkSize - 1;
    if (world_rank == world_size - 1) {
        endChar = 'z';
    }

    if (world_rank == 0) {
        char matchedStrings[100][length + 1];  
        int matchedCount = 0;

        for (int i = 1; i < world_size; i++) {
            char buffer[length + 1];
            for (int j = 0; j < 100; j++) {
                MPI_Recv(buffer, length + 1, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                strcpy(matchedStrings[matchedCount], buffer);
                matchedCount++;
            }
        }

        printf("Match found:\n");
        for (int i = 0; i < matchedCount; i++) {
            printf("%s\n", matchedStrings[i]);
        }
    } else {
        generateAndCompare(length, searchString, startChar, endChar);
    }

    MPI_Finalize();

    return 0;
}