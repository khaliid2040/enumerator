#include <ifaddrs.h>
#include <net/if.h>
#include <dirent.h>
#include "../main.h"
#define ADDR_SIZE 48
#include <stdio.h>
#include <stdlib.h>

void printNetworkConnections() {
    const char* protocols[] = {"tcp", "udp"};
    const char* commands[] = {"ss -atn", "ss -uan"};
    char line[SIZE];
    char localAddr[SIZE], remoteAddr[SIZE], state[SIZE];

    // Iterate over each protocol and corresponding command
    for (int i = 0; i < 2; i++) {
        FILE *fp = popen(commands[i], "r");
        if (fp == NULL) {
            perror("popen");
            continue;
        }

        // Skip header line if needed
        fgets(line, sizeof(line), fp);

        // Print column headers
        if (i == 0) {
            printf("%-8s %-35s %-35s %-35s\n", "Protocol", "Local Address", "Remote Address", "State");
        } else {
            printf("%-8s %-35s %-35s\n", "Protocol", "Local Address", "Remote Address");
        }

        while (fgets(line, sizeof(line), fp) != NULL) {
            int numFields = sscanf(line, "%s %*d %*d %s %s ", state, localAddr, remoteAddr);

            // Check if parsing was successful
            if (numFields < 2) {
                fprintf(stderr, "Failed to parse line: %s", line);
                continue;
            }

            // Map state to human-readable format for TCP
            if (i == 0) { // TCP
                if (strcmp(state, "ESTAB") == 0) {
                    strcpy(state, "established");
                } else if (strcmp(state, "LISTEN") == 0) {
                    strcpy(state, "listening");
                } else if (strcmp(state, "TIME-WAIT") == 0) {
                    strcpy(state, "waiting");
                } else {
                    strcpy(state, "unknown"); // Handle unexpected states
                }
                printf("%-8s %-35s %-35s %-35s\n", protocols[i], localAddr, remoteAddr, state);
            } else { // UDP
                printf("%-8s %-35s %-35s\n", protocols[i], localAddr, remoteAddr);
            }
        }

        if (pclose(fp) == -1) {
            perror("pclose");
        }

        // Print a separator between protocol outputs
        if (i == 0) {
            printf("------------------------------------------------------------------------------------\n");
        }
    }
}

void interface(const char *intface) {
	struct ifaddrs *ifap, *ifa;
	int family,printed=0;
	char subnetmask[ADDR_SIZE];
	if (getifaddrs(&ifap) != -1) {
		for (ifa= ifap; ifa != NULL; ifa= ifa->ifa_next) {
			if (ifa->ifa_addr == NULL) {
				continue;
			}
			if (strcmp(ifa->ifa_name,intface)==0) {
				if (!printed) {	
					printf(ANSI_COLOR_BLUE "%s\n" ANSI_COLOR_RESET,ifa->ifa_name);
				}		
				printed=1;
				family= ifa->ifa_addr->sa_family;
				
				if (family == AF_INET) {
					char ipv4_addr[ADDR_SIZE];
					struct sockaddr_in *sock= (struct sockaddr_in *) ifa->ifa_addr;
					inet_ntop(AF_INET,&	sock->sin_addr,ipv4_addr,ADDR_SIZE);
					printf(DEFAULT_COLOR"IPv4:\t\t"ANSI_COLOR_RESET "%s\n",ipv4_addr);
					if (ifa->ifa_netmask != NULL) {
						struct sockaddr_in *subnet= (struct sockaddr_in *) ifa->ifa_netmask;
						inet_ntop(AF_INET,&(subnet->sin_addr),subnetmask,sizeof(subnetmask));
						printf(DEFAULT_COLOR "Subnet:\t\t"ANSI_COLOR_RESET "%s\n",subnetmask);
					}
							
				} else if (family== AF_INET6) {
					char ipv6_addr[ADDR_SIZE];
					struct sockaddr_in6 *sock6= (struct sockaddr_in6 *) ifa->ifa_addr;
					inet_ntop(AF_INET6,&sock6->sin6_addr,ipv6_addr,ADDR_SIZE);	
					printf(DEFAULT_COLOR "IPv6:\t\t	"ANSI_COLOR_RESET "%s\n",ipv6_addr);
				}
			}
				
		}
		freeifaddrs(ifap);	
	}
}
void network(void) {
	DIR *block= opendir("/sys/class/net");
	struct dirent *entry;
	char *net[SIZE];
	int counter=0;
	if (block != NULL) {
		while ((entry= readdir(block)) != NULL) {
			if (strcmp(entry->d_name,".")  == 0 || strcmp(entry->d_name,"..") == 0) {
				continue;
			}
			net[counter]= malloc(strlen(entry->d_name)+1);
			strcpy(net[counter],entry->d_name);		
			counter++;
		}
	}
	closedir(block);
	char path[SIZE];
	char buffer[SIZE];
	for (int i=0; i<counter; i++) {
		interface(net[i]);
		snprintf(path,SIZE,"/sys/class/net/%s/address",net[i]);
		FILE *address= fopen(path,"r");
		if (address != NULL) {
			while(fgets(buffer,SIZE,address) != NULL) {
				printf(DEFAULT_COLOR "MAC:\t\t"ANSI_COLOR_RESET "%s",buffer);
			}
		}
		//memset(buffer,0,sizeof(buffer));
		snprintf(path,sizeof(path),"/sys/class/net/%s/operstate",net[i]);
		FILE *state= fopen(path,"r");
		if (state==NULL) {
			perror("fopen");
			continue;
		}
		printf(DEFAULT_COLOR"State:\t\t"ANSI_COLOR_RESET);
		if  (fgets(buffer,sizeof(buffer),state) != NULL) {
			//trim new line character
			int len= strlen(buffer);
			buffer[len - 1]='\0';
			//making colored output
			if (!strcmp(buffer,"up")) {
				printf(ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET,buffer);
			} else if (!strcmp(buffer,"down")) {
				printf(ANSI_COLOR_RED "%s\n"ANSI_COLOR_RESET,buffer);
			} else {
				printf(ANSI_COLOR_RED "%s\n"ANSI_COLOR_RESET,buffer);
			}
		}
		fclose(address);
		fclose(state);
		free(net[i]);
	}
	printf(ANSI_COLOR_YELLOW "getting connection information...\n" ANSI_COLOR_RESET);
	printNetworkConnections();
}