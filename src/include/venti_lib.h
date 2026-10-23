#ifndef VENTI_LIB_H
#define VENTI_LIB_H

void *memset(void *dest, int value, unsigned int count);

inline uint32_t sum8_u32(uint32_t *data) {
    uint32_t sum = 0;
    for (size_t i = 0; i < 8; i++) {
        sum += data[i];
    }
    return sum;
};

// uint32_t sum_u32(uint32_t *data, size_t length) {
//     uint32_t sum = 0;
//     for (size_t i = 0; i < length; i++) {
//         sum += data[i];
//     }
//     return sum;
// };

#endif