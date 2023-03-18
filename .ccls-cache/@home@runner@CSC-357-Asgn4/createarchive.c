#include "mytar.h"

int insert_special_int(char *where, size_t size, int32_t val) {
  /* For interoperability with GNU tar. GNU seems to
   * set the high–order bit of the first byte, then
   * treat the rest of the field as a binary integer
   * in network byte order.
   * Insert the given integer into the given field
   * using this technique. Returns 0 on success, nonzero
   * otherwise
   */
  int err = 0;
  if (val < 0 || (size < sizeof(val))) {
    /* if it’s negative, bit 31 is set and we can’t use the flag
     * if len is too small, we can’t write it. Either way, we’re
     * done.
     */
    err++;
  } else {
    /* game on....*/
    memset(where, 0, size); /* Clear out the buffer */
    *(int32_t *)(where + size - sizeof(val)) = htonl(val); /* place int */
    *where |= 0x80; /* set that high–order bit */
  }
  return err;
}

int openArchive(char *tarfile) {
  int tar_fd = open(tarfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (tar_fd == -1) {
    // Print error message and return if file cannot be opened
    fprintf(stderr, "Error opening file: %s\n", tarfile);
    exit(EXIT_FAILURE);
  }
  return tar_fd;
}

void splitName(char *filepath, hPtr header, struct stat st) {
  char filename[NAME_MAX] = {0};
  strcpy(filename, filepath);
  // Add trailing '/' if file is a directory
  if (S_ISDIR(st.st_mode)) {
#ifdef TEST
    fprintf(stdout, "%s is a dir so add slash to end of pathname\n", filepath);
#endif
    filename[strlen(filename)] = '/';
  }
#ifdef TEST
  else { // TODO remove else and return ifdef above
    fprintf(stdout, "%s is not dir\n", filepath);
  }
  printf("Adding %s to name/prefix feilds of header\n", filename);
#endif

  if (strlen(filename) <= MT_NAMELEN) {
    /* copy filename to header name feild
     * if filename is shorter than MT_NAMELEN
     * it fills rest with '\0' */
    strncpy(header->name, filename, MT_NAMELEN);
    strncpy(header->prefix, "", MT_PFXLEN);
#ifdef TEST
    printf("Name field got: %s\n", header->name);
    printf("Prefix feild got: %s\n", header->prefix);
#endif
  } else {
    char name[MT_NAMELEN];
    int i;
    strncpy(header->prefix, filename, MT_PFXLEN);
    for (i = MT_PFXLEN; i < strlen(filename); i++) {
      name[i - MT_PFXLEN] = filename[i];
    }
    strncpy(header->name, name, MT_NAMELEN);
#ifdef TEST
    printf("Name field got: %s\n", header->name);
    printf("Prefix feild got: %s\n", header->prefix);
#endif
  }
}

void setType(char *filename, struct stat st, hPtr header) {
#ifdef TEST
  printf("Getting Type, Size, and Link for: %s\n", filename);
#endif
  if (S_ISDIR(st.st_mode)) {
    header->typeflag = DIRECTORY;
    // Directories don't have a size?
    strcpy(header->size, "00000000000");
    /* Directories don't have links */
    char zero_link[MT_LINKLEN] = {0};
    strcpy(header->linkname, zero_link);
  } else if (S_ISREG(st.st_mode)) {
    header->typeflag = REGULAR_FILE;
    sprintf(header->size, "%011lo", st.st_size);
    /* regular files don't have links */
    char zero_link[MT_LINKLEN] = {0};
    strcpy(header->linkname, zero_link);
  } else if (S_ISLNK(st.st_mode)) {
    header->typeflag = SYM_LINK;
    // Symbolic links don't have a size?
    strcpy(header->size, "00000000000");
    readlink(filename, header->linkname, MT_LINKLEN);
  } else {
    // Print error message and return if file type not supported
    fprintf(stderr, "Unsupported file type: %s\n", filename);
    return;
  }
}

void setIDName(struct stat st, hPtr header) {
#ifdef TEST
  printf("Setting UID and GID\n");
#endif
  struct passwd *pentry;
  struct group *pgroup;
  /* Find uname */
  pentry = getpwuid(st.st_uid);
  if (pentry != NULL) {
    strcpy(header->uname, pentry->pw_name);
  }
  /* Find gname */
  pgroup = getgrgid(st.st_gid);
  if (pgroup != NULL) {
    strcpy(header->gname, pgroup->gr_name);
  }
}

uint32_t getChecksum(uint8_t *header) {
  uint32_t checksum = 0;
  int i;
  /* Fill the checksum field with spaces */
  memset(header + CSUM_OFFSET, ' ', MT_CSUMLEN);

  /* add all bytes in header */
  for (i = 0; i < HEADER_SIZE; i++) {
    checksum += (uint8_t)header[i];
#ifdef TEST_THIS_ONE_IS_TURNED_OFF
    printf("[%d]Chk added: %c/%d\n", i, header[i], (uint8_t)header[i]);
#endif
  }
  return checksum;
}

// Define function to create a header for a file
hPtr create_header(char *filename, struct stat st, int strict) {
  uint32_t checksum;
  hPtr header;
  header = malloc(HEADER_SIZE);
  header = memset(header, 0, HEADER_SIZE);
#ifdef TEST
  printf("Made new header struct with size %d\n", HEADER_SIZE);
#endif

  // Fill header with filename
  splitName(filename, header, st);

  // Add file data to header
  sprintf(header->mode, "%07o", st.st_mode & 0777);
#ifdef TEST
  printf("%s's mode is %07o\n", filename, st.st_mode);
  printf("Mode field got: %s\n", header->mode);
#endif

  // sprintf(header->uid, "%07o", st.st_uid);
  insert_special_int(header->uid, MT_UIDLEN, st.st_uid);
#ifdef TEST
  printf("%s's uid is %07o\n", filename, st.st_uid);
  printf("UID field got: %s\n", header->uid);
#endif
  sprintf(header->gid, "%07o", st.st_gid);
  sprintf(header->mtime, "%011lo", st.st_mtime);
  if (strict) {
    strcpy(header->magic, MAGIC);
    strcpy(header->version, VERSION);
  } else {
    strcpy(header->magic, LAX_MAGIC);
    strcpy(header->version, LAX_VERSION);
  }
#ifdef TEST
  printf("GID field got: %s\n", header->gid);
  printf("MTime field got: %s\n", header->mtime);
#endif

  // Set Type and size + link accordingly
  setType(filename, st, header);
#ifdef TEST
  printf("TypeFlag field got: %c\n", header->typeflag);
  printf("Size field got: %s\n", header->size);
  printf("Linkname field got: %s\n", header->linkname);
  printf("Magic field got: %s\n", header->magic);
  printf("Version field got: %s\n", header->version);
#endif

  // Set uname and gname
  setIDName(st, header);
#ifdef TEST
  printf("UName field got: %s\n", header->uname);
  printf("GName field got: %s\n", header->gname);
#endif

  // Calculate and add header checksum
  checksum = getChecksum((uint8_t *)header);
  sprintf(header->chksum, "%06o", checksum);
#ifdef TEST
  printf("Checksum found as %0o\n", checksum);
  printf("Checksum field got: %s\n", header->chksum);
#endif
  return header;
}

void add_file(int tar_fd, char *filepath, int verbose, int strict) {
  // Open file and get metadata
  int fd = open(filepath, O_RDONLY);
  if (fd == -1) {
    // Print error message and return if file cannot be opened
    fprintf(stderr, "Error opening file: %s\n", filepath);
    return;
  }
  struct stat st;
  if (fstat(fd, &st) < 0) {
    fprintf(stderr, "Error with file stat: %s\n", filepath);
    return;
  }

  // Create header for file
  hPtr header = create_header(filepath, st, strict);

  // Write header to archive
  write(tar_fd, header, HEADER_SIZE);

  // If file has non-zero size
  if (strcmp(header->size, "00000000000")) {
    // Write file contents to archive
    char buffer[BLOCK_SIZE];
    int bytes_read, bytes_written;
    while ((bytes_read = read(fd, buffer, BLOCK_SIZE)) > 0) {
      bytes_written = write(tar_fd, buffer, bytes_read);
    }
    if (bytes_written < BLOCK_SIZE) { // fill remainder of block
      char zero_block[BLOCK_SIZE] = {0};
      write(tar_fd, zero_block, (BLOCK_SIZE - bytes_written));
    }
  }

  // Print message if verbose
  if (verbose > 0) {
    printf("%s\n", filepath);
  }

  // Close file
  close(fd);
}

void add_dir(int tar_fd, char *path, int verbose, int strict) {
#ifdef TEST
  printf("Adding directory -> start with dir itself: %s\n", path);
#endif
  // Add the directory itself
  add_file(tar_fd, path, verbose, strict);

  // Traverse directory and add files to archive
  DIR *dirp = opendir(path);
  struct dirent *entry;
  while ((entry = readdir(dirp)) != NULL) {
    // Skip '.' and '..' directories
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    // Build full path to file
    char full_path[NAME_MAX];
    int num = snprintf(full_path, NAME_MAX, "%s/%s", path, entry->d_name);
    if (num >= NAME_MAX) {
      printf("Error: The path %s is too long.\n", full_path);
      exit(EXIT_FAILURE);
    }
#ifdef TEST
    printf("Add subdir or file: %s in %s\n", full_path, path);
#endif

    // if directory
    if (entry->d_type == DT_DIR) {
      add_dir(tar_fd, full_path, verbose, strict);
    } else if (entry->d_type == DT_LNK) {
      // TODO :add_lnk(); ?
      add_file(tar_fd, full_path, verbose, strict);
    } else { // else just a file
      add_file(tar_fd, full_path, verbose, strict);
    }
  }

  // Close directory
  closedir(dirp);
}

void parseFile(char *path, struct stat st, int tar_fd, int verbose,
               int strict) {
#ifdef TEST
  printf("Parse: %s\n", path);
#endif
  // Check if path is a directory
  if (S_ISDIR(st.st_mode)) {
    // Add directory to archive
    add_dir(tar_fd, path, verbose, strict);
  } else if (S_ISREG(st.st_mode)) {
#ifdef TEST
    printf("%s is a regular file\n", path);
#endif
    // Add file to archive
    add_file(tar_fd, path, verbose, strict);
  } else if (S_ISLNK(st.st_mode)) {
    // Add link to archive
    // TODO: add_lnk(); ?
    add_file(tar_fd, path, verbose, strict);
  } else {
    // Print error message and return if file type not supported
    fprintf(stderr, "Unsupported file type: %s\n", path);
    return;
  }
}

// Define function to create a tar archive
void create_archive(char *tarfile, char **paths, int num_paths, int verbose,
                    int strict) {
  // Open archive file for writing
  int tar_fd = openArchive(tarfile);

  // Process each path
  for (int i = 0; i < num_paths; i++) {
    char *path = paths[i];
#ifdef TEST
    printf("Processing path: %s\n", path);
#endif
    if (strlen(path) >= NAME_MAX) {
      printf("Error: The path is too long: %s\n", path);
      exit(EXIT_FAILURE);
    }
    struct stat st;
    if (lstat(path, &st) < 0) { /* If stat is successful */
#ifdef TEST
      printf("lstat() failed on: %s\n", path);
#endif
    } else {
      parseFile(path, st, tar_fd, verbose, strict);
    }
  }

  // Write two zero blocks to signify end of archive
  char zero_block[BLOCK_SIZE] = {0};
  write(tar_fd, zero_block, BLOCK_SIZE);
  write(tar_fd, zero_block, BLOCK_SIZE);

  // Close archive file
  close(tar_fd);
}