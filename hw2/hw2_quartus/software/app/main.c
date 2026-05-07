#include <stdio.h>
#include <stdint.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"

unsigned int permute(unsigned int input, const int table[], int table_size, int input_bits);
unsigned int left_circular_shift(unsigned int value, int bits, int shifts);
unsigned int sbox_lookup(const int sbox[4][4], unsigned int input_4bit);
void generate_keys(unsigned int key_10bit, unsigned int *key1_8bit, unsigned int *key2_8bit);
unsigned int round_function(unsigned int input_4bit, unsigned int key_8bit);
unsigned int encrypt(unsigned int input, unsigned int key_10bit);
unsigned int decrypt(unsigned int encrypted, unsigned int key_10bit);
void print_bits(unsigned int value, int length);

const int P10[] = {3, 5, 2, 7, 4, 10, 1, 9, 8, 6};
const int P8[] = {6, 3, 7, 4, 8, 5, 10, 9};
const int IP[] = {2, 6, 3, 1, 4, 8, 5, 7};
const int IP_INV[] = {4, 1, 3, 5, 7, 2, 8, 6}; // Must Invert Initial Permutation
const int EP[] = {4, 1, 2, 3, 2, 3, 4, 1};
const int P4[] = {2, 4, 3, 1};

const int S0[4][4] = {
    {1, 0, 3, 2},
    {3, 2, 1, 0},
    {0, 2, 1, 3},
    {3, 1, 3, 2}
};

const int S1[4][4] = {
    {0, 1, 2, 3},
    {2, 0, 1, 3},
    {3, 0, 1, 0},
    {2, 1, 0, 3}
};

unsigned int permute(unsigned int input, const int table[], int table_size, int input_bits)
{
    unsigned int output = 0;
    
    for (int i = 0; i < table_size; i++)
	{
        output <<= 1;
        int bit_pos = input_bits - table[i];
        output |= (input >> bit_pos) & 1;
    }
    
    return output;
}

unsigned int left_circular_shift(unsigned int value, int bits, int shifts)
{
    shifts = shifts % bits;
    unsigned int mask = (1 << bits) - 1;
    return ((value << shifts) | (value >> (bits - shifts))) & mask;
}

unsigned int sbox_lookup(const int sbox[4][4], unsigned int input_4bit)
{
    // Row = first bit (bit 3) and last bit (bit 0)
    int row = ((input_4bit >> 3) & 1) * 2 + (input_4bit & 1);
    
    // Column = middle two bits (bit 2 and bit 1)
    int col = ((input_4bit >> 2) & 1) * 2 + ((input_4bit >> 1) & 1);
    
    return (unsigned int)(sbox[row][col] & 0x03);
}

void generate_keys(unsigned int key_10bit, unsigned int *key1_8bit, unsigned int *key2_8bit)
{
    // P10
    unsigned int p10_key = permute(key_10bit, P10, 10, 10);
    
    // Split into 5-bit halves
    unsigned int left = (p10_key >> 5) & 0x1F;
    unsigned int right = p10_key & 0x1F;
    
    // Left shift each half by 1
    left = left_circular_shift(left, 5, 1);
    right = left_circular_shift(right, 5, 1);
    
    // Combine and apply P8 to get Key 1
    unsigned int combined = (left << 5) | right;
    *key1_8bit = permute(combined, P8, 8, 10);
    
    // Left shift each half by 2 more positions for Key 2
    left = left_circular_shift(left, 5, 1);
    right = left_circular_shift(right, 5, 1);
    
    // Combine and apply P8 to get Key 2
    combined = (left << 5) | right;
    *key2_8bit = permute(combined, P8, 8, 10);
}

