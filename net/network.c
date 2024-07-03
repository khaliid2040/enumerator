#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <dirent.h>
#include "../main.h"
#define ADDR_SIZE 48
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
						printf("IPv4 %s\n",ipv4_addr);
						if (ifa->ifa_netmask != NULL) {
							struct sockaddr_in *subnet= (struct sockaddr_in *) ifa->ifa_netmask;
							inet_ntop(AF_INET,&(subnet->sin_addr),subnetmask,sizeof(subnetmask));
							printf("Subnet 	%s\n",subnetmask);
						}
								
					} else if (family== AF_INET6) {
						char ipv6_addr[ADDR_SIZE];
						struct sockaddr_in6 *sock6= (struct sockaddr_in6 *) ifa->ifa_addr;
						inet_ntop(AF_INET6,&sock6->sin6_addr,ipv6_addr,ADDR_SIZE);	
						printf("IPv6 %s\n",ipv6_addr);
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
	char path[SIZE];
	char buffer[SIZE];
	for (int i=0; i<counter; i++) {
		interface(net[i]);
		snprintf(path,SIZE,"/sys/class/net/%s/address",net[i]);
		FILE *address= fopen(path,"r");
		if (address != NULL) {
			while(fgets(buffer,SIZE,address) != NULL) {
				printf("MAC %s",buffer);
			}
		}
		fclose(address);
		free(net[i]);
	}
}