#pragma once

#include <cstdint>

#define PKT_ID 4
#define PKT_LEN 5
#define PKT_INSTRUCT 7
#define PKT_ERR 8
#define PKT_PARAM 8
#define PKT_STAT_PARAM 9

#define HEADER_LEN 7
#define MIN_PACKET_LEN (HEADER_LEN + 3)

#define PARAM(i) (PKT_PARAM + i - 1)
#define STAT_PARAM(i) (PKT_STAT_PARAM + i - 1)

#define BCAST_ID 0xFE

#define INST_PING 0x01
#define INST_READ 0x02
#define INST_WRITE 0x03
#define INST_STATUS 0x55
#define INST_SYN_READ 0x82
#define INST_SYN_WRITE 0x83

enum dyn_status : uint8_t
{
    STATUS_OK = 0,
    RESULT_FAIL = 1,
    INSTRUCT_ERR = 2,
    CRC_MISMATCH = 3,
    DATA_RANGE_ERR = 4,
    DATA_LEN_ERR = 5,
    DATA_LIMIT_ERR = 6,
    ACCESS_ERR = 7
};

/** Read a two-byte parameter starting at the given index */
inline uint16_t get_pkt_short( uint8_t *pkt, int start_idx )
{
    uint16_t val = pkt[start_idx];
    return val | ((uint16_t)pkt[start_idx + 1] << 8);
}
