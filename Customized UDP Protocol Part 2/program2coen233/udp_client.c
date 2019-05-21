/* Programming Assignment 2
Submitted by: Arushi Sabharwal
Student ID - W1468298*/

#include<stdio.h>
#include<strings.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<stdint.h>																											//Library for unsigned int data type
#include<sys/time.h>																										//Library to access timeval
#include<unistd.h>																											//Library for close function of socket

#define SPACKETID 0xFFFF
#define EPACKETID 0xFFFF
#define CLIENTID 0xAB
#define ACCPER 0xFFF8
#define NOTPAID 0xFFF9
#define NOTEXIST 0xFFFA
#define ACCOK 0xFFFB
#define CLOSE 0xFFFC																										//Creating a separate type for closing packet
#define MIN_SUBSNO 1000000000
#define MAX_SUBSNO 4294967295


struct packet																												//Structure to define the packet information
{
	uint16_t s_pack_id;
	uint8_t  client_id;
	uint16_t type;
	uint8_t  seg_no;
	uint8_t  length;
	uint8_t  tech;
	uint32_t  subs_no;
	uint16_t e_pack_id;
};

struct packet init()																										//Function to initialize packet information
{
	struct packet req;
	req.s_pack_id = SPACKETID;
	req.e_pack_id = EPACKETID;
	req.type = ACCPER;
	req.client_id = CLIENTID;
	return req;
}

void error( char *msg)
{
	perror(msg);
	exit(0);
}

void print_packet_details(struct packet data)																				//Function to print the details of the data packets
{
	printf("%x\t%hhx\t%x\t%d\t%d\t%d\t%u\t%x\n", data.s_pack_id, data.client_id, data.type, data. seg_no,
						data.length, data.tech, data.subs_no, data.e_pack_id );
}


int main( int argc, char *argv[])
{
	int sock, length, n, counter;
	struct sockaddr_in server,from;
	struct packet request,response;
	struct hostent *hp;
	int opt_tech=0;
	uint64_t num_check = 0;																									//variable declared to check if subscriber number entered is within the range
	int segmentNo=1;
	char sel;
	int flag;


	if(argc!=3)																												//Condition to check that the user entered 3 arguments at commandline
	{
			printf("You did not enter port correctly\n");
			exit(1);
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock<0)
	{
		error("socket not created properly");
	}

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]); 																							//Converting "localhost" to network readable format

	if(hp==0)
	{
		error("Unknown host");
	}

	bcopy((char*)hp->h_addr, (char*)&server.sin_addr, hp->h_length);
	server.sin_port=htons(atoi(argv[2]));
	length = sizeof(struct sockaddr_in);

	struct timeval tv;
	tv.tv_sec = 3;																											//setting a 3 second timeout for the recvfrom function
	tv.tv_usec = 0;

	if(setsockopt(sock,  SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval))<0)
	{																														//To set options at socket level, we set level = SOL_SOCKET and SO_RCVTIMEO is used to set timeout value for
		error("Error setting timeout");																						//recvfrom() Function and the value of time out is taken from the timeval variable that we pass
	}

  do
	{
		request = init();
		printf("\n\n***************************************************************************************************\n");
		printf("***************************************************************************************************\n\n");
		printf("Please enter the 10 digit Mobile Number you want to check in database: \n");
		printf("(The number should be between the range: %u and %lu)\n",MIN_SUBSNO,MAX_SUBSNO);
		scanf("%lu", &num_check);
			if(num_check>=MIN_SUBSNO && num_check<=MAX_SUBSNO)																//condition to check if the subscriber number entered by user is within the range
				{
					request.subs_no = num_check;

					printf("\nTechnology Options available: \n");
					printf("\tCode\tTechnology\n");
					printf("\t2\t2G\n");
					printf("\t3\t3G\n");
					printf("\t4\t4G\n");
					printf("\t5\t5G\n");
					printf("\nPlease enter the technology:  ");
					scanf("%d",&opt_tech);

						if(opt_tech>=2 && opt_tech<=5)
						{
								request.tech = opt_tech;
								request.length = 0x5;
								request.seg_no = segmentNo;

								printf("\n\nClient is sending the below packet:\n\n");
								print_packet_details(request);																//Function called to display packet details

								length = sizeof(struct sockaddr_in);
								n = sendto(sock, &request, sizeof(struct packet), 0,(struct sockaddr*) &server, length);
								segmentNo++;
								if(n<0)
								{
									error("sendto");
								}
								counter=0;
								for(int retry=0;retry<3;retry++)
								{
									n = recvfrom(sock, &response, sizeof(struct packet), 0, (struct sockaddr*)&from, &length);

									if(n>=0)
									{
										printf("\n\nReceived the following from server:\n\n");								//Packet receive code
										print_packet_details(response);														//Function called to display packet details

										if(response.type == NOTPAID)
										{
											printf("\n\nThe Subscriber has not paid\n");
										}
										if(response.type == NOTEXIST)
										{
											printf("\n\nThe Subscriber does not exist\n");
										}
										if(response.type == ACCOK)
										{
											printf("\n\nAccess Granted\n");
										}

										break;
									}
									else
									{
											printf("\n\nWaiting for server to reply: %d seconds elapsed\n",(retry+1)*3);
											counter++;
										if(retry<2)
										{
											length = sizeof(struct sockaddr_in);
											if(sendto(sock,&request, sizeof(struct packet), 0,(struct sockaddr*) &server, length)<0)
											{
												error("sendto");
											}
										}
									}
							  }
							}
								else
								{
									printf("Error: Invalid Technology \n\n");
								}
				}
					else
					{
							printf("Error: Invalid Subscriber Number \n\n");
					}

					if(counter==3)
					{																										//As the counter reaches 3, we know that this is the maximum count
						printf("\nServer does not respond\n"); 																//and we display the msg "server does not respond"
						exit(0);
					}

				flag =0;
			do {
					if(flag)
							{
								printf("Error: Not a valid option. Please Try again!\n");
							}
						printf("\nDo you want to send again ? Please enter : Y/N  \n");
						scanf("%s",&sel);
						flag  = 1;
				}	while( sel != 'y' && sel!='Y' &&sel != 'n' && sel!='N'); 												//Condition to ask the user to input again if invalid

	}	while(sel =='y'||sel=='Y');

		request = init();																									//initializing closing packet
		request.subs_no = 0;
		request.tech = 0;
		request.length = 0;
		request.seg_no = segmentNo;
		request.type = CLOSE;
		length = sizeof(struct sockaddr_in);
		if(sendto(sock,&request, sizeof(struct packet), 0,(struct sockaddr*) &server, length)<0)
		{
			error("sendto");
		}

		printf("\nClosing Socket\n\n");
		close(sock);																										//Closing socket at client side

		printf("Good Bye!!!!\n");
}
