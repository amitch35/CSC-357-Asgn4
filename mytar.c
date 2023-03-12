/* Include necessary headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <inttypes.h>
#include <dirent.h>

/* #define _GNU_SOURCE // TODO: IDK if needed */

//Define constants
#define BLOCK_SIZE 512
#define HEADER_SIZE 512
#define NAME_SIZE 100

// TODO: vvv
#define MT_NAMELEN  100
#define MT_MODELEN    8
#define MT_UIDLEN     8
#define MT_GIDLEN     8
#define MT_SIZELEN   12
#define MT_MTIMELEN  12
#define MT_CSUMLEN    8
#define MT_TYPELEN    1
#define MT_LINKLEN  100
#define MT_MAGLEN     6
#define MT_VERLEN     2
#define MT_UNAMLEN   32
#define MT_GNAMLEN   32
#define MT_MAJLEN     8
#define MT_MINLEN     8
#define MT_PFXLEN   155
// TODO: ^^^

#define MIN(a,b) (((a)<(b))?(a):(b))

//Define functions
void create_archive(char *tarfile, char **paths, int num_paths, int verbose);
void list_contents(char *tarfile, int verbose);
void extract_archive(char *tarfile, int verbose, int strict);

//Main function
int main(int argc, char **argv) {
//Declare variables
char *tarfile;
char **paths;
int num_paths = 0;
int verbose = 0;
int strict = 0;
int cflag = 0;
int tflag = 0;
int xflag = 0;
int k = 0;

if (argc < 2) {
        fprintf(stderr, "Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
        exit(-1);
}

//Parse command-line arguments
while ((argv[1][k]) != '\0') {
    //printf("Switch with opt: %c\n", argv[1][k]); // TODO: remove
    switch (argv[1][k]) {
        case 'c':
            cflag = 1;
            break;
        case 't':
            tflag = 1;
            break;
        case 'x':
            xflag = 1;
            break;
        case 'v':
            verbose++;
            break;
        case 'S':
            strict = 1;
            break;
        case 's':
            strict = 1;
            break;
        case 'f':
            tarfile = argv[2];
            break;
        default:
            //Print usage message
            fprintf(stderr, "Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
            exit(EXIT_FAILURE);
    }
    k++;
}

//Check for required options
if (!cflag && !tflag && !xflag) {
    //Print usage message
    printf("C: %d, T: %d, X: %d flags", cflag, tflag, xflag); //TODO: remove
    fprintf(stderr, "Usage1: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
    exit(EXIT_FAILURE);
}

//Check if file name is provided
if (tarfile == NULL) {
    //Print usage message
    fprintf(stderr, "Usage2: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
    exit(EXIT_FAILURE);
}

//Check for paths
if (3 < argc) {
    num_paths = argc - 3;
    paths = &argv[3];
}

//Dispatch command based on options
if (cflag) {
    //printf("CALL TO create_archive\n"); // TODO: Remove
    create_archive(tarfile, paths, num_paths, verbose);
} else if (tflag) {
    printf("CALL TO list_contents\n"); // TODO: Remove
    //list_contents(tarfile, verbose);
} else if (xflag) {
    printf("CALL TO extract_archive\n"); // TODO: Remove
    //extract_archive(tarfile, verbose, strict);
}

return 0;
}

