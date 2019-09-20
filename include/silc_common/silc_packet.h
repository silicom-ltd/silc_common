
#ifndef _SILC_PKT_PROCESS_H
#define _SILC_PKT_PROCESS_H
#include <string.h>
#include <pcap.h>
#include "dagapi.h"
#include "dagutil.h"


#ifdef __cplusplus
extern "C" {
#endif

/* ERF types locally defined, as these not yet supported in dag */
#ifndef TYPE_IPV4
#define TYPE_IPV4				22
#endif
#ifndef TYPE_IPV6
#define TYPE_IPV6				23
#endif

#define EDA_PKT_OFFSET_NOT_DEFINED	0xffff


#pragma pack(4)
typedef struct eda_erf_flags_s {
	uint8_t			iface:2;
	uint8_t			vlen:1;
	uint8_t			trunc:1;
	uint8_t			rxerror:1;
	uint8_t			dserror:1;
	uint8_t			reserved:1;
	uint8_t			direction:1;
} eda_erf_flags_t;

typedef union eda_dag_ts_u
{
	uint64_t ts64;
	struct {
		uint32_t ts_sub_sec;
		uint32_t ts_sec;
	}ts32;
}eda_dag_ts_t;

typedef struct eda_pkt_dag_hdr_s
{
	eda_dag_ts_t ts;
	uint8_t	type;
	union
	{
		eda_erf_flags_t	flags;
		uint8_t			byte;
	}flags;
	uint16_t	rlen;
	uint16_t	lctr;
	uint16_t	wlen;
}silc_pkt_dag_hdr_t;

#pragma pack()



typedef enum
{
	SILC_LINK_ETH		= 0,
	SILC_LINK_HDLC	= 1,
	SILC_LINK_ATM		= 2,
	SILC_LINK_NONE	= 3, /* No link info, e.g. ERF22 and ERF23*/
}eda_link_t;

typedef enum
{
	EDA_HDLC_CHDLC_ETH	= 0,
	EDA_HDLC_PPP		= 1,
}eda_hdlc_t;

#pragma pack(8)
/*this only for core engine, support ERF only*/
typedef struct silc_pkt_cache_s
{
	/*prepared by eda_pkt_cache_prepre*/
	silc_pkt_dag_hdr_t			erf_header;
	char*						p_pkt_data;

	uint16_t					erf_rlen;		/*host rlen*/
	uint16_t					erf_wlen;		/*host wlen*/

	uint16_t					network_type; /* ethertype or ppp_proto */
	uint16_t					link_type;
	uint16_t					hdlc_type;
	uint8_t						vlan_mpls_count;


/*beaware these are only populated by flow_engine*/
	uint16_t					ip_len;
	uint16_t					ip_h_len;
	uint8_t						ip_proto;

	uint16_t					tcp_h_len;
	uint16_t					tcp_pl_len;
	uint32_t					tcp_seq;

	uint32_t					flow_count;
	uint32_t					netflow_v5_pkt_size;
/*flow engine only fields end*/

	char						pkt_data_pcap[10240];
}silc_pkt_cache;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif
