#include "mytar.h"
#include <stdio.h>

void format_permissions(char *permissions, const char *mode_str) {
  int mode;
  permissions[0] = mode_str[0];
  mode = strtol(mode_str + 1, NULL, 8);

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

void get_time(long int time) {
  char time_str[17];
  time_t t = (time_t)time;

  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M ", localtime(&t));
}

/* Prints the contents of an archive file */
void print_header(char *path, hPtr header, int tarFD, int v) {
  struct stat fileStat;
  char permissions[11];
  char owner[18];
  char filesize[9];
  char mtime[17];
  char time_buffer[80];

  /* Read file information */
  if (fstat(tarFD, &fileStat) == -1) {
      perror("Error reading file information");
      exit(EXIT_FAILURE);
  }

  /* Format permissions string */
  format_permissions(permissions, header->mode);

  /* Format owner string */
  snprintf(owner, 64, "%s/%s", header->uname, header->gname);

  /* Format size string */
  snprintf(filesize, 32, "%9lu", (long int) strtol(header->size, NULL, 8));

  /* Format time string */
  get_time(strtol(header->mtime, NULL, 8));
  snprintf(mtime, sizeof(mtime), "%s", time_buffer);

  /* Print file information */
  if (v) {
    printf("%s %s %s %s %s -> ", permissions, owner, filesize, mtime, path);
  } else {
    printf("%s\n", path);
  }
}

char *getPath(char *prefix, char *name)
{
    char *fullPath = (char *)calloc(strlen(prefix) + strlen(name) + 1, sizeof(char ));
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

void openTar(char *tarfile, int argc, char *argv[], int v) {
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
        if (strcmp(header->name, argv[i]) == 0) {
          /* Found the file */
          print_header(header->name, header, tarFD, v);
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
      path = getPath(header->prefix, header->name);
      print_header(path, header, tarFD, v);
      free(path);
    }
  }
}


void list_archive(char *tarfile, int argc, char *argv[], int verbose) {
  if(argc <= 2){
    perror("Missing arguments\n");
    exit(EXIT_FAILURE);
  }else{
    openTar(tarfile, argc, argv, verbose);
  }
}
