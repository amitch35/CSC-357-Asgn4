#include "mytar.h"

void parse_mode(mode_t *mode, const char *mode_str) {
  sscanf(mode_str, "%o", mode);
}

void format_permissions(char *permissions, const char *mode_str) {
    permissions[0] = mode_str[0];
    int mode = strtol(mode_str + 1, NULL, 8);

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

void print_time(long int time) {
   char time_str[17];
   time_t t = (time_t)time;

   strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M ", localtime(&t));

   printf("%s", time_str);
}

char *getPath(char *prefix, char *name) {
  char *fullPath =
      (char *)calloc(strlen(prefix) + strlen(name) + 1, sizeof(char));
  /* If prefix is not empty */
  if (prefix[0]) {
    strcpy(fullPath, prefix);
    /* Append a '/' */
    strcat(fullPath, "/");
  }
  strcat(fullPath, name);
  return fullPath;
}

void print_header(char *tarfile, int verbose, int strict, hEntry *entry, char *prefix) {
    char permissions[11];
    char mtime[18];
    mode_t mode;
    struct tm tm_time;
    char *fullPath;

    if (prefix == NULL){
        prefix = "";
    }

    if (verbose) {
        parse_mode(&mode, entry->mode);
        format_permissions(permissions, entry->typeflag);
        print_owner(entry->gid);
        print_time(atoi(entry.mtime));

        fullPath = getPath(prefix, entry->name);

        printf("%s %s/%s %8s %s %s\n", permissions, entry->uid, entry->gid,
                entry->size, mtime, fullPath);

        free(fullPath);
    } else {
        printf("%s\n", entry->name);
    }
}
