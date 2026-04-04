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
        output_file.put(((ch - 'a' + key) % 26) + 'a');
    }

}

void Decrypt_CaesarCipher(ifstream& input_file, ofstream& output_file, int key) // Reads each char from ifstream, decrypts it using key, and writes it to ofstream.
{
    char ch;
    while (input_file.get(ch))
    {
        output_file.put(((ch - 'a' - key + 26) % 26) + 'a');
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

    string input_filename = "plaintext.txt";
    string output_filename = "ciphertext.txt";
    ifstream input_file;
    ofstream output_file;

    while (1)
    {
        // Output display
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
        if (mode_input <= 0 && mode_input >= 4)
        {
            cout << "Invalid mode." << endl;
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

                key = key % 26;

                // Open files
                input_file.open(input_filename.c_str());
                if (!input_file.is_open())
                {
                    cout << "Error: Could not open input file '" << input_filename << "'." << endl;
                    break;
                }

                output_file.open(output_filename.c_str());
                if (!output_file.is_open())
                {
                    cout << "Error: Could not open output file '" << output_filename << "'." << endl;
                    input_file.close();
                    break;
                }

                // Encrypt
                cout << "Encrypting..." << endl;
                Encrypt_CaesarCipher(input_file, output_file, key);
                cout << "Encrypting Finished" << endl;

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

                key = key % 26;

                // Open files
                input_file.open(output_filename.c_str());
                if (!input_file.is_open())
                {
                    cout << "Error: Could not open input file '" << output_filename << "'." << endl;
                    break;
                }

                output_file.open(input_filename.c_str());
                if (!output_file.is_open())
                {
                    cout << "Error: Could not create output file '" << input_filename << "'." << endl;
                    input_file.close();
                    break;
                }

                // Decrypt
                cout << "Decrypting..." << endl;
                Decrypt_CaesarCipher(input_file, output_file, key);
                cout << "Decrypting Finished" << endl;

                // Close files
                input_file.close();
                output_file.close();

                break;
            }
            case 3: // Cryptanalysis
            {
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