/* Programming Assignment 2
Submitted by: Arushi Sabharwal
Student ID - W1468298*/

#include<strings.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdint.h>																																						//Library for unsigned int datatype
#include<unistd.h>																																						//Library for close function of socket

#define SPACKETID 0xFFFF
#define EPACKETID 0xFFFF
#define CLIENTID 0xAB
#define ACCPER 0xFFF8
#define NOTPAID 0xFFF9
#define NOTEXIST 0xFFFA
#define ACCOK 0xFFFB
#define CLOSE 0xFFFC 																																					//Creating a separate type for closing packet

struct packet {
	uint16_t s_pack_id;
	uint8_t  client_id;
	uint16_t type;
	uint8_t  seg_no;
	uint8_t  length;
	uint8_t  tech;
	uint32_t  subs_no;
	uint16_t e_pack_id;
};

struct packet init_resp(struct packet p)
{
	struct packet req;
	req.s_pack_id = p.s_pack_id;
	req.client_id = p.client_id;
	req.seg_no = p.seg_no;
	req.length = p.length;
	req.tech = p.tech;
	req.subs_no	= p.subs_no;
	req.e_pack_id	= p.e_pack_id;

	return req;
}

struct database {																																						//Structure defined to store values while reading the database.txt

	uint32_t  subs_no;
	uint8_t   tech;
	uint8_t   paid;
};

void error( char *msg)
{
	perror(msg);
	exit(0);
}

void print_packet_details(struct packet data)																															//Function to print the details of the data packets
{
	printf("%x\t%hhx\t%x\t%d\t%d\t%d\t%u\t%x", data.s_pack_id, data.client_id,
							data.type, data. seg_no, data.length, data.tech, data.subs_no, data.e_pack_id );
}

int main(int argc, char *argv[])
{
	int sock, add_length, fromlen, n;
	struct sockaddr_in server;
	struct sockaddr_in from;
	struct packet data,response;
	int receivedPacketNo = 0;
	struct database db;
	int flag = 0;

	if(argc<2)
	{
		printf("No port number provided");
		exit(0);
	}

	FILE *fp;
	char line[255];

	sock = socket(AF_INET, SOCK_DGRAM, 0); 																																//Here we have mentioned AF_INET to notify Internet domain and SOCK_DGRAM shows that we are creating
																																										//this socket for UDP protocol, 0 shows that use the regular UDP protocol, no change
	if(sock<0) 																																							//If socket is not created properly socket fucntion returns a value <0
	{
		error("Opening socket");
	}

	add_length = sizeof(server);
	bzero(&server, add_length);																																			//Clearing the server structure variable, so that any garbage value is removed

																																										//Here we are setting the address for the server based on the inputs we provided in command line

	server.sin_family = AF_INET; 																																		//Here we are using Internet address in the family
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(argv[1])); 																															//atoi converts port number argument to integer which is further
																																										//converted to network readable format by htons(host to network short)
	if(bind(sock, (struct sockaddr*)&server, add_length)<0)
	{
		error("Binding the socket to address");
	}

	fromlen = sizeof(struct sockaddr_in);
	printf("Welcome to the Server! \n");

	while(1)																																							//Creating an infinite loop for the server so that it can accept multiple user's request and keeps running
	{
		printf("\n\n***************************************************************************************************\n");
		printf("***************************************************************************************************\n\n");

		n=recvfrom(sock,&data,sizeof(struct packet),0,(struct sockaddr*)&from, &fromlen);

			if(n<0)
			{
				error("recvfrom");
			}
			if(data.type == CLOSE) 																																		//condition to check Closing Packet
			{
				close(sock);
				fclose (fp);
				printf("\nClosing socket!\n\nGood Bye!!!\n");
				exit(0);
			}

			printf("New Packet:\n(Expected Packet Number: %d   Received Packet Number: %d)\n\n", receivedPacketNo+1, data.seg_no);
			printf("Received the following from client: \n\n");
			print_packet_details(data);

			response = init_resp(data);
			response.type = NOTEXIST;

			flag=0;
			fp = fopen("database.txt", "r");																															//Opening database.txt in read mode
			if(fp == NULL)
			{
				printf("cannot open file\n");
				exit(0);
			}
		  while (fgets(line, sizeof(line), fp) != NULL) 																												//read a line
		      {
				char *search = " ";
				char *token;

				token = strtok(line, search); 																															//Token will point to Subscriber Number.
				db.subs_no = (unsigned)atol(token);

				token = strtok(NULL, search);																															//Token will point to "Technology".
				db.tech = (uint8_t) atoi(token);

				token = strtok(NULL, search);																															//Token will point to "Paid status".
				db.paid = (uint8_t) atoi(token);

					if(data.subs_no == db.subs_no) 																														//If subscriber number entered by user matches the one in database, we want to break out of the loop
						{
							flag=1;
							break;
						}

		      }
			response.type = NOTEXIST;
			if(flag && data.tech == db.tech)
			{
						response.type = (db.paid) ? ACCOK : NOTPAID;																									//Ternary operator used to check condition, type will be ACCOK if paid=1, else type will be NOTPAID
			}

			if(response.type == NOTPAID)
			{
				printf("\n\nThe Subscriber has not paid\n");
			}
			else if(response.type == NOTEXIST && flag)
			{
				printf("\n\nThe Subscriber exists but technology does not match\n");
			}
			else if(response.type == NOTEXIST)
			{
				printf("\n\nThe Subscriber does not exists\n");
			}
			else if(response.type == ACCOK)
			{
				printf("\n\nAccess Granted\n");
			}

				sendto(sock,&response,sizeof(struct packet),0,(struct sockaddr *)&from,fromlen);
				printf("\n\nServer is sending the below packet to client:\n\n");
				print_packet_details(response);
				receivedPacketNo++;
		}
	}
