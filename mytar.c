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
#include <pwd.h>
#include <grp.h>

#include "util.h"

//Define functions
void create_archive(char *tarfile, char **paths, int num_paths, int verbose, int strict);
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
        case 's': // tryAsgn4 uses a small s on the general tests
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
    create_archive(tarfile, paths, num_paths, verbose, strict);
} else if (tflag) {
    printf("CALL TO list_contents\n"); // TODO: Remove
    //list_contents(tarfile, verbose);
} else if (xflag) {
    printf("CALL TO extract_archive\n"); // TODO: Remove
    //extract_archive(tarfile, verbose, strict);
}

return 0;
}

int openArchive(char *tarfile){
    int tar_fd = open(tarfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (tar_fd == -1) {
        // Print error message and return if file cannot be opened
        fprintf(stderr, "Error opening file: %s\n", tarfile);
        exit(EXIT_FAILURE);
    }
    return tar_fd;
}

void splitName(char *filepath, hPtr header, struct stat st){
    char filename[NAME_MAX + 1];
    // Add trailing '/' if file is a directory
    if (S_ISDIR(st.st_mode)) {
        filename[strlen(filename)] = '/';
    }
    
    if (strlen(filename) <= MT_NAMELEN){
        /* copy filename to header name feild
         * if filename is shorter than MT_NAMELEN
         * it fills rest with '\0' */
        strncpy(header->name, filename, MT_NAMELEN);
    } else {
        char name[MT_NAMELEN];
        int i;
        strncpy(header->prefix, filename, MT_PFXLEN);
        for (i = MT_PFXLEN; i < strlen(filename); i++){
            name[i - MT_PFXLEN] = filename[i];
        }
        strncpy(header->name, name, MT_NAMELEN);
    }
}

void setType(char *filename, struct stat st, hPtr header){
    if (S_ISDIR(st.st_mode)){
        header->typeflag[0] = DIRECTORY;
        // Directories don't have a size?
        strcpy(header->size, "00000000000");
    } else if (S_ISREG(st.st_mode)){
        header->typeflag[0] = REGULAR_FILE;
        sprintf(header->size, "%011lo", st.st_size);
    } else if (S_ISLNK(st.st_mode)){
        header->typeflag[0] = SYM_LINK;
        // Symbolic links don't have a size?
        strcpy(header->size, "00000000000");
        readlink(filename, header->linkname, MT_LINKLEN);
    } else {
        // Print error message and return if file type not supported
        fprintf(stderr, "Unsupported file type: %s\n", filename);
        return;
    }
}

void setIDName(struct stat st, hPtr header){
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

int getChecksum(uint8_t *ptr){
    int checksum = 0, i;
    uint8_t *start = ptr; // Save start of header
    /* add all bytes in header */
    for (i = 0; i < HEADER_SIZE; i++){
        checksum = checksum + *ptr;
        ptr++;
    }
    /* subtract values currently in checksum (unknown) */
    for (i = 0; i < MT_CSUMLEN; i++){
        checksum = checksum - *(start + CSUM_OFFSET + i);
    }
    /* Add check sum bytes as spaces */
    checksum = checksum + (' ' * MT_CSUMLEN);
    return checksum;
}

// Define function to create a header for a file
hPtr create_header(char *filename, struct stat st) {
    int checksum;
    hPtr header;
    header = malloc(sizeof(hEntry));
    header = memset(header, 0, HEADER_SIZE);
    
    // Fill header with filename
    splitName(filename, header);

    // Add file data to header
    sprintf(header->mode, "%07o", st.st_mode & 0777);

    sprintf(header->uid, "%07o", st.st_uid);
    sprintf(header->gid, "%07o", st.st_gid);
    sprintf(header->mtime, "%011lo", st.st_mtime);
    strcpy(header->magic, MAGIC);
    strcpy(header->version, VERSION);

    // Set Type and size accordingly
    setType(filename, st, header);

    // Set uname and gname
    setIDName(st, header);
    
    // Calculate and add header checksum
    checksum = getChecksum((uint8_t *) &header);
    sprintf(header->chksum, "%07o", checksum);
}

void add_file(int tar_fd, char *filepath, int verbose) {
    // Open file and get metadata
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        // Print error message and return if file cannot be opened
        fprintf(stderr, "Error opening file: %s\n", filepath);
        return;
    }
    struct stat st;
    if (fstat(fd, &st) < 0){
        fprintf(stderr, "Error with file stat: %s\n", filepath);
        return;
    }

    // Create header for file
    hPtr header = create_header(filepath, st);

    // Write header to archive
    write(tar_fd, header, HEADER_SIZE);

    // If file has non-zero size
    if (strcmp(header->size, "00000000000") {
        // Write file contents to archive
        char buffer[BLOCK_SIZE];
        int bytes_read, bytes_written;
        while ((bytes_read = read(fd, buffer, BLOCK_SIZE)) > 0) {
            bytes_written = write(tar_fd, buffer, bytes_read);
        }
        if (bytes_written < BLOCK_SIZE){ // fill remainder of block
            char zero_block[BLOCK_SIZE] = { 0 };
            write(tar_fd, zero_block, (BLOCK_SIZE - bytes_wrtten));
        }
    }

    // Print message if verbose
    if (verbose > 0) {
        printf("%s\n", filepath);
    }

    // Close file
    close(fd);
}

void add_dir(int tar_fd, char *path, int verbose) {
    // Add the directory itself
    add_file(tar_fd, path, verbose);
    
    // Traverse directory and add files to archive
    DIR *dirp = opendir(path);
    struct dirent *entry;
    while ((entry = readdir(dirp)) != NULL) {
        // Skip '.' and '..' directories
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build full path to file
        char full_path[NAME_MAX];
        int num = snprintf(full_path, NAME_MAX, "%s/%s", path, entry->d_name);
        if (num >= NAME_MAX){
            printf("Error: The path %s is too long.\n", full_path);
            exit(EXIT_FAILURE);
        }

        // if directory
        if (entry->d_type == DT_DIR) {
            add_dir(tar_fd, full_path, verbose);
        } else if (entry->d_type == DT_LNK){
            add_lnk();
        } else { // else just a file
            add_file(tar_fd, full_path, verbose);
        }
    }

    // Close directory
    closedir(dirp);
}

void parseFile(char *path, struct stat st, int tar_fd, int verbose, int strict){
    hPtr header;
    // Check if path is a directory
    if (S_ISDIR(st.st_mode)) {
        // Add directory to archive
        add_dir(tar_fd, path, verbose);
    } else if(S_ISREG(st.st_mode)) {
        // Add file to archive
        add_file(tar_fd, path, verbose);
    } else {
        // Add link to archive 
        // TODO: add_lnk()
    }
}

// Define function to create a tar archive
void create_archive(char *tarfile, char **paths, int num_paths, int verbose, int strict) {
    // Open archive file for writing
    int tar_fd = openArchive(tarfile);

    // Process each path
    for (int i = 0; i < num_paths; i++) {
        char *path = paths[i];
        if (strlen(path) >= NAME_MAX){
            printf("Error: The path %s is too long.\n", path);
            exit(EXIT_FAILURE);
        }
        struct stat st;
        if(lstat(path, &st) > 0){ /* If stat is successful */
            parseFile(path, st, tar_fd, verbose, strict);
        }
    }

    // Write two zero blocks to signify end of archive
    char zero_block[BLOCK_SIZE] = { 0 };
    write(tar_fd, zero_block, BLOCK_SIZE);
    write(tar_fd, zero_block, BLOCK_SIZE);

    // Close archive file
    close(tar_fd);

}