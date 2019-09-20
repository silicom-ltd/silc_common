/*
 * silc_udp.h
 *
 *  Created on: Jul 14, 2012
 *      Author: jeff_zheng
 */

#ifndef SILC_NET_H_
#define SILC_NET_H_

#include "silc_common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

static inline void silc_sock_addr_dump(struct sockaddr* p_addr)
{
	char addrbuf[256];
	struct sockaddr_in* p_addr4 = (struct sockaddr_in*) p_addr;
	struct sockaddr_in6* p_addr6 = (struct sockaddr_in6*) p_addr;
	switch(p_addr->sa_family)
	{
	case AF_UNSPEC:
		SILC_LOG("AF family not specified");
		break;
	case AF_INET:
		inet_ntop(p_addr->sa_family, &p_addr4->sin_addr, addrbuf, sizeof(addrbuf));
		SILC_LOG("Inet 4 address %s:%u", addrbuf, ntohs(p_addr4->sin_port));
		break;
	case AF_INET6:
		inet_ntop(p_addr->sa_family, &p_addr6->sin6_addr, addrbuf, sizeof(addrbuf));
		SILC_LOG("Inet 6 address %s:%u", addrbuf, ntohs(p_addr6->sin6_port));
		break;
	default:
		SILC_LOG("Unsupported af family");
		break;
	}

}

/**
 *
 * @param af			af family, can be 0, then the function will do a dns query on the serv_addr
 * @param serv_addr		address of the server, can be ipv4 or ipv6 address, but must match af
 * @param port			port number
 * @param p_saddr			a pointer to a piece of memory that are used to store ipv4 addr
 * @param p_saddr6_cache	a pointer to a piece of memory that are used to store ipv6 addr
 * @param p_saddr_len		return the actual len of the saddr
 * @return
 */

static inline int silc_sock_addr_prepare(int af, char* serv_addr, uint16_t port,
		struct sockaddr* p_saddr, socklen_t saddr_len)
{
	void* p_ipaddr;
	struct sockaddr_in*	p_saddr4 = (struct sockaddr_in*)p_saddr;
	struct sockaddr_in6* p_saddr6 = (struct sockaddr_in6*)p_saddr;
	socklen_t local_slen;
	int ret;
	if(af == AF_INET)
	{
		p_ipaddr = &p_saddr4->sin_addr;
		local_slen = sizeof(struct sockaddr_in);
		if(local_slen > saddr_len)
		{
			SILC_ERR("saddr len %u not enough to hold ipv4 saddr, size %u", saddr_len, local_slen);
			return -1;
		}
		p_saddr4->sin_addr.s_addr = INADDR_ANY;
		p_saddr4->sin_port = htons(port);
	}
	else if (af == AF_INET6)
	{
		p_ipaddr = &p_saddr6->sin6_addr;
		local_slen = sizeof(struct sockaddr_in6);
		if(local_slen > saddr_len)
		{
			SILC_ERR("saddr len %u not enough to hold ipv6 saddr, size %u", saddr_len, local_slen);
			return -1;
		}
		p_saddr6->sin6_addr = in6addr_any;
		p_saddr6->sin6_port = htons(port);
	}
	else if (af != AF_UNSPEC)
	{
		SILC_ERR("Invalid af %d", af);
		return -1;
	}
	p_saddr->sa_family = af;


	if(serv_addr)
	{
		struct addrinfo hints, *result_list, *rp;
		char portstr[32];
		if (af != AF_UNSPEC)
		{
			ret = inet_pton(af, serv_addr, p_ipaddr);
			if(ret > 0)
			{//valid address for af (INET4 or INET6) found
				return local_slen;
			}
		}

		//either af is 0, or serv_addr is a name not an address
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = af;
		sprintf(portstr, "%u", port);
		ret = getaddrinfo(serv_addr, portstr, &hints, &result_list);
		if(ret!=0)
		{
			SILC_ERR("Invalid bind address, af %d, %s", af, serv_addr);
			return -1;
		}

		//result_list is a list as suggested by its name. Mutliple addresses may be returned by getaddrinfo.
		//Please refer to the example in man getaddrinfo for how to traverse this list
		//in here, we currently look only on the first entry
		if(result_list)
		{
			p_saddr->sa_family = result_list->ai_family;
			rp = result_list;
			if(rp->ai_addrlen > saddr_len)
			{
				SILC_ERR("saddr len %u not enough to hold ipv6 saddr, size %u", saddr_len, local_slen);
				return -1;
			}
			memcpy(p_saddr, rp->ai_addr, rp->ai_addrlen);
			freeaddrinfo(result_list);
			return rp->ai_addrlen;
		}
		else
			return -1;

	}
	return local_slen;
}

