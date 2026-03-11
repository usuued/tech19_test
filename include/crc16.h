#ifndef CRC16_H
#define CRC16_H

#include <cstdint>
#include <cstddef>

namespace crc {

// CRC-16-CCITT implementation
// Polynomial: 0x1021
// Initial value: 0xFFFF
uint16_t crc16_ccitt(const uint8_t* data, size_t length);

} // namespace crc

#endif // CRC16_H
