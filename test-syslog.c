/*
 *  Author: wangzb
 *  Date:   2015-03-18
 *  Func:   Test syslog
 */

#include <syslog.h>
#include <stdio.h>



void main()
{
    //openlog("test_server", LOG_PID | LOG_CONS |LOG_NDELAY | LOG_NOWAIT, LOG_SYSLOG);
    
    syslog(LOG_INFO, "xxxxxx=======>>>>\n");

    //closelog();

}
