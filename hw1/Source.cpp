#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void Encrypt_CaesarCipher(ifstream& input_file, ofstream& output_file, int key);
void Decrypt_CaesarCipher(ifstream& input_file, ofstream& output_file, int key);
void clear_input_buffer();

void Encrypt_CaesarCipher(ifstream& input_file, ofstream& output_file, int key) // Reads each char from ifstream, encrypts it using key, and writes it to ofstream.
{
    char ch;
    while (input_file.get(ch))
    {
        output_file.put(toupper(((ch - 'a' + key) % 26) + 'a')); // Lecture mentioned plaintext inputs are lowercase, but ciphertexts will be uppercase -WM
    }

}

void Decrypt_CaesarCipher(ifstream& input_file, ofstream& output_file, int key) // Reads each char from ifstream, decrypts it using key, and writes it to ofstream.
{
    char ch;
    while (input_file.get(ch))
    {
        output_file.put(tolower(((ch - 'A' - key + 26) % 26) + 'A')); // Lecture mentioned ciphertext inputs are uppercase, but but plaintexts will be lowercase -WM
    }

}

void clear_input_buffer()
{
    cin.ignore(10000, '\n');
}

int main()
{
    int mode_input = 0;
    int key;

    /*
        For debugging purposes I decided to just use 3 files because it made more sense in my head. - William Mitchell

        The flow is now:
            encryption:
                plaintext.txt  -> encrypt ->  ciphertext.txt

            decryption:
                ciphertext.txt -> decrypt ->  decrypted.txt

        I also made it print what file was written to.

    */
    string plaintext_filename = "plaintext.txt";
    string ciphertext_filename = "ciphertext.txt";
    string decrypted_filename = "decrypted.txt";

    ifstream input_file;
    ofstream output_file;

    /*
        I also cleared files before opening for best practice since the file states might get buggy. Idk, yet to see it break,
        so probably not totally needed. -WM
    */

    while (1)
    {
        // Output display
        printf("=================================\n");
        printf("1. Encryption\n");
        printf("2. Decryption\n");
        printf("3. Cryptanalysis \n");
        printf("4. Exit\n");
        printf("Enter your choice: ");

        mode_input = 0; // Reset mode for input
        if (!(cin >> mode_input)) // Enter mode
        {
            cout << "Invalid input." << endl;
            cin.clear();  // Clear error flags
            clear_input_buffer();
            continue;
        }
        if (mode_input < 1 || mode_input > 4)
        {
            cout << "Invalid mode.\n\n" << endl;
            continue;
        }

        switch (mode_input)
        {
            case 1: // Encryption 
            {
                // Enter key
                cout << "Enter encryption key: ";
                if (!(cin >> key))
                {
                    cout << "Invalid key." << endl;
                    cin.clear();
                    clear_input_buffer();
                    break;
                }

                key = ((key % 26) + 26) % 26;

                input_file.clear();
                output_file.clear();


                // Open files
                input_file.open(plaintext_filename.c_str());
                if (!input_file.is_open())
                {
                    cout << "Error: Could not open input file '" << plaintext_filename << "'.\n" << endl;
                    break;
                }

                output_file.open(ciphertext_filename.c_str());
                if (!output_file.is_open())
                {
                    cout << "Error: Could not open output file '" << ciphertext_filename << "'.\n" << endl;
                    input_file.close();
                    break;
                }

                // Encrypt
                cout << "Encrypting..." << endl;
                Encrypt_CaesarCipher(input_file, output_file, key);
                cout << "Encrypting Finished" << endl;
                cout << "\n\nOutput written to '" << ciphertext_filename << "'.\n" << endl;

                // Close files
                input_file.close();
                output_file.close();

                break;
            }
            case 2: // Decryption
            {
                // Enter key
                cout << "Enter decryption key: ";
                if (!(cin >> key))
                {
                    cout << "Invalid key." << endl;
                    cin.clear();
                    clear_input_buffer();
                    break;
                }

                key = ((key % 26) + 26) % 26;

                input_file.clear();
                output_file.clear();

                // Open files
                input_file.open(ciphertext_filename.c_str());
                if (!input_file.is_open())
                {
                    cout << "Error: Could not open input file '" << ciphertext_filename << "'.\n" << endl;
                    break;
                }

                output_file.open(decrypted_filename.c_str());
                if (!output_file.is_open())
                {
                    cout << "Error: Could not create output file '" << decrypted_filename << "'.\n" << endl;
                    input_file.close();
                    break;
                }

                // Decrypt
                cout << "Decrypting..." << endl;
                Decrypt_CaesarCipher(input_file, output_file, key);
                cout << "Decrypting Finished" << endl;
                cout << "\n\nOutput written to '" << decrypted_filename << "'.\n" << endl;

                // Close files
                input_file.close();
                output_file.close();

                break;
            }
            case 3: // Cryptanalysis
            {
                /*
                    Assumptions:
                        1. 'E' is the most common letter in the english language.
                        2. Need several words in ciphertext before that is almost universally true
                */
                input_file.clear();
                output_file.clear();

                input_file.open(ciphertext_filename.c_str());
                if (!input_file.is_open())
                {
                    cout << "Error: Could not open input file '" << ciphertext_filename << "'.\n" << endl;
                    break;
                }

                output_file.open(decrypted_filename.c_str());
                if (!output_file.is_open())
                {
                    cout << "Error: Could not create output file '" << decrypted_filename << "'.\n" << endl;
                    input_file.close();
                    break;
                }

                // Current count of all letters in file: frequency[0] = A, frequency[1] = B, etc...
                int frequency[26] = { 0 };
                char ch;

                
                while (input_file.get(ch))
                {
                    // I couldn't really figure out if all ciphertext inputs would be uppercase, all of profs examples were, so I just called toupper on all of the chars -WM
                    ch = toupper(static_cast<unsigned char>(ch));

                    if (ch >= 'A' && ch <= 'Z')
                    {
                        frequency[ch - 'A']++; // ex. B - A = [1] and then you increment the count. You are adding up total of every specific letter.
                    }
                }

                // Find most common letter in ciphertext
                int max_index = 0;
                for (int i = 1; i < 26; i++)
                {
                    if (frequency[i] > frequency[max_index]) // if new letter is bigger than current...
                    {
                        max_index = i; // new i'th letter becomes max index
                    }
                }

                char most_common_cipher_letter = 'A' + max_index; // Returns the letter, ex: A + (letter 2) = C most common cipher letter

                // Assume most common ciphertext letter maps to plaintext 'E'
                int guessed_key = (most_common_cipher_letter - 'E' + 26) % 26; // Prof said E is most common no matter what, get the shift key

                cout << "Running cryptanalysis..." << endl;
                cout << "Most common ciphertext letter: " << most_common_cipher_letter << endl;
                cout << "Guessed key: " << guessed_key << endl;

                // Reset stream to beginning for second pass
                input_file.clear();
                input_file.seekg(0, ios::beg);

                // Decrypt using guessed key
                while (input_file.get(ch))
                {

                    if (ch >= 'A' && ch <= 'Z')
                    {
                        output_file.put(tolower(((ch - 'A' - guessed_key + 26) % 26) + 'A')); // All plaintexts are apparently lowercase, even though this is written to decrypt.txt
                    }
                    else
                    {
                        output_file.put(ch);
                    }
                }

                cout << "Cryptanalysis finished." << endl;
                cout << "Output written to '" << decrypted_filename << "'.\n" << endl;

                input_file.close();
                output_file.close();

                break;
            }
            case 4: // Exit
            {
                return 0;
            }
        }

        printf("\n");
    }

    return 0;
}