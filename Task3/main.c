#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <sys/time.h>
#include <errno.h>

#define PACKET_SIZE 64
#define PACKET_COUNT 4

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void send_ping(const char *ping_ip) {
    struct icmp *icmp_header;
    struct timeval start_time, end_time;
    struct sockaddr_in ping_address;
    socklen_t addr_len;
    char send_packet[PACKET_SIZE], recv_packet[PACKET_SIZE];
    int recv_bytes,sockfd, seq_no = 0;
    int sent = 0, received = 0;

    memset(&ping_address, 0, sizeof(ping_address));
    inet_pton(AF_INET, ping_ip, &ping_address.sin_addr);
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    printf("\nPinging %s:\n", ping_ip);

    for (int i = 0; i < PACKET_COUNT; i++) {

        memset(send_packet, 0, PACKET_SIZE);

        icmp_header = (struct icmp *)send_packet;
        icmp_header->icmp_type = ICMP_ECHO;
        icmp_header->icmp_code = 0;
        icmp_header->icmp_id = getpid();
        icmp_header->icmp_seq = seq_no++;
        icmp_header->icmp_cksum = checksum(icmp_header, sizeof(struct icmp));

        gettimeofday(&start_time, NULL);
        if (sendto(sockfd, send_packet, sizeof(struct icmp), 0, (struct sockaddr*) &ping_address, sizeof(ping_address)) <= 0) {
            perror("sendto");
            continue;
        }

        sent++;

        addr_len = sizeof(ping_address);
        recv_bytes = recvfrom(sockfd, recv_packet, PACKET_SIZE, 0, (struct sockaddr *)&ping_address, &addr_len);

        if (recv_bytes < 0) {
            printf("Request timed out.\n");
            continue;
        }

        gettimeofday(&end_time, NULL);

        struct ip *ip_hdr = (struct ip *)recv_packet;
        icmp_header = (struct icmp *)(recv_packet + (ip_hdr->ip_hl << 2));

        if (icmp_header->icmp_type == ICMP_ECHOREPLY && icmp_header->icmp_id == getpid()) {
            double time_current = ((end_time.tv_sec - start_time.tv_sec) * 1000.0) + ((end_time.tv_usec - start_time.tv_usec) / 1000.0);
            received++;
            printf("Ping to %s: seq=%d, time=%.2f ms\n", ping_ip, icmp_header->icmp_seq, time_current);
        } else {
            printf("Received packet with unknown type.\n");
        }
        sleep(1);
    }
     close(sockfd);
     double percent = (1 - received / sent)*100;
     printf("Packet loss is %.1f%\n", percent );
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("u have not entered ip adresses\n");
        return EXIT_FAILURE;
    }
    for (int i = 1; i < argc; i++) {
        const char *ping_ip = argv[i];
        send_ping(ping_ip);
    }
    return EXIT_SUCCESS;
}