unsigned int round_function(unsigned int input_4bit, unsigned int key_8bit)
{
    // Expand R from 4 to 8 bits
    unsigned int expanded = permute(input_4bit, EP, 8, 4);
    
    // XOR with key
    unsigned int xored = expanded ^ key_8bit;
    
    // Split into left and right 4 bits
    unsigned int left_4bit = (xored >> 4) & 0x0F;
    unsigned int right_4bit = xored & 0x0F;
    
    // Apply S-boxes (4 bits → 2 bits each)
    unsigned int s0_out = sbox_lookup(S0, left_4bit);
    unsigned int s1_out = sbox_lookup(S1, right_4bit);
    
    // Combine to 4 bits
    unsigned int combined_sbox = (s0_out << 2) | s1_out;
    
    // Apply P4 permutation
    unsigned int p4_out = permute(combined_sbox, P4, 4, 4);
    
    return p4_out;
}

unsigned int encrypt(unsigned int input, unsigned int key_10bit)
{
    unsigned int key1_8bit, key2_8bit;
    
    generate_keys(key_10bit, &key1_8bit, &key2_8bit);
    
    // Initial Permutation
    unsigned int ip_output = permute(input, IP, 8, 8);
    
    // Split into left and right halves (4 bits each)
    unsigned int left = (ip_output >> 4) & 0x0F;
    unsigned int right = ip_output & 0x0F;
    
    // Round 1 (using Key 1)
    unsigned int f_output = round_function(right, key1_8bit);
    unsigned int new_right = left ^ f_output;
    unsigned int new_left = right;
    
    // Round 2 (using Key 2)
    f_output = round_function(new_right, key2_8bit);
    unsigned int final_left = new_left ^ f_output;
    unsigned int final_right = new_right;
    
    // Combine halves
    unsigned int combined = (final_left << 4) | final_right;
    
    // Invert Initial Permutation
    unsigned int encrypted = permute(combined, IP_INV, 8, 8);
	
    return encrypted;
}

unsigned int decrypt(unsigned int encrypted, unsigned int key_10bit)
{
    unsigned int key1_8bit, key2_8bit;
    
    // Generate subkeys
    generate_keys(key_10bit, &key1_8bit, &key2_8bit);
    
    // Initial Permutation
    unsigned int ip_output = permute(encrypted, IP, 8, 8);
    
    // Split into halves
    unsigned int left = (ip_output >> 4) & 0x0F;
    unsigned int right = ip_output & 0x0F;
    
    // Round 1 with Key 2 (reversed order)
    unsigned int f_output = round_function(right, key2_8bit);
    unsigned int new_right = left ^ f_output;
    unsigned int new_left = right;
    
    // Round 2 with Key 1 (reversed order)
    f_output = round_function(new_right, key1_8bit);
    unsigned int final_left = new_left ^ f_output;
    unsigned int final_right = new_right;
    
    // Combine and apply inverse IP
    unsigned int combined = (final_left << 4) | final_right;
    unsigned int output = permute(combined, IP_INV, 8, 8);
    
    return output;
}

void print_bits(unsigned int value, int length)
{
    for(int i = length-1; i >= 0; i--)
    {
        printf("%d", (value >> i) & 1);
    }
}

int main (void)
{
    printf("Program Started...\r\n");

    unsigned int buttons;
	unsigned int input;
	unsigned int key;
	unsigned int encrypted;
	unsigned int output;
	
	while(1)
	{
		printf("Enter 10 bit Key.\r\n");

		while (buttons & 0x01)
		{
			buttons = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE);
		}
		while (!(buttons & 0x01))
		{
			buttons = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE);
		}

		buttons = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE);
		key = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);

		printf("Key: ");
		print_bits(key, 10);
		printf("\r\n");
		

		printf("Enter 8 bit input.\r\n");

		while (buttons & 0x01)
		{
			buttons = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE);
		}
		while (!(buttons & 0x01))
		{
			buttons = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE);
		}

		input = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE); // Read 10 bits
		input = input & 0xFF;  // Mask to 8 bits
		
		printf("Input: ");
		print_bits(input, 8);
		printf("\r\n");


		printf("Encrypting...\r\n");
		encrypted = encrypt(input, key);
		
		printf("Encrypted value: ");
		print_bits(encrypted, 8);
		printf("\r\n");

		IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, (0 | encrypted));


		printf("Decrypting...\r\n");
		output = decrypt(encrypted, key);
		
		printf("Decrypting output: ");
		print_bits(output, 8);
		printf("\r\n\r\n");

	}
}
