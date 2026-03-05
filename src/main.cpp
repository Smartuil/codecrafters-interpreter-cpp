#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::string read_file_contents(const std::string& filename);

int main(int argc, char *argv[])
{
    // Disable output buffering
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here!" << std::endl;

    if (argc < 3)
    {
        std::cerr << "Usage: ./your_program tokenize <filename>" << std::endl;
        return 1;
    }

    const std::string command = argv[1];

    if (command == "tokenize")
    {
        std::string file_contents = read_file_contents(argv[2]);
        bool has_error = false;
        int line = 1;
        
        for (size_t i = 0; i < file_contents.size(); i++)
        {
            char c = file_contents[i];
            switch (c)
            {
                case '(':
                    std::cout << "LEFT_PAREN ( null" << std::endl;
                    break;
                case ')':
                    std::cout << "RIGHT_PAREN ) null" << std::endl;
                    break;
                case '{':
                    std::cout << "LEFT_BRACE { null" << std::endl;
                    break;
                case '}':
                    std::cout << "RIGHT_BRACE } null" << std::endl;
                    break;
                case ',':
                    std::cout << "COMMA , null" << std::endl;
                    break;
                case '.':
                    std::cout << "DOT . null" << std::endl;
                    break;
                case '-':
                    std::cout << "MINUS - null" << std::endl;
                    break;
                case '+':
                    std::cout << "PLUS + null" << std::endl;
                    break;
                case ';':
                    std::cout << "SEMICOLON ; null" << std::endl;
                    break;
                case '*':
                    std::cout << "STAR * null" << std::endl;
                    break;
                case '=':
                    if (i + 1 < file_contents.size() && file_contents[i + 1] == '=')
                    {
                        std::cout << "EQUAL_EQUAL == null" << std::endl;
                        i++;
                    }
                    else
                    {
                        std::cout << "EQUAL = null" << std::endl;
                    }
                    break;
                case '!':
                    if (i + 1 < file_contents.size() && file_contents[i + 1] == '=')
                    {
                        std::cout << "BANG_EQUAL != null" << std::endl;
                        i++;
                    }
                    else
                    {
                        std::cout << "BANG ! null" << std::endl;
                    }
                    break;
                case '<':
                    if (i + 1 < file_contents.size() && file_contents[i + 1] == '=')
                    {
                        std::cout << "LESS_EQUAL <= null" << std::endl;
                        i++;
                    }
                    else
                    {
                        std::cout << "LESS < null" << std::endl;
                    }
                    break;
                case '>':
                    if (i + 1 < file_contents.size() && file_contents[i + 1] == '=')
                    {
                        std::cout << "GREATER_EQUAL >= null" << std::endl;
                        i++;
                    }
                    else
                    {
                        std::cout << "GREATER > null" << std::endl;
                    }
                    break;
                case '/':
                    if (i + 1 < file_contents.size() && file_contents[i + 1] == '/')
                    {
                        while (i < file_contents.size() && file_contents[i] != '\n')
                        {
                            i++;
                        }
                        i--;
                    }
                    else
                    {
                        std::cout << "SLASH / null" << std::endl;
                    }
                    break;
                case '"':
                {
                    size_t start = i;
                    i++;
                    while (i < file_contents.size() && file_contents[i] != '"')
                    {
                        if (file_contents[i] == '\n')
                        {
                            line++;
                        }
                        i++;
                    }
                    if (i >= file_contents.size())
                    {
                        std::cerr << "[line " << line << "] Error: Unterminated string." << std::endl;
                        has_error = true;
                    }
                    else
                    {
                        std::string lexeme = file_contents.substr(start, i - start + 1);
                        std::string literal = file_contents.substr(start + 1, i - start - 1);
                        std::cout << "STRING " << lexeme << " " << literal << std::endl;
                    }
                    break;
                }
                default:
                    if (c == '\n')
                    {
                        line++;
                    }
                    else if (c != ' ' && c != '\t' && c != '\r')
                    {
                        std::cerr << "[line " << line << "] Error: Unexpected character: " << c << std::endl;
                        has_error = true;
                    }
                    break;
            }
        }
        std::cout << "EOF  null" << std::endl;
        
        if (has_error)
        {
            return 65;
        }
    }
    else
    {
        std::cerr << "Unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}

std::string read_file_contents(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error reading file: " << filename << std::endl;
        std::exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}
