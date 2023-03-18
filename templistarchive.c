#include "mytar.h"
#include <stdio.h>

void format_permissions(char *permissions, 
                      const char *mode_str, const hPtr header)  {
  int mode = strtol(mode_str, NULL, 8);
  /* Extract the file type */
  switch (header->typeflag) {
    case DIRECTORY:
      permissions[0] = 'd';
      break;
    case SYM_LINK:
      permissions[0] = 'l';
      break;
    default:
      permissions[0] = '-';
  }

  permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
  permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
  permissions[3] = (mode & S_IXUSR) ? 'x' : '-';

  permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
  permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
  permissions[6] = (mode & S_IXGRP) ? 'x' : '-';

  permissions[7] = (mode & S_IROTH) ? 'r' : '-';
  permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
  permissions[9] = (mode & S_IXOTH) ? 'x' : '-';
  permissions[10] = '\0';
}

/* Prints the contents of an archive file */
void print_header(char *path, hPtr header, int tarFD, int v) {
  struct stat fileStat;
  char permissions[11];
  char owner[18];
  char filesize[9];
  char mtime[17];
  time_t mtime_t;
  off_t size, offset;

  /* Read file information */
  if (fstat(tarFD, &fileStat) == -1) {
      perror("Error reading file information");
      exit(EXIT_FAILURE);
  }

  /* Print file information */
  if (v) {
    /* Format permissions string */
    format_permissions(permissions, header->mode, header);

    /* Format owner string */
    snprintf(owner, 64, "%s/%s", header->uname, header->gname);

    /* Format size string */
    snprintf(filesize, 9, "%lu", strtol(header->size, NULL, 8));

    /* Format time string */
    mtime_t = (time_t)strtol(header->mtime, NULL, 8);
    strftime(mtime, sizeof(mtime), "%Y-%m-%d %H:%M", localtime(&mtime_t));

    /* Print the formatted information */
    printf("%s %s %14s %s %s\n", permissions, owner, filesize, mtime, path);

    /* Skip over the file's data */
    size = strtol(header->size, NULL, 8);
    offset = lseek(tarFD, (size + (BLOCK_SIZE - 1)) / 
                                BLOCK_SIZE * BLOCK_SIZE, SEEK_CUR);
    if (offset == -1) {
        perror("Error skipping over file data");
        exit(EXIT_FAILURE);
    }
  } else {
    /* Skip over the file's data */
    size = strtol(header->size, NULL, 8);
    offset = lseek(tarFD, (size + (BLOCK_SIZE - 1)) / 
                            BLOCK_SIZE * BLOCK_SIZE, SEEK_CUR);
    if (offset == -1) {
        perror("Error skipping over file data");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", path);
  }
}

int validate_header(hPtr header) {
  uint32_t chksum = 0;
  uint32_t givenChkSum = strtol(header->chksum, NULL, 8);
  /* Compute checksum */
  chksum = getChecksum((uint8_t *)header);
#ifdef TEST
  printf("Given chksum = %d\nCalculated = %d\n => %d\n", 
        givenChkSum, chksum, (chksum != givenChkSum));
#endif

  /* Check for bad headers */
  if (chksum != givenChkSum ||
      strncmp(header->magic, LAX_MAGIC, 5) != 0 ||
      (strcmp(header->version, LAX_VERSION) != 0 &&
      strncmp(header->version, VERSION, 2) != 0) ||
      header->name[0] == '\0') {
#ifdef TEST
      printf("Given | Expected\n");
      printf("magic--> %s | %s => %d\n", 
            header->magic, LAX_MAGIC, strncmp(header->magic, LAX_MAGIC, 5));
      printf("version--> %s | %s => %d\n", 
          header->version, LAX_VERSION, 
          (strcmp(header->version, LAX_VERSION) != 0 &&
          strncmp(header->version, VERSION, 2)));
      printf("name--> %s | Not null => %d\n", 
            header->name, header->name[0] == '\0');
#endif
    return -1; /* Header is not valid */
  }

  return 0; /* Header is valid */
}


char *getPath(char *prefix, char *name)
{
    char *fullPath = malloc(NAME_MAX + 1 * sizeof(char));
    memset(fullPath, '\0', MT_CSUMLEN); /* Initialize whole thing to \0 chars */
    /* If prefix is not empty */
    if (prefix[0])
    {
      strcpy(fullPath, prefix);
      /* Append a '/' */
      strcat(fullPath, "/");
    }
    strcat(fullPath, name);
    return fullPath;
}


void openTarListing(char *tarfile, int argc, char *argv[], int v) {
  int tarFD, i, num;
  char *path;
  hPtr header = (hPtr) malloc(sizeof(hEntry));
  if (header == NULL) {
    perror("Unable to allocate memory for header\n");
    exit(EXIT_FAILURE);
  }
  
  /* Open tarfile */
  tarFD = open(tarfile, O_RDONLY);
  if (tarFD < 0) {
    perror("unable to open tarfile\n");
    exit(EXIT_FAILURE);
  }
  
  /* Check for any names given on the command line */
  if (argc > 3) {
    /* List the files in the archive */
    for (i = 3; i < argc; i++) {
      /* Search for the file in the archive */
      while ((num = read(tarFD, header, BLOCK_SIZE)) > 0) {
        /* Reached the end */
        if (!header->name[0]) {
          break;
        }
        /*new*/
        path = getPath(header->prefix, header->name);
        /* Check if header is valid */
        if (validate_header(header)) {
          perror("Malformed header found.\n");
          exit(1);
        }

        if (strcmp(path, argv[i]) == 0) {
          /* Found the file */
          printf("Given: %s\n", path);
          print_header(path, header, tarFD, v);
          break;
        }
      }
      /* If the file is not found */
      if (num == 0) {
        printf("mytar: %s: not found in archive\n", argv[i]);
      }
    }
  } else { /*no names were given*/
    /* List all files in the archive */
    while ((num = read(tarFD, header, BLOCK_SIZE)) > 0) {
      /* Reached the end */
      if (!header->name[0]) {
        break;
      }
      /*new*/
      if (validate_header(header)) {
        fprintf(stderr, "Malformed header found. Bailing.\n");
        exit(1);
      }

      path = getPath(header->prefix, header->name);
      print_header(path, header, tarFD, v);
    }
  }

  if (num < 0) {
    perror("Nothing to read\n");
    exit(EXIT_FAILURE);
  }

  close(tarFD);
}


void list_archive(char *tarfile, int argc, char *argv[], int verbose) {
  if(argc <= 2){
    perror("Missing arguments\n");
    exit(EXIT_FAILURE);
  }else{
    openTarListing(tarfile, argc, argv, verbose);
  }
}
