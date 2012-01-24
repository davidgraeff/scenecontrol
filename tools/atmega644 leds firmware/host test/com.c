#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>
#include <signal.h>
#include <inttypes.h>

////////// Buffer ////////////
#include <stdio.h>

#define NMAX 100

int iput = 0;  /* Position fuer naechstes abzulegende Zeichen */
int iget = 0;  /* Position fuer naechstes zu lesendes Zeichen */
int buffersize = 0;     /* Anzahl der Zeichen im Buffer */

char buffer[NMAX] = {0};
char returnbuffer[NMAX];
char ignorebuffer[NMAX];

__attribute__((always_inline))
static inline char nextchar(int offset) {
    return buffer[(iget+offset)%NMAX];
}

static inline void drop(int n) {
    if (n>buffersize)
        n=buffersize;
    iget = (iget + n) % NMAX;
    buffersize -= n;
}

void get(const int len)
{
    for (int i=0;i<len;++i) {
        if (buffersize > 0) {
            returnbuffer[i] = buffer[iget];
            iget = (iget + 1) % NMAX;
            buffersize--;
        }
        else {
            printf("Buffer ist leer\n");
        }
    }
}

static inline void put(const char* chars, int len)
{
    for (int i=0;i<len;++i) {
        if (buffersize < NMAX) {
            buffer[iput] = chars[i];
            iput = (iput + 1) % NMAX;
            buffersize++;
        }
        else
            printf("Stack ist voll\n");
    }
}

/////////////////////////////
int fd = 0;
int controller;
int waiting;

char reading = 0;

uint8_t leds = 0;
uint8_t curtainpos = 0;
uint8_t curtainmax = 0;
uint8_t ledvalues[100] = {0};

void parse(void);
int strip(void);
void receiveFromDevice(void);

void catch_int(int v)
{
    /* re-set the signal handler again to catch_int, for next time */
    if (v == SIGIO) {
        signal(SIGIO, catch_int);
        receiveFromDevice();
        if (reading)
            return;
        reading = 1;
        while (buffersize>2) {
            //printf("receive %i\n", buffersize);
            int jumped = strip();
            if (jumped) {
                printf("Verworfen (%i): ", jumped);
                int i;
                for (i=0;i<jumped;++i) {
                    char n = ignorebuffer[i];
                    if ((n>='a' && n<='z') || (n>='A' && n<='Z'))
                        printf("%c", n);
                    else
                        printf("(%i)", n);
                }
                printf("\n");
            }
            printf("Buffer (%i): ", buffersize);
            int i;
            for (i=0;i<buffersize;++i) {
                char n = nextchar(i);
                if ((n>='a' && n<='z') || (n>='A' && n<='Z'))
                    printf("%c", n);
                else
                    printf("(%i)", n);
            }
            printf("\n");
            parse();
        }
        reading = 0;
        return;
    }
    close(fd);
    exit(0);
}

int rs232_open(const char *device, int baudrate) {
    // open
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) return 0;

    struct termios options;

    /*
     * Get the current options for the port...
     */

    tcgetattr(fd, &options);

    /*
     * Set the baud rates to 19200...
     */
    switch (baudrate) {
    case 2400:
        cfsetispeed(&options, B2400);
        cfsetospeed(&options, B2400);
        break;
    case 4800:
        cfsetispeed(&options, B4800);
        cfsetospeed(&options, B4800);
        break;
    case 9600:
        cfsetispeed(&options, B9600);
        cfsetospeed(&options, B9600);
        break;
    case 19200:
        cfsetispeed(&options, B19200);
        cfsetospeed(&options, B19200);
        break;
    case 57600:
        cfsetispeed(&options, B57600);
        cfsetospeed(&options, B57600);
        break;
    case 115200:
        cfsetispeed(&options, B115200);
        cfsetospeed(&options, B115200);
        break;
    default:
        break;
    }

    /*
     * Enable the receiver and set local mode...
     */
    options.c_cflag &= ~PARENB;    // set no parity, stop bits, data bits
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    options.c_cflag |= (CLOCAL | CREAD);

    /*
     * Set the new options for the port...
     */

    tcsetattr(fd, TCSANOW, &options);
    fcntl(fd, F_SETFL, O_NONBLOCK|O_ASYNC);
    //fcntl(fd, F_SETFL, 0);

    return fd;
}

void receiveFromDevice(void) {
    errno = 0;
    char b[100];
    int ret = read(fd, b, 100);
    if (errno==EAGAIN)
        return; // no content
    else if (errno==EIO) {
        return;
    } else if (ret <= 0) {
        perror("");
        return;
    }
    put(b, ret);
}

