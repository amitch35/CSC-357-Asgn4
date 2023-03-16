/* Include necessary headers */
#include "mytar.h"

// Define functions
void create_archive(char *tarfile, char **paths, int num_paths, int verbose,
                    int strict);
void list_archive(char *tarfile, int argc, char *argv[], int verbose);
void extractArchive(char *tarfile,  char **paths, int num_paths, int v);

// Main function
int main(int argc, char **argv) {
  // Declare variables
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

  // Parse command-line arguments
  while ((argv[1][k]) != '\0') {
#ifdef TEST
    printf("Switch with opt: %c\n", argv[1][k]);
#endif
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
    case 's': // tryAsgn4 uses a small s on the general tests
      strict = 1;
      break;
    case 'f':
      tarfile = argv[2];
      break;
    default:
      // Print usage message
      fprintf(stderr, "Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
      exit(EXIT_FAILURE);
    }
    k++;
  }

  // Check for required options
  if (!cflag && !tflag && !xflag) {
    // Print usage message
#ifdef TEST
    printf("C: %d, T: %d, X: %d flags", cflag, tflag, xflag);
#endif
    fprintf(stderr, "Usage1: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
    exit(EXIT_FAILURE);
  }

  // Check if file name is provided
  if (tarfile == NULL) {
    // Print usage message
    fprintf(stderr, "Usage2: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
    exit(EXIT_FAILURE);
  }

  // Check for paths
  if (3 < argc) {
    num_paths = argc - 3;
    paths = &argv[3];
  }

  // Dispatch command based on options
  if (cflag) {
#ifdef TEST
    printf("CALL TO create_archive\n");
#endif
    create_archive(tarfile, paths, num_paths, verbose, strict);
  } else if (tflag) {
#ifdef TEST
    printf("CALL TO list_archive\n");
#endif
    list_archive(tarfile, argc, argv, verbose);
  } else if (xflag) {
#ifdef TEST
    printf("CALL TO extract_archive\n");
#endif
    extractArchive(tarfile, paths, num_paths, verbose);
  }

  return 0;
}