#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>
#include <string.h>
#include <signal.h>

int inotifyFd, wd;


static void displayInotifyEvent(struct inotify_event *i)
{
    printf("    wd =%2d; ", i->wd);
    if (i->cookie > 0)
        printf("cookie =%4d; ", i->cookie);

    printf("mask = ");
    if (i->mask & IN_ACCESS)        printf("IN_ACCESS ");
    if (i->mask & IN_ATTRIB)        printf("IN_ATTRIB ");
    if (i->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
    if (i->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE ");
    if (i->mask & IN_CREATE)        printf("IN_CREATE ");
    if (i->mask & IN_DELETE)        printf("IN_DELETE ");
    if (i->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF ");
    if (i->mask & IN_IGNORED)       printf("IN_IGNORED ");
    if (i->mask & IN_ISDIR)         printf("IN_ISDIR ");
    if (i->mask & IN_MODIFY)        printf("IN_MODIFY ");
    if (i->mask & IN_MOVE_SELF)     printf("IN_MOVE_SELF ");
    if (i->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM ");
    if (i->mask & IN_MOVED_TO)      printf("IN_MOVED_TO ");
    if (i->mask & IN_OPEN)          printf("IN_OPEN ");
    if (i->mask & IN_Q_OVERFLOW)    printf("IN_Q_OVERFLOW ");
    if (i->mask & IN_UNMOUNT)       printf("IN_UNMOUNT ");
    printf("\n");

    if (i->len > 0)
        printf("        name = %s\n", i->name);
}

static void signal_handler (int signo)
{
    if (signo == SIGINT){
        printf ("Захвачен сигнал SIGINT!\n");
        int ret = close(inotifyFd);
        if (ret == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
    }
    else if (signo == SIGTERM){
        printf ("Захвачен сигнал SIGTERM!\n Enter new path of the directory: ");
        
        char newPath[BUFSIZ];

        scanf("%s", newPath);
        
        wd = inotify_add_watch(inotifyFd, newPath, IN_ACCESS | IN_MODIFY);
        if (wd == -1) {
            perror("inotify_add_watch");
            exit(EXIT_FAILURE);
        }

        printf("Add to watching %s using wd %d\n", newPath, wd);
    }
    else {
        fprintf (stderr, "Неожиданный сигнал!\n");
        exit (EXIT_FAILURE);
    }
}

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))


int main(int argc, char **argv)
{
    int j;
    char buf[BUF_LEN] __attribute__ ((aligned(8)));
    ssize_t numRead;
    char *p;
    struct inotify_event *event;
    
    mkdir("/home/andrey/Documents/Unix/Laba/tmp/", 0777);    

    inotifyFd = inotify_init();
    if (inotifyFd == -1) {
        perror("inotify_init");
        exit(EXIT_FAILURE);

    }
        
    wd = inotify_add_watch(inotifyFd, "/home/andrey/Documents/Unix/Laba/tmp/", IN_ALL_EVENTS);
    if (wd == -1) {
        perror("inotify_add_watch");
        exit(EXIT_FAILURE);
    }

    printf("Watching %s using wd %d\n", "/home/andrey/Documents/Unix/Laba/tmp/", wd);
    
    if (signal (SIGINT, signal_handler) == SIG_ERR) {
        fprintf (stderr, "Невозможно обработать SIGINT!\n");
        exit (EXIT_FAILURE);
    }

    if (signal (SIGTERM, signal_handler) == SIG_ERR) {
        fprintf (stderr, "Невозможно обработать SIGTERM!\n");
        exit (EXIT_FAILURE);
    }
/*
    if (signal (SIGPROF, SIG_DFL) == SIG_ERR) {
        fprintf (stderr, "Невозможно сбросить SIGPROF!\n");
        exit (EXIT_FAILURE);
    }

    if (signal (SIGHUP, SIG_IGN) == SIG_ERR) {
        fprintf (stderr, "Невозможно игнорировать SIGHUP!\n");
        exit (EXIT_FAILURE);
    }
*/  
    for (;;) {
        numRead = read(inotifyFd, buf, BUF_LEN);
        if (numRead == 0)
            return 1;

        if (numRead == -1)
            return 1;

        printf("Read %ld bytes from inotify fd\n", (long) numRead);

        
        for (p = buf; p < buf + numRead; ) {
            event = (struct inotify_event *) p;
            displayInotifyEvent(event);

            p += sizeof(struct inotify_event) + event->len;
        }
    }
    
    exit(EXIT_SUCCESS);
}  
