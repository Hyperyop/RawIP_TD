/*
 * rawip_example.c
 *
 *  Created on: May 4, 2016
 *      Author: jiaziyi
 */


#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include "header.h"

#define SRC_IP  "192.168.1.111" //set your source ip here. It can be a fake one
#define SRC_PORT 54321 //set the source port here. It can be a fake one

#define DEST_IP "129.104.89.108" //set your destination ip here
#define DEST_PORT 5555 //set the destination port here
#define TEST_STRING "test data" //a test string as packet payload

int main(int argc, char *argv[])
{
	char source_ip[] = SRC_IP;
	char dest_ip[] = DEST_IP;
	struct sockaddr_in to;
	memset(&to, 0, sizeof(to));
	to.sin_family = AF_INET;
	inet_pton(AF_INET,dest_ip,&to.sin_addr.s_addr);
	to.sin_port  = htons(DEST_PORT);

	int fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    int hincl = 1;                  /* 1 = on, 0 = off */
    setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &hincl, sizeof(hincl));

	if(fd < 0)
	{
		perror("Error creating raw socket ");
		exit(1);
	}

	char packet[65536], *data;
	char data_string[] = TEST_STRING;
	memset(packet, 0, 65536);

	//IP header pointer
	struct iphdr *iph = (struct iphdr *)packet;

	//UDP header pointer
	struct udphdr *udph = (struct udphdr *)(packet + sizeof(struct iphdr));
	struct pseudo_udp_header psh; //pseudo header

	//data section pointer
	data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);

	//fill the data section
	strncpy(data, data_string, strlen(data_string));

	//fill the IP header here
	iph->version=4;
	iph->ihl=5;
	iph->tos = 0;
	iph->tot_len =  htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data));
	iph->id = 123;
	iph->frag_off = 0;
	iph->ttl = 30;
	iph->protocol = 17;
	inet_pton(AF_INET,source_ip ,&iph->saddr);
	inet_pton(AF_INET,dest_ip ,&iph->daddr);
	iph->check = 0;
	iph->check =  htons(checksum((unsigned short *)iph, sizeof(struct iphdr)));

	//fill the UDP header
	udph->source = htons(SRC_PORT);
	udph->dest = htons(DEST_PORT);
	udph->len =  htons(sizeof(struct udphdr)+strlen(data_string));
	udph->check = 0;
	psh.source_address=iph->saddr;
	psh.dest_address=iph->daddr;
	psh.protocol=iph->protocol;
	psh.udp_length=udph->len;
	psh.placeholder=0;
	udph->check = htons(checksum((unsigned short *)&psh,sizeof(struct pseudo_udp_header))+checksum((unsigned short *)udph,sizeof(struct udphdr)+strlen(data)));
	//send the packet
	int bytes_sent = sendto(fd, packet, ntohs(iph->tot_len) , 0, (struct sockaddr*)&to, sizeof(to));
	printf("%d",bytes_sent);
	return 0;

}
