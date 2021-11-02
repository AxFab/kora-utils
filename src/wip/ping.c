/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2021  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 */
#include "utils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <sys/times.h>

opt_t options[] = {
    /*
    -t             Effectue un test ping sur l'h▒te sp▒cifi▒ jusqu'▒ son arr▒t.
                   Pour arr▒ter, appuyez sur Ctrl+C.
    -n count       Nombre de demandes d'▒cho ▒ envoyer.
    -l size        Taille du tampon d'envoi.
    -w timeout     D▒lai d'attente pour chaque r▒ponse, en millisecondes.
    */
    END_OPTION("ICMP Ping.")
};

char *usages[] = {
    "[OPTION] HOST",
    NULL,
};

struct ICMP_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence_number;
    uint8_t payload[0];
};

struct IP4_header {
    uint8_t header_length : 4;
    uint8_t version : 4;
    uint8_t service_type;
    uint16_t length;
    uint16_t identifier;
    uint16_t offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint8_t source[4];
    uint8_t target[4];
};

uint16_t checksum(void *buf, size_t len)
{
    uint32_t sum = 0;
    uint16_t *ptr = buf;
    len /= 2;
    for (unsigned i = 0; i < len; ++i)
        sum += ntohs(ptr[i]);
    if (sum > 0xFFFF)
        sum = (sum >> 16) + (sum & 0xFFFF);
    return htons(~(sum & 0xFFFF) & 0xFFFF);
}

// // Checksum: 0x5c0b incorrect, should be 0xf7fe
// // Checksum: 0xb02e incorrect, should be 0xf7fe

// void dump(void *buf, size_t len)
// {
//     uint8_t *ptr = buf;
//     unsigned i = 0;
//     for (; len - i >= 16 ; i += 16) {
//         printf("%04x   %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", i
//             , ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]
//             , ptr[i+4], ptr[i+5], ptr[i+6], ptr[i+7]
//             , ptr[i+8], ptr[i+9], ptr[i+10], ptr[i+11]
//             , ptr[i+12], ptr[i+13], ptr[i+14], ptr[i+15] );
//     }
//     if (i < len)
//         printf("%04x  ", i);
//     for (; i < len; ++i) {
//         printf(" %02x", ptr[i]);
//     }
//     printf("\n");
// }

struct {
    int ping_break;
    int payload;
    int max_packet;
    int timeout;
} _;

static int ping_break = 0;

static void sigint_handler(int sig)
{
    (void)sig;
    ping_break = 1;
    printf("\n");
}

void ping_parse_arg(void *a, int opt, char *arg)
{
    switch (opt) {
    case 't':
        _.max_packet = 1;
        _.timeout = -1;
        break;
    case 'n':
        _.max_packet = strtol(arg, NULL, 0);
        break;
    case 'l':
        _.payload = strtol(arg, NULL, 0);
        break;
    case 'w':
        _.timeout = strtol(arg, NULL, 0);
        break;
    }
}

int main(int argc, char **argv)
{
    long prec = 100000 / sysconf(_SC_CLK_TCK);
    char data[512];
    char buf[512];
    size_t len = 8;

    struct hostent * host = gethostbyname(argv[1]);
    if (!host) {
        fprintf(stderr, "%s: Name or service not known\n", argv[1]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        fprintf(stderr, "%s: No socket available: %s\n", argv[1], strerror(errno));
        return 1;
    }

    signal(SIGINT, sigint_handler);

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;

    memcpy(&dest.sin_addr.s_addr, host->h_addr, host->h_length);

    char addr[16];
    uint8_t *haddr = (uint8_t *)host->h_addr_list[0];
    snprintf(addr, 16, "%d.%d.%d.%d", haddr[0], haddr[1], haddr[2], haddr[3]);
    // inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);
    printf("PING %s (%s) %d bytes of data.\n", argv[1], addr, len);

    struct ICMP_header* head = (struct ICMP_header*)buf;
    head->type = 8; // PING
    head->code = 0;
    head->identifier = rand() % 0xffff;

    int psent = 0;
    int presv = 0;
    clock_t start_tick = times(NULL);
    while (ping_break == 0) {
        // Prepare payload
        for (unsigned i = sizeof(struct ICMP_header); i < len; ++i)
            head->payload[i] = i;
        head->sequence_number = htons(psent + 1);
        head->checksum = 0;
        head->checksum = checksum(buf, len);

        // Send
        clock_t send_tick = times(NULL);
        int ret = sendto(sock, buf, len, 0, (struct sockaddr*)&dest, sizeof(struct sockaddr_in));
        if (ret != (int)len) {
            fprintf(stderr, "sendto: %s\n", strerror(errno));
            break;
        }
        ++psent;

        ret = recv(sock, data, 4096, 0);
        clock_t recv_tick = times(NULL);
        long elapsed = (recv_tick - send_tick) * prec;
        if (ret > 0) {
            presv++;

            struct IP4_header *h1 = (void *)data;
            struct ICMP_header *h2 = (void *)&data[sizeof(struct IP4_header)];

            printf("%zd bytes from %s: icmp_seq=%d ttl=%d time=%d.%02dms\n",
                ret, addr, htons(h2->sequence_number), h1->ttl, elapsed / 100, elapsed % 100);
        }

        struct timespec ts;
        ts.tv_sec = 1;
        ts.tv_nsec = 0;
        nanosleep(&ts, NULL);
    }

    clock_t end_tick = times(NULL);
    long long smtime = (end_tick - start_tick) * prec;

    printf("--- %s ping statistics ---\n", argv[1]);
    printf("%d packets transmitted, %d received, %d%% packet loss, time %lld.%02dms\n",
        psent, presv, (psent - presv) * 100 / psent, smtime / 100LL, (int)smtime % 100);
    // printf("rtt min/avg/max/mdev = 3.439/4.033/4.628/0.594 ms");

    close(sock);
    return 0;
}

