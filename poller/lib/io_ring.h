#ifndef __LIB_IO_RING__
#define __LIB_IO_RING__

int ir_socket(int af, int sock_type, int protocol);
ssize_t ir_sendto(int sockfd, const void *buf, size_t len, int flags,
		       const struct sockaddr *dest_addr, socklen_t addrlen);

#endif /*__LIB_IO_RING__*/
