#ifndef _CONFIGS_H
#define _CONFIGS_H

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <poll.h>


#define DBG_PRINTF printf
#define FB_DEVICE_NAME "/dev/fb0"

#endif /*_CONFIG_H*/ 