int sendToDevice(const char *data, int len) {
    int ret = write(fd, data, len);
    if (ret <= 0) {
        perror("Fehler beim senden!\n");
        return 0;
    }
    return ret; /* > 0 = ok */
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int strip(void) {
    int jumped = 0;
    while (buffersize>=2) {
        if (nextchar(0) == 'O' && nextchar(1) == 'K')
            return jumped;
        else {
            ++jumped;
            get(1);
            ignorebuffer[jumped] = returnbuffer[0];
        }
    }
    return jumped;
}

void parse(void) {
    if (buffersize<3) return;
    if (nextchar(0) != 'O' || nextchar(1) != 'K') {
        printf("Sync string not found!\n");
        return;
    }
    char funchar = nextchar(2);
    printf("Funktion: %c\n", funchar);
    switch (funchar) {
    case 'O':
        drop(3);
        printf("Ack;\n");
        break;
    case 'M':
        if (buffersize<2+3) return;
        drop(3);
        printf("Rollo Position: %i/%i\n", nextchar(0), nextchar(1));
        drop(2);
        break;
    case 'I':
        if (buffersize<1+3) return;
        drop(3);
        printf("Init; Protocol Version: %i\n", nextchar(0));
        drop(1);
        break;
    case 'S':
        if (buffersize<1+3) return;
        drop(3);
        printf("Sensoren: %i\n", nextchar(0));
        drop(1);
        break;
    case 'L':
        if (buffersize<1+3) return;
        leds = nextchar(0);
        if (buffersize<leds) {
            return;
        }
        drop(3);
        printf("Leds (%i): ", leds);
        drop(1);
        get(leds);
        int i = 0;
        for (;i<leds;++i) {
            ledvalues[i] = returnbuffer[i];
            printf("%u ", ledvalues[i]);
        }
        printf("\n");
        break;
    default:
        drop(3);
        break;
    }
}

void setled(const uint8_t channel, const uint8_t value) {
    printf("Change channel %i to %i\n", channel, value);
    {
        char t[] = {0xbf, channel, value};
        sendToDevice(t,3);
    }
}

void toAll(const int activate) {
    unsigned char id;
    for (id=0;id<leds;++id) {
        setled(id, (activate?255:0));
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_int);
    signal(SIGIO, catch_int);
    if (argc<=1) {
        printf("Erstes Argument muss das Gerät sein!\n");
        return -1;
    }

    fd = rs232_open(argv[1], 115200);//115200 57600
    if (!fd) {
        perror("Kann nicht öffnen\n");
        return -1;
    }
    uint8_t v1,v2;
    if (argc>=3)
        v1 = (uint8_t)atoi(argv[3]);
    if (argc>=4)
        v2 = (uint8_t)atoi(argv[4]);

    printf("Room Leds Test v0.1 - Device init send\n");
    printf("Kommandos: off [channel] [value], on [channel] [value], curtaindown, curtainup, curtainstop\n");
    {
        char t[] = {0xef};
        sendToDevice(t,1);
    }

    if (argc>=3 && strcmp(argv[2],"off")==0) {
        if (argc == 4) {
            setled(v1, v2);
        } else {
            toAll(0);
        }
    } else if (argc>=3 && strcmp(argv[2],"on")==0) {
        if (argc == 4) {
            setled(v1, v2);
        } else {
            toAll(1);
        }
    } else if (argc>=3 && strcmp(argv[2],"curtaindown")==0) {
        {
            char t[] = {0xdf, 0};
            sendToDevice(t,2);
        }
    } else if (argc>=3 && strcmp(argv[2],"curtainup")==0) {
        {
            char t[] = {0xdf, 1};
            sendToDevice(t,2);
        }
    } else if (argc>=3 && strcmp(argv[2],"curtainstop")==0) {
        {
            char t[] = {0xdf, 255};
            sendToDevice(t,2);
        }
    }

    char input[100] = {0};
    char channel;
    int mode = 0;
    while (1) {
        fgets(input, 100, stdin);
        switch (mode) {
        case 0:
            switch (input[0]) {
            case 'i':
            {
                char t[] = {0xef};
                sendToDevice(t,1);
            }
            break;
            case 'c':
                printf("Enter channel number: ");
                mode = 1;
                continue;
            }
            break;
        case 1:
            channel = input[0]-'0';
            printf("\nChannel %i Value (0-9): ", channel);
            mode = 2;
        case 2:
            setled(channel, (input[0]-'0')*255/10);
        default:
            break;
        }
    }

    close(fd);
    return 0;
}
