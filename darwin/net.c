/* Copyright 2023-2025, Mansour Moufid <mansourmoufid@gmail.com> */

/*
 * This file is part of Aluminium Library.
 *
 * Aluminium Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Aluminium Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with Aluminium Library. If not, see <https://www.gnu.org/licenses/>.
 */

#include <arpa/inet.h> // inet_addr, inet_ntoa
#include <ifaddrs.h> // getifaddrs
#include <netinet/in.h> // in_addr_t, struct sockaddr_in
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h> // struct sockaddr
#include <sys/types.h>

#include "al.h"

static inline
bool
private_ip(in_addr_t addr)
{
    // in_addr_t addr = inet_addr(ip);
    int a = (addr >> 0) & 0xff;
    int b = (addr >> 8) & 0xff;
    // RFC 1918
    return (a == 10)
        || (a == 172 && (b >= 16 && b <= 31))
        || (a == 192 && b == 168);
}

char *
al_net_get_local_ip_address(void)
{
    char *ip = NULL;
    struct ifaddrs *addresses = NULL;
    if (getifaddrs(&addresses) != 0)
        return NULL;
    struct ifaddrs *address = addresses;
    while (address != NULL) {
        struct sockaddr *addr = address->ifa_addr;
        if (addr->sa_family == AF_INET) {
            struct sockaddr_in *in = (void *) addr;
            if (private_ip(in->sin_addr.s_addr)) {
                ip = inet_ntoa(in->sin_addr);
                break;
            }
        }
        address = address->ifa_next;
    }
    if (addresses != NULL)
        freeifaddrs(addresses);
    if (ip != NULL)
        return strdup(ip);
    return NULL;
}