static inline int silc_sock_server_open(int af, int type, char* serv_addr, uint16_t port, uint32_t sock_rx_buf_size, uint32_t sock_tx_buf_size)
{
	int sock;
	int setoptarg, getoptarg = 0;
	int ret = 0;
	struct sockaddr_storage saddr_cache;
	struct sockaddr* p_saddr = (struct sockaddr*)&saddr_cache;
	socklen_t	saddr_len = sizeof(saddr_cache);
	socklen_t optlen;

	saddr_len = silc_sock_addr_prepare(af, serv_addr, port, p_saddr, saddr_len);
	if(saddr_len <0)
		return -1;

	sock = socket(AF_INET, type, 0);
	if(sock < 0)
	{
		SILC_ERR("Failed to open socket %s", strerror(errno));
		return -1;
	}

	setoptarg = 1;
	ret = setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&setoptarg,sizeof(setoptarg));
	if(ret!=0)
	{
		SILC_LOG("setsockopt return %d %s",ret,strerror(errno));
	}

	ret = bind(sock, p_saddr, saddr_len);
	{
		if(ret<0)
		{
			SILC_ERR("Failed to bind address, error %d:%s", errno, strerror(errno));
			silc_sock_addr_dump(p_saddr);
			return ret;
		}
	}

	if(sock_rx_buf_size)
	{
		setoptarg = sock_rx_buf_size;
		ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &setoptarg, sizeof(setoptarg));
		if(ret < 0)
		{
			SILC_ERR("Failed to set receive buffer size, %s", strerror(errno));
		}
		optlen = sizeof(getoptarg);
		if(0!=getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &getoptarg, &optlen))
		{
			SILC_ERR("Failed to read back receive buffer size");
			return -1;
		}
		else
		{
			SILC_LOG("socket receive buffer size set to %d", getoptarg);
		}
	}

	if(sock_tx_buf_size)
	{
		setoptarg = sock_tx_buf_size;
		ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &setoptarg, sizeof(setoptarg));
		if(ret < 0)
		{
			SILC_ERR("Failed to set send buffer size, %s", strerror(errno));
		}
		optlen = sizeof(getoptarg);
		if(0!=getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &getoptarg, &optlen))
		{
			SILC_ERR("Failed to read back send buffer size");
			return -1;
		}
		else
		{
			SILC_LOG("socket send buffer size set to %d", getoptarg);
		}
	}
	return sock;
}

static inline int silc_sock_client_open(int af, int type, char* serv_addr, uint16_t port, uint32_t sock_buf_size, struct sockaddr* p_ret_saddr, socklen_t* p_saddr_len)
{
	int fd;
	int setoptarg, getoptarg = 0;
	int ret = 0;
	socklen_t saddr_len;
	socklen_t optlen;

	if(serv_addr == NULL)
		return -1;

	saddr_len = silc_sock_addr_prepare(af, serv_addr, port, p_ret_saddr, *p_saddr_len);
	if(saddr_len <0)
		return -1;

	fd = socket(AF_INET, type, 0);
	if(fd < 0)
	{
		SILC_ERR("Failed to open socket %s", strerror(errno));
		return -1;
	}

	if(sock_buf_size)
	{
		setoptarg = sock_buf_size;
		ret = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &setoptarg, sizeof(setoptarg));
		if(ret < 0)
		{
			SILC_ERR("Failed to set send buffer size, %s", strerror(errno));
		}

		optlen = sizeof(getoptarg);
		if(0!=getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &getoptarg, &optlen))
		{
			SILC_ERR("Failed to read back receive buffer size");
			return -1;
		}
		else
		{
			SILC_LOG("socket send buffer size set to %d", getoptarg);
		}
	}

	*p_saddr_len = saddr_len;

	return fd;
}

static inline int silc_sock_client_connect(int sock, struct sockaddr* p_remote_saddr, socklen_t p_saddr_len)
{
	if(connect(sock, p_remote_saddr, p_saddr_len) != 0)
	{
		SILC_ERR("Failed to connect remote, %s", strerror(errno));
		return -1;
	}
	return 0;
}

static inline int silc_sock_server_listen(int sock, int backlog)
{
	if(listen(sock, backlog) != 0)
	{
		SILC_ERR("Failed to listen, %s", strerror(errno));
		return -1;
	}
	return 0;
}

static inline int silc_sock_server_accept(int sock, struct sockaddr* p_remote_saddr, socklen_t* p_saddr_len)
{
	int ret = accept(sock, p_remote_saddr, p_saddr_len);
	if(ret < 0)
	{
		SILC_ERR("Failed to accept, %s", strerror(errno));
	}
	return ret;
}

#endif /* SILC_NET_H_ */
