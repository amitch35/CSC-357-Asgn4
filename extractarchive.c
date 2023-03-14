#ifndef MYTAR_H
#include "mytar.h"
#endif

/* Extract symbolic links */
void extractLD(hPtr header) {
  mode_t mode;
  uid_t uid;
  gid_t gid;

  /* Converting mode string */
  mode = strtol(header->mode, NULL, 8);
  /* Set mode */
  if (chmod(header->name, mode) == -1) {
    perror("chmod error\n");
  }
  /* Converting id strings  */
  uid = strtol(header->uid, NULL, 8);
  gid = strtol(header->gid, NULL, 8);
  /* Setting ownerships*/
  if (chown(header->name, uid, gid) < 0) {
    perror("chown error\n");
  }

  if (S_IFLNK) {
    /* Create a symbolic link */
    symlink(header->linkname, header->name);
  }
  if (S_IFDIR) {
    /* Create directory */
    mkdir(header->name, mode);
  }
}

void extractFile(hPtr header, int tarFD, int fdOut) {
  char buffer[BLOCK_SIZE];
  int bytesRead, bytesWritten, outFileFD;
  int mode = strtol(header->mode, NULL, 8);
  outFileFD = open(header->name, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

/* Extract all files */
void extract(char *path, hPtr header, int tarFD, int v) {
  /* Print name of file */
  if (v) {
    printf("%s\n", path);
  }
  if (header->typeflag == REGULAR_FILE) {
    /* Extract file */
    // Open the file first
    // extractFile(path, header);
  }
  if (header->typeflag == DIRECTORY || header->typeflag == SYM_LINK) {
    /* Extract directory or link */
    extractLD(header);
  }
}

/* Open tarfile */
void openTar(char *tarfile, int argc, char *argv[], int v) {
  int tarFD, i, num, result;
  hPtr header = {0};
  char *path;
  /* Open tarfile */
  tarFD = open(tarfile, O_RDONLY);
  if (tarFD < 0) {
    perror("unable to open tarfile\n");
    exit(1);
  }
  /* Read all files in the archive */
  while ((num = read(tarFD, header, BLOCK_SIZE)) > 0) {
    /* Reached the end */
    if (!header->name[0]) {
      break;
    }
    /* Get full path name */
    path = getPath(header->prefix, header->name);
    /* If no files specified */
    if (argc < 4) {
      /* Extract all files */
      extract(path, header, tarFD, v);

    } else {
      /* Look for the named files */
      for (i = 3; i < argc; i++) {
        /* Extract each named file */
      }
    }
  }
}

void extractArchive(char *tarfile, int argc, char *argv[], int v) {
  int i;
  if (argc < 2) {
    perror("Not enough arguments\n");
    exit(1);
  } else {
    /* Open tarfile */
    openTar(tarfile, argc, argv, v);
  }
}