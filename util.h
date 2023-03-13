#ifndef _MYTAR_UTILS
#define _MYTAR_UTILS
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <arpa/inet.h>

//Define constants
#define BLOCK_SIZE  512
#define HEADER_SIZE 512

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

#define CSUM_OFFSET 148

#define MAGIC "ustar\0"
#define VERSION "00"

#define REGULAR_FILE '0'
#define REGULAR_FILE_ALT '\0'
#define SYM_LINK '2'
#define DIRECTORY '5'

typedef struct header *hPtr;
typedef struct header{
    char name[MT_NAMELEN];
    char mode[MT_MODELEN];
    char uid[MT_UIDLEN];
    char gid[MT_GIDLEN];
    char size[MT_SIZELEN];
    char mtime[MT_MTIMELEN];
    char chksum[MT_CSUMLEN];
    char typeflag;
    char linkname[MT_LINKLEN];
    char magic[MT_MAGLEN];
    char version[MT_VERLEN];
    char uname[MT_UNAMLEN];
    char gname[MT_GNAMLEN];
    char devmajor[MT_MAJLEN];
    char devminor[MT_MINLEN];
    char prefix[MT_PFXLEN];
}hEntry;

#define MIN(a,b) (((a)<(b))?(a):(b))

#endif