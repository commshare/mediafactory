/*

  Copyright (c) 2015 Martin Sustrik

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <string>
#include "rtspserver.h"
#include "st.h"

#include <signal.h>
#include <execinfo.h>

static void SegvHandler(int signum) {
    void *array[10];
    size_t size;
    char **strings;
    size_t i, j;

    signal(signum, SIG_DFL);

    size = backtrace (array, 10);
    strings = (char **)backtrace_symbols (array, size);

    fprintf(stderr, "SegvHandler received SIGSEGV! Stack trace:\n");
    for (i = 0; i < size; i++) {
        fprintf(stderr, "%d %s \n",i,strings[i]);
    }

    free (strings);
    exit(1);
}

int main()
{
    signal(SIGSEGV, SegvHandler); // SIGSEGV      11       Core    Invalid memory reference
    signal(SIGABRT, SegvHandler); // SIGABRT       6       Core    Abort signal from

    void* handle = rtspserv(554);

    for(;;)
    {
        sleep(1);
    }
    
    rtspstop(handle);
}
