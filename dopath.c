#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

static int                                      /* we return whatever func() returns */
dopath(char* fullpath)
{
    struct stat             statbuf;
    struct dirent   *dirp;
    DIR                             *dp;
    int                             ret, n;

    if (lstat(fullpath, &statbuf) < 0)      /* stat error */
        return(func(fullpath, &statbuf, FTW_NS));
    if (S_ISDIR(statbuf.st_mode) == 0)      /* not a directory */
        return(func(fullpath, &statbuf, FTW_F));

    /*
     * It's a directory.  First call func() for the directory,
     * then process each filename in the directory.
     */
    if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
        return(ret);

    n = strlen(fullpath);
    if (n + NAME_MAX + 2 > pathlen) {       /* expand path buffer */
        pathlen *= 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL)
            err_sys("realloc failed");
    }
    fullpath[n++] = '/';
    fullpath[n] = 0;

    if ((dp = opendir(fullpath)) == NULL)   /* can't read directory */
        return(func(fullpath, &statbuf, FTW_DNR));

    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0  ||
            strcmp(dirp->d_name, "..") == 0)
            continue;               /* ignore dot and dot-dot */
        strcpy(&fullpath[n], dirp->d_name);     /* append name after "/" */
        if ((ret = dopath(func)) != 0)          /* recursive */
            break;  /* time to leave */
    }
    fullpath[n-1] = 0;      /* erase everything from slash onward */

    if (closedir(dp) < 0)
        perror("can't close directory %s", fullpath);
    return(ret);
}