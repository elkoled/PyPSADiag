#include <stdio.h>
#include <inttypes.h>

// --- PSA ALGORITHM ---

// Transformation function with PSA not-so-secret sauce
int16_t transform(uint8_t data_msb, uint8_t data_lsb, uint8_t sec[])
{
    int16_t data = (data_msb << 8) | data_lsb;
    int32_t result = ((data % sec[0]) * sec[2]) - ((data / sec[0]) * sec[1]);
    
    if (result < 0)
        result += (sec[0] * sec[2]) + sec[1];
    
    return result;
}

// Challenge reponse calculation for a given pin and challenge
uint32_t compute_response(uint8_t pin[], uint8_t chg[])
{
    // Using uint8_t to ensure 0xB2 is treated as 178 (not -78)
    uint8_t sec_1[3] = {0xB2, 0x3F, 0xAA};
    uint8_t sec_2[3] = {0xB1, 0x02, 0xAB};

    // Compute each 16b part
    int16_t res_msb = transform(pin[0], pin[1], sec_1) | transform(chg[0], chg[3], sec_2);
    int16_t res_lsb = transform(chg[1], chg[2], sec_1) | transform(res_msb >> 8, res_msb & 0xFF, sec_2);
    
    // Combine to 32-bit unsigned
    return ((uint32_t)(uint16_t)res_msb << 16) | (uint16_t)res_lsb;
}

// --- MAIN LOOP (FULL SCAN) ---

int main() {
    // Pair 1
    uint8_t seed1[4] = {0xD5, 0xDA, 0x2A, 0xE9};
    uint32_t expected1 = 0x3F3F77FF;

    // Pair 2
    uint8_t seed2[4] = {0x12, 0x44, 0x9A, 0xC8};
    uint32_t expected2 = 0x3B3B7E3E;

    printf("Starting Full Scan (0x0000 - 0xFFFF)...\n");
    printf("----------------------------------------\n");

    uint8_t candidate[2];
    int match_count = 0;

    // Loop through 0 to 65535
    for (int i = 0; i <= 0xFFFF; i++) {
        
        candidate[0] = (i >> 8) & 0xFF;
        candidate[1] = i & 0xFF;

        // Verify against Pair 1
        if (compute_response(candidate, seed1) == expected1) {
            // Verify against Pair 2
            if (compute_response(candidate, seed2) == expected2) {
                
                // PRINT MATCH
                printf("[MATCH FOUND] Key: %04X\n", i);
                
                match_count++;
                
                // Continue loop to find other collisions
            }
        }
    }

    printf("----------------------------------------\n");
    printf("Scan Complete.\n");
    if (match_count == 0) {
        printf("Result: No keys found.\n");
    } else {
        printf("Result: Found %d valid key(s).\n", match_count);
    }

    return 0;
}