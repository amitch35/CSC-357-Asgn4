// /*main*/
// int main(int argc, char *argv[]) {
//   int inFd, outFd;
//   int flags[5] = {0};
//   int opt, count = 0;
//   int i;
//   hEntry header;

//   /* Check for mandatory arguments */
//   if (argc < 3) {
//     fprintf(stderr, "Usage: %s [ctxvS]f tarfile [path...]\n", argv[0]);
//     exit(EXIT_FAILURE);
//   }

//   /* Parse commands*/
//   while ((opt = getopt(argc, argv, "ctxvSf:")) != -1) {
//     switch (opt) {
//     case 'c':
//       flags[CREATE] = 1;
//       break;
//     case 't':
//       flags[LIST] = 1;
//       break;
//     case 'x':
//       flags[EXTRACT] = 1;
//       break;
//     case 'v':
//       flags[VERBOSE] = 1;
//       break;
//     case 'S':
//       flags[STRICT] = 1;
//       break;
//     case 'f':
//       flags[ARCHIVE] = 1;
//       strncpy(header.name, optarg, 100);
//       break;
//     default:
//       fprintf(stderr, "Unknown option: %c\n", optopt);
//       exit(EXIT_FAILURE);
//     }
//   }

//   /* Check for duplicate options */
//   for (i = 0; i < 3; i++) {
//     if (flags[i]) {
//       count++;
//     }
//   }
//   if (count != 1) {
//     fprintf(stderr, "Exactly one of c, t, or x must be specified\n");
//     exit(EXIT_FAILURE);
//   }

//   /* Open input and output files */
//   if (flags[CREATE]) {
//     outFd = open(header.name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
//     if (outFd == -1) {
//       perror("Error opening output file");
//       exit(EXIT_FAILURE);
//     }

//     create_archive(outFd, argc, argv, optind, flags[VERBOSE]);
//   }

//   if (flags[LIST]) {
//     inFd = open(header.name, O_RDONLY);
//     if (inFd == -1) {
//       perror("Error opening input file");
//       exit(EXIT_FAILURE);
//     }
//   }

//   if (flags[EXTRACT]) {
//     inFd = open(header.name, O_RDONLY);
//     if (inFd == -1) {
//       perror("Error opening input file");
//       exit(EXIT_FAILURE);
//     }
//   }

//   if (!flags[CREATE] && !flags[LIST] && !flags[EXTRACT]) {
//     fprintf(stderr, "No valid option specified\n");
//     exit(EXIT_FAILURE);
//   }

//   /* Cleanup */
//   if (flags[CREATE]) {
//     close(outFd);
//   } else if (flags[LIST] || flags[EXTRACT]) {
//     close(inFd);
//   }

//   return 0;
// }

// /*create archive*/
// #include "archive.h"
// #include "mytar.h"
// #include "util.h"

// void createarchive(int outFd, int argc, char *argv[], int verbose) {
//   /*Loop through remaining arguments and add files/directories to archive*/
//   for (int i = optind; i < argc; i++) {
//     add_to_archive(outFd, argv[i], verbose);
//   }
// }

// void write_header(int outFd, char *path, struct stat st) {
//   /*Write header*/
//   hEntry header;
//   memset(&header, 0, sizeof(hEntry));
//   strncpy(header.name, path, NAME_LENGTH);
//   sprintf(header.mode, "%07o", st.st_mode & 07777);
//   sprintf(header.uid, "%07o", st.st_uid);
//   sprintf(header.gid, "%07o", st.st_gid);
//   sprintf(header.size, "%011llo", (long long)st.st_size);
//   sprintf(header.mtime, "%011lo", (long)st.st_mtime);
//   memset(header.chksum, ' ', CHKSUM_LENGTH);
//   header.typeflag = get_file_type(st);
//   strncpy(header.magic, "ustar", MAGIC_LENGTH);
//   strncpy(header.version, "00", VERSION_LENGTH);
//   strncpy(header.uname, "user", UNAME_LENGTH);
//   strncpy(header.gname, "user", GNAME_LENGTH);
//   strncpy(header.devmajor, "0000000", DEVMAJOR_LENGTH);
//   strncpy(header.devminor, "0000000", DEVMINOR_LENGTH);
//   strncpy(header.prefix, "", PREFIX_LENGTH);
//   uint32_t checksum = calculate_checksum(&header);
//   sprintf(header.chksum, "%06o", checksum);

//   if (write(outFd, &header, sizeof(hEntry)) == -1) {
//     perror("write");
//     exit(EXIT_FAILURE);
//   }
// }

// void add_directory_to_archive(int outFd, char *path, int verbose) {
//   DIR *dir = opendir(path);
//   if (dir == NULL) {
//     perror("opendir");
//     return;
//   }
//   struct dirent *entry;
//   while ((entry = readdir(dir)) != NULL) {
//     if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
//       continue;
//     }
//     char *subpath = join_path(path, entry->d_name);
//     add_to_archive(outFd, subpath, verbose);
//     free(subpath);
//   }
//   closedir(dir);
// }

// void add_file_to_archive(int outFd, char *path) {
//   int inFd = open(path, O_RDONLY);
//   if (inFd == -1) {
//     perror("open");
//     return;
//   }
//   char buf[BUFSIZ];
//   ssize_t nread;
//   while ((nread = read(inFd, buf, sizeof(buf))) > 0) {
//     if (write(outFd, buf, nread) == -1) {
//       perror("write");
//       exit(EXIT_FAILURE);
//     }
//   }
//   close(inFd);
// }

// void add_to_archive(int outFd, char *path, int verbose) {
//   struct stat st;
//   if (lstat(path, &st) == -1) {
//     perror("lstat");
//     return;
//   }

//   write_header(outFd, path, st);

//   if (verbose) {
//     printf("%s\n", path);
//   }

//   if (S_ISDIR(st.st_mode)) {
//     add_directory_to_archive(outFd, path, verbose);
//   } else {
//     add_file_to_archive(outFd, path);
//   }
// }
