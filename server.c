#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <netinet/tcp.h>

#define BUFLEN 1560
#define FD_START 4
#define INVALID_MESSAGE ("Format invalid pentru mesaj!")

typedef struct message {
    char *topic;
    int type;
    char *payload;
} message;

message *build_msg(char *buf) {
    message *m = (message*)malloc(sizeof(message));
    m->topic = (char*)malloc(50 * sizeof(char));
    //m->payload = (char*)calloc(1500, sizeof(char));

    sprintf(m->topic, "%s", buf);

    m->type = 0;
    memcpy(&m->type, buf + 50, sizeof(char));

    if(m->type == 0) {
        m->payload = (char*)calloc(13, sizeof(char));
        uint8_t sign = 0;
        memcpy(&sign, buf + 51, sizeof(uint8_t));

        uint32_t integer = 0;
        memcpy(&integer, buf + 52, sizeof(uint32_t));
        integer = ntohl(integer);
        if(sign == 1) {
            integer *= -1;
        }

        sprintf(m->payload, "%d", integer);
    } else if(m->type == 1) {
        m->payload = (char*)calloc(13, sizeof(char));
        uint16_t sh = 0;
        memcpy(&sh, buf + 51, sizeof(uint16_t));
        sh = ntohs(sh);

        double sh_real = (double)sh / 100;
        if(sh_real == (int)sh_real) {
            sprintf(m->payload, "%d", (int)sh_real);
        } else {
            sprintf(m->payload, "%.2f", sh_real);
        }
    } else if(m->type == 2) {
        m->payload = (char*)calloc(13, sizeof(char));
        uint8_t sign = 0;
        memcpy(&sign, buf + 51, sizeof(uint8_t));

        uint32_t mod = 0;
        memcpy(&mod, buf + 52, sizeof(uint32_t));
        mod = ntohl(mod);

        uint8_t power = 0;
        memcpy(&power, buf + 56, sizeof(uint8_t));

        double rez = 0;
        rez += mod;
        for(u_int8_t i = 0; i < power; i++) {
            rez /= 10;
        }
        if(sign == 1) {
            rez *= -1;
        }
        sprintf(m->payload, "%.*f", power, rez);
    } else if(m->type == 3) {
        m->payload = (char*)calloc(1500, sizeof(char));
        memcpy(m->payload, buf + 51, 1500);
    }

    return m;
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        fprintf(stderr, "Eg: %s 1948\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    /*
    ██╗░░░██╗██████╗░██████╗░
    ██║░░░██║██╔══██╗██╔══██╗
    ██║░░░██║██║░░██║██████╔╝
    ██║░░░██║██║░░██║██╔═══╝░
    ╚██████╔╝██████╔╝██║░░░░░
    ░╚═════╝░╚═════╝░╚═╝░░░░░
    */
    int sockfd_udp;
    // Creating socket file descriptor
    if ((sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "udp socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    struct sockaddr_in servaddr;
    memset((char*)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    int rc = bind(sockfd_udp, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    if (rc < 0) {
        fprintf(stderr, "udp bind failed\n");
        exit(EXIT_FAILURE);
    }

    /*
    ████████╗░█████╗░██████╗░
    ╚══██╔══╝██╔══██╗██╔══██╗
    ░░░██║░░░██║░░╚═╝██████╔╝
    ░░░██║░░░██║░░██╗██╔═══╝░
    ░░░██║░░░╚█████╔╝██║░░░░░
    ░░░╚═╝░░░░╚════╝░╚═╝░░░░░
    */
    int sockfd_tcp, newsockfd_tcp;
	struct sockaddr_in cli_addr_tcp;
	socklen_t clilen_tcp;
	int *clients;

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

    FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

    sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
    int aux = 1;
    int deactivate = setsockopt(sockfd_tcp, IPPROTO_TCP, TCP_NODELAY, (char *)&aux, sizeof(int));
    if (sockfd_tcp < 0 || deactivate < 0) {
        fprintf(stderr, "tcp socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    int ret = bind(sockfd_tcp, (struct sockaddr *) &servaddr, sizeof(struct sockaddr));
	if (ret < 0) {
        fprintf(stderr, "tcp bind failed\n");
        exit(EXIT_FAILURE);
    }

    ret = listen(sockfd_tcp, 1);
	if (ret < 0) {
        fprintf(stderr, "listen failed\n");
        exit(EXIT_FAILURE);
    }

    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd_tcp, &read_fds);
    FD_SET(sockfd_udp, &read_fds);
	fdmax = sockfd_tcp;


    //LOOP - UL
    char buffer[BUFLEN];
    struct sockaddr_in cliaddr_udp;
    socklen_t len_udp = sizeof(cliaddr_udp);
    memset(&cliaddr_udp, 0, sizeof(cliaddr_udp));

    while(1) {

        //TCP
        tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		if (ret < 0) {
            fprintf(stderr, "select failed\n");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
            // se citeste de la tastatura
            memset(buffer, 0, BUFLEN);
            fgets(buffer, BUFLEN - 1, stdin);

            if (strncmp(buffer, "exit", 4) == 0) {
                break;
            }

        }

        for(int i = 0; i <= fdmax; i++) {
            
            if(FD_ISSET(i, &tmp_fds)) {
                // if (i == STDIN_FILENO) {
                //     // se citeste de la tastatura
                //     memset(buffer, 0, BUFLEN);
                //     fgets(buffer, BUFLEN - 1, stdin);
                //     if (strncmp(buffer, "exit", 4) == 0) {
                //         break;
                //     }
                if(i == sockfd_tcp) {
                    // a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
                    clilen_tcp = sizeof(cli_addr_tcp);
                    newsockfd_tcp = accept(sockfd_tcp, (struct sockaddr *) &cli_addr_tcp, &clilen_tcp);
                    if (newsockfd_tcp < 0) {
                        fprintf(stderr, "accept failed\n");
                        exit(EXIT_FAILURE);
                    }

                    // se adauga noul socket intors de accept() la multimea descriptorilor de citire
                    FD_SET(newsockfd_tcp, &read_fds);
					if (newsockfd_tcp > fdmax) { 
						fdmax = newsockfd_tcp;
					}

                    //TODO
                    //add_client(newsockfd, clients);
                    printf("New client C1 connected from %s:%d.\n",
                            inet_ntoa(cli_addr_tcp.sin_addr), ntohs(cli_addr_tcp.sin_port));
                
                } else if(i == sockfd_udp) {
                    int n = recvfrom(sockfd_udp, buffer, sizeof(buffer) - 1, 0,
                                    (struct sockaddr *)&cliaddr_udp, &len_udp);
                    if (n < 0) {
                        fprintf(stderr, "recvfrom failed\n");
                        exit(EXIT_FAILURE);
                    } else {
                        // Add \0 to the end of the received message
                        buffer[n] = '\0';

                        message *m = build_msg(buffer);
                        puts(m->payload);
                    }
                } else {
                    // s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
                    memset(buffer, 0, BUFLEN);
					ret = recv(i, buffer, sizeof(buffer), 0);
					if (ret < 0) {
                        fprintf(stderr, "recv failed\n");
                        exit(EXIT_FAILURE);
                    } else if(ret == 0) {
                        // conexiunea s-a inchis
						printf("Client C1 disconnected.\n");

                        //TODO
                        //rm_client(i, clients);

                        close(i);

                        // se scoate din multimea de citire socketul inchis 
						FD_CLR(i, &read_fds);
                    } else {
                        printf("S-a primit de la clientul de pe socketul %d mesajul: %s\n", i, buffer);

                        ret = send(i, buffer, sizeof(buffer), 0);

                        if (ret < 0) {
                            fprintf(stderr, "send failed\n");
                            exit(EXIT_FAILURE);
                        }
                    }

                }
            }
        }

        //UDP
        // int n = recvfrom(sockfd_udp, buffer, sizeof(buffer) - 1, 0,
        //                 (struct sockaddr *)&cliaddr_udp, &len_udp);
        // if (n < 0) {
        //     fprintf(stderr, "recvfrom failed\n");
        //     exit(EXIT_FAILURE);
        // } else {
        //     // Add \0 to the end of the received message
        //     buffer[n] = '\0';

        //     message *m = build_msg(buffer);
        //     puts(m->payload);
        // }

    }

    close(sockfd_tcp);
    close(sockfd_udp);
}