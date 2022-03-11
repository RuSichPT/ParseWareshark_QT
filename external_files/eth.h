
//eth.h

//Define to prevent recursive inclusion
#ifndef __ETH_H
#define __ETH_H

#include "types.h"

#define ETH_MTU          	1500
#define SIZEOF_ETH_HDR 		(sizeof(eth_hdr))  // + ETH_PAD_SIZE)

#define MAX_PKT_SIZE    (ETH_MTU + SIZEOF_ETH_HDR)

/* These macros should be calculated by the preprocessor and are used
   with compile-time constants only (so that there is no little-endian
   overhead at runtime). */
#define PP_HTONS(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) ((((x) & 0xff) << 24) | \
                     (((x) & 0xff00) << 8) | \
                     (((x) & 0xff0000UL) >> 8) | \
                     (((x) & 0xff000000UL) >> 24))
#define PP_NTOHL(x) PP_HTONL(x)

#define ETHTYPE_ARP       0x0806U
#define ETHTYPE_IP        0x0800U
#define ETHTYPE_VLAN      0x8100U
#define ETHTYPE_PPPOEDISC 0x8863U  /* PPP Over Ethernet Discovery Stage */
#define ETHTYPE_PPPOE     0x8864U  /* PPP Over Ethernet Session Stage */

#define ARP_HW_TYPE_ETH		PP_HTONS(0x0001)
#define ARP_PROTO_TYPE_IP	PP_HTONS(ETHTYPE_IP)

#define ARP_TYPE_REQUEST	PP_HTONS(1)
#define ARP_TYPE_RESPONSE	PP_HTONS(2)

#define BRDCST_HWADDR 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF

#define ETHARP_HWADDR_LEN     6



#define IP_PROTO_ICMP    1
#define IP_PROTO_IGMP    2
#define IP_PROTO_UDP     17
#define IP_PROTO_UDPLITE 136
#define IP_PROTO_TCP     6

#pragma pack(push, 1)
typedef struct {
  uint8_t addr[ETHARP_HWADDR_LEN];
} eth_addr;

/** Ethernet header */
typedef struct {
eth_addr        dest;
eth_addr        src;
uint16_t        type;
uint8_t         data[];
} eth_hdr;


typedef struct {
  /* version / header length */
  uint8_t _v_hl;
  /* type of service */
  uint8_t _tos;
  /* total length */
  uint16_t _len;
  /* identification */
  uint16_t _id;
  /* fragment offset field */
  uint16_t _offset;
#define IP_RF 0x8000U        /* reserved fragment flag */
#define IP_DF 0x4000U        /* dont fragment flag */
#define IP_MF 0x2000U        /* more fragments flag */
#define IP_OFFMASK 0x1fffU   /* mask for fragmenting bits */
  /* time to live */
  uint8_t _ttl;
  /* protocol*/
  uint8_t _proto;
  /* checksum */
  uint16_t _chksum;
  /* source and destination IP addresses */
  uint32_t src_addr;
  uint32_t dest_addr;
  uint8_t  data[];
} ip_hdr;

typedef struct {
  uint16_t src_port;
  uint16_t dest_port;
  uint16_t len;
  uint16_t chksum;
  uint8_t  data[];
} udp_t;

typedef struct
{
	uint16_t hw_type;
	uint16_t proto_type;
	uint8_t hw_addr_len;
	uint8_t proto_addr_len;
	uint16_t type;
	uint8_t mac_addr_from[ETHARP_HWADDR_LEN];
	uint32_t ip_addr_from;
	uint8_t mac_addr_to[ETHARP_HWADDR_LEN];
	uint32_t ip_addr_to;
} arp_message_t;

typedef struct {
  uint8_t type;
  uint8_t code;
  uint16_t chksum;
  uint16_t id;
  uint16_t seqno;
} icmp_echo_hdr;

#define ICMP_ER   0    /* echo reply */
#define ICMP_DUR  3    /* destination unreachable */
#define ICMP_SQ   4    /* source quench */
#define ICMP_RD   5    /* redirect */
#define ICMP_ECHO 8    /* echo */
#define ICMP_TE  11    /* time exceeded */
#define ICMP_PP  12    /* parameter problem */
#define ICMP_TS  13    /* timestamp */
#define ICMP_TSR 14    /* timestamp reply */
#define ICMP_IRQ 15    /* information request */
#define ICMP_IR  16    /* information reply */

#define ICMP_TTL       255

#define ICMP_DATA_STANDART_LEN  32
#define ICMP_STANDART_TOTAL_LEN (sizeof(ip_hdr) + sizeof(icmp_echo_hdr) + ICMP_DATA_STANDART_LEN)

typedef struct {
ip_hdr iphdr; 
  uint16_t src_port;
  uint16_t dest_port;
} tcp_udp_ports;
#pragma pack(pop)

#define IP_MAX_FRAGMENTS 4 // максимальное количество фрагментированных IP пакетов (IP_MF more fragments flag) для которых будет храниться _id
typedef struct {
  ip_hdr* iphdr;
  uint16_t _id;
} ip_fragments_table_typedef;

#define IP_BRDCST               (0xFFFFFFFFu)
#define BRDCST                  IP_BRDCST

#endif // #ifndef __ETH_H
