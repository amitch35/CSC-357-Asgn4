#include "mytar.h"

void setPermissions(const char* path, mode_t previousMode)
{
    struct stat fileStat;

    /* Get the current mode of the file */
    if (stat(path, &fileStat) != 0) {
        perror("Error getting file stat - setPermissions");
        exit(EXIT_FAILURE);
    }

    /* Determine the new mode for the file */
    mode_t newMode = previousMode & 07777; // Mask off any extra bits
    if (S_ISDIR(fileStat.st_mode)) {
        /* Add execute permission for directories */
        newMode |= S_IXUSR | S_IXGRP | S_IXOTH; 
    }
    if (newMode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
        /* Add read permission for files with execute bits */
        newMode |= S_IRUSR | S_IRGRP | S_IROTH;
    }

    /* Set the new mode for the file */
    if (chmod(path, newMode) != 0) {
        perror("Error setting file mode - setPermissions");
        exit(EXIT_FAILURE);
    }
}

void setFileSettings(hPtr header, char *path){
    mode_t mode;
    //gid_t gid; 
    struct utimbuf times;
    struct tm* mtimeTM;

    /* Converting mode string */
    mode = strtol(header->mode, NULL, 8);
    /* Set mode */
    setPermissions(path, mode);

    /* Set the modification time */
    time_t mtime = strtol(header->mtime, NULL, 8); /* convert to time_t val */
#ifdef TEST
    printf("Set file settings for %s\n", path);
    char time_str[17];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M ", localtime(&mtime));
    printf("Time aquired: %s\n", time_str);
#endif
    /* convert mtime to tm struct */
    mtimeTM = localtime(&mtime);
#ifdef TEST
    printf("Time set: %s", asctime(mtimeTM));
#endif
    /* Set access and modification times */
    times.actime = time(NULL); /* Use current access time */
    times.modtime = mktime(mtimeTM);
    /* Change the modification time of the file */
    if(utime(path, &times) != 0){
        perror("Error changing modification time");
    }

    /* Converting id strings /// 
    gid = strtol(header->gid, NULL, 8);
    // Setting ownerships  - Note: Cannot change uid *
    if (lchown(path, -1, gid) < 0) {
        perror("chown error");
    } */
}

/* Extract symbolic links */
/* directory or link and recursively call extract_all if directory */
void extractLD(hPtr header, int v) {
    mode_t mode;
    char *path;

    /* Get full path name */
    path = getPath(header->prefix, header->name);

    /* Converting mode string */
    mode = strtol(header->mode, NULL, 8);

    /* If verbose print path */
    if (v) {
        printf("%s\n", path);
    }
    
    if (S_ISLNK(mode)) {
        /* Create a symbolic link */
        symlink(header->linkname, path);
        //setFileSettings(header, path); // TODO
    }
    else {
        /* Create directory */
        char dirname[NAME_MAX];
        strcpy(dirname, path);
        *(strrchr(dirname, '/')) = '\0';
#ifdef TEST
        printf("Make dir with: %s\n", dirname);
#endif
        if (mkdir(dirname, mode) != 0){
            perror("Couldn't make directory");
            exit(EXIT_FAILURE);
        }
        setFileSettings(header, dirname); // TODO
    }
}

void extractFile(int tarFD, hPtr header, int v) {
    char buffer[BLOCK_SIZE];
    char *path;
    int total_bytes_written = 0;
    int bytes_read, bytes_written, outFileFD;
    int mode = strtol(header->mode, NULL, 8);
    int size = strtol(header->size, NULL, 8);

    /* Get full path name */
    path = getPath(header->prefix, header->name);
    
    /* If verbose print path */
    if (v) {
        printf("%s\n", path);
    }

#ifdef TEST
    printf("Opening file called: %s", path);
#endif
    
    outFileFD = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, mode);
    if (outFileFD == -1) {
        perror("Error opening file\n");
    }
    
    // Write file contents to file
    while ((total_bytes_written < size) &&
            (bytes_read = read(tarFD, buffer, BLOCK_SIZE)) > 0) {
        bytes_written = write(outFileFD, buffer, 
                MIN(bytes_read, (size - total_bytes_written)));
        total_bytes_written += bytes_written;
    }
    
    setFileSettings(header, path);
}

void extract_subset(char *tarfile, int tarFD, char **paths, int num_paths, 
        int v){
    int num;
    hPtr header = {0};
    /* Read all files in the archive */
    while ((num = read(tarFD, header, BLOCK_SIZE)) > 0) {
        /* Reached the end */
        if (!header->name[0]) { /* if first char in header is '\0' */
          break;
        }
        /* Look for the named files in order they appear in archive */
        
    }

}

void extract_all(char *tarfile, int tarFD, int v){
    int num;
    hPtr header;
    header = malloc(HEADER_SIZE);
    header = memset(header, 0, HEADER_SIZE);
#ifdef TEST
    printf("CALL TO extract_all\n"); 
#endif
    /* Read all files in the archive */
    while ((num = read(tarFD, header, BLOCK_SIZE)) > 0) {
#ifdef TEST
        printf("%d bytes read from archive\n", num); 
#endif
        /* Reached the end */
        if (!header->name[0]) { /* if first char in header is '\0' */
          break;
        }
        if (header->typeflag == REGULAR_FILE || 
                    header->typeflag == REGULAR_FILE_ALT) {
            /* Extract file */
            /* open file (w/ settings) & write content */
#ifdef TEST
            printf("Extract file\n"); 
#endif
            extractFile(tarFD, header, v);
        }
        if (header->typeflag == DIRECTORY || header->typeflag == SYM_LINK) {
            /* Extract Link or directory */
#ifdef TEST
            printf("Extract directory or link\n"); 
#endif
            extractLD(header, v);
        }
    }
    if (num == -1){
        perror("Error reading from tarfile");
        exit(EXIT_FAILURE);
    }
}

void extractArchive(char *tarfile,  char **paths, int num_paths, int v) {
    /* Open tarfile */
    int tarFD;
#ifdef TEST
        printf("Extracting from %s\n", tarfile); 
#endif
    tarFD = open(tarfile, O_RDONLY);
    if (tarFD < 0) {
        perror("unable to open tarfile\n");
        exit(EXIT_FAILURE);
    }

    /* If specific paths are given */
    if (num_paths > 0) {
        /* Extract each named file */
        // extract_subset(tarfile, tarFD, paths, num_paths, v); // TODO
#ifdef TEST
        printf("num_path greater than zero\n"); 
#endif
    } else {
        /* Extract all files */
        extract_all(tarfile, tarFD, v);
    }
}