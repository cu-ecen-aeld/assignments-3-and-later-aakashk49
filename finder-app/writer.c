#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>

int main(int argc, char* argv[]){
    openlog("myapp", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_ERR, "This is an informational message");
    if(argc < 3){
        printf("Enter file name and string to write\n");
        return 1;
    }
    int fd = open(argv[1],O_RDWR|O_CREAT);
    if(fd == -1){
        printf("Could not open file %s\n",argv[1]);
        syslog(LOG_ERR,"Could not open file %s\n",argv[1]);
        return 1;
    }else{
        int slen = strlen(argv[2]);

        int wr = write(fd,argv[2],slen);
        syslog(LOG_DEBUG, "Writing %s to %s",argv[2],argv[1]);
        printf("Write return = %d\n",wr);
    }
    close(fd);
}