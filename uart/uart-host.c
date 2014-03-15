#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <uart.h>

int uart_fd;

struct termios old_options;

unsigned int baud_to_value(speed_t baud)
{
        switch (baud) {
        case B0:      return 0;
        case B50:     return 50;
        case B75: return 75;
        case B110: return 110;
        case B134: return 134;
        case B150: return 150;
        case B200: return 200;
        case B300: return 300;
        case B600: return 600;
        case B1200: return 1200;
        case B1800: return 1800;
        case B2400: return 2400;
        case B4800: return 4800;
        case B9600: return 9600;
        case B19200: return 19200;
        case B38400: return 38400;
        case B57600: return 57600;
        case B115200: return 115200;
        case B230400: return 230400;
        case B460800: return 460800;
        case B500000: return 500000;
        case B576000: return 576000;
        case B921600: return 921600;
        case B1000000: return 1000000;
        case B1152000: return 1152000;
        case B1500000: return 1500000;
        case B2000000: return 2000000;
        case B2500000: return 2500000;
        case B3000000: return 3000000;
        case B3500000: return 3500000;
        case B4000000: return 4000000;
        default: break;
        }

        return 0;
}


void uart_init(char *sport)
{
        int rc;
        struct termios options;

        /**
         * O_NOCTTY -- ttyS is not our controlling terminal:
         *   kernel, do not do fancy stuff! We're not getty, stupid!
         *
         * O_NDELAY -- don't block for DTR, we're not talking to a modem
         */
        uart_fd = open(sport, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
        if (uart_fd < 0) {
                debug_perror(0, "Error opening serial port %s", sport);
                exit(EXIT_FAILURE);
        }

        // get serial options
        tcgetattr(uart_fd, &old_options);
        // clear struct
        bzero(&options, sizeof(options));

        //both needed because cfsetspeed is not available on Windows.
        cfsetispeed(&options, UART_BAUD_RATE);
        cfsetospeed(&options, UART_BAUD_RATE);


        options.c_cflag |= (CS8 | CLOCAL | CREAD);
        options.c_iflag |= IGNBRK;
        options.c_lflag = 0;
        options.c_cc[VTIME]    = 0;   /* inter-character timer unused */
        options.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */
        cfmakeraw(&options);
        tcflush(uart_fd, TCIFLUSH);
        rc = tcsetattr(uart_fd, TCSANOW, &options);

        if (rc == -1) {
                debug_perror(0, "Error setting serial options");
                close(uart_fd);
                exit(EXIT_FAILURE);
        } else
                debug(1,
                      "Serial CAN communication established at %lu Baud",
                      baud_to_value(cfgetospeed(&options)));
}

void uart_close()
{
        close(uart_fd);
        tcsetattr(uart_fd, TCSANOW, &old_options);
}

void uart_putc(char c)
{
        ssize_t ret = write(uart_fd, &c, 1);
        if (ret != 1) {
                debug(0, "uart_putc faild: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
}

void uart_putstr(char *str)
{
        while (*str) {
                uart_putc(*str++);
        }
}

unsigned char uart_getc_nb(char *c)
{
        int ret;

        ret = read(uart_fd, c, 1);

        if (ret <= 0) {
                debug(10, "uart char: %d\n", c);
                return 0;
        }
        return 1;
}

char uart_getc(void)
{
        char c;
        int ret;
        fd_set rset;

        FD_ZERO(&rset);
        FD_SET(uart_fd, &rset);

        ret = select(uart_fd + 1, &rset, (fd_set *) NULL, (fd_set *) NULL, NULL);
        debug_assert(ret >= 0, "uart-host.c: select failed");

        uart_getc_nb(&c);

        return c;
}
