// #include <functional>
// #include <cstdio>
// #include <cfenv>
// #include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
// #include <istream>
// #include <sstream>
#include <netinet/in.h>
// #include <sstream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
// #include <string.h>
// #include <filesystem>
#include <vector>

// namespace fs = std::filesystem; 
using namespace std;

#include <iostream>
#include <vector>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <fstream>


class SimpleReadline
{
    private:
        vector<string> history;
        struct termios orig_termios;

        void enableRawMode()
        {
            tcgetattr(STDIN_FILENO, &orig_termios);
            struct termios raw = orig_termios;
            raw.c_lflag &= ~(ECHO | ICANON);
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        }

        void disableRawMode()
        {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        }

    public:
        SimpleReadline()
        {
            enableRawMode();
        }

        ~SimpleReadline()
        {
            disableRawMode();
        }

        string readLine(const string &__pwd)
        {
            string line;
            char c;
            string shell("server-$ ");
            string prompt(shell);

            int length(prompt.length() + 1);
            int cursorPos(0);
            int historyIndex(history.size());
            cout << prompt;
            cout.flush();

            while((read(STDIN_FILENO, &c, 1) == 1) && (c != '\n'))
            {
                // functions
                if(c == '\033')
                {
                    read(STDIN_FILENO, &c, 1);
                    read(STDIN_FILENO, &c, 1);
                    
                    // Up and Down Arrows (History Navigation)
                    if(c == 'A' || c == 'B')
                    {
                        // Up arrow
                        if(c == 'A' && historyIndex > 0)
                        {
                            if(!line.empty())
                            {
                                history[historyIndex] = line;
                            }

                            historyIndex--;
                            std::cout << "\033[2K\r" << prompt << history[historyIndex];
                            line = history[historyIndex];
                            cursorPos = line.length();
                            std::cout.flush();
                            continue;
                        }

                        // Down arrow
                        if(c == 'B' && historyIndex < history.size())
                        {
                            history[historyIndex] = line;
                            historyIndex++;
                            std::cout << "\033[2K\r" << prompt << history[historyIndex];
                            line = history[historyIndex];
                            cursorPos = line.length();
                            std::cout.flush();
                            continue;
                        }

                        // Down arrow while line is empty will do nothing
                        if(c == 'B' && line.empty())
                        {
                            continue;
                        }

                        // // Down arrow at last entry
                            // if(c == 'B' && historyIndex >= history.size())
                            // {
                            //     if(historyIndex == history.size())
                            //     {
                            //         history[historyIndex] = line;
                            //         historyIndex++;
                            //     }
                            
                            //     if (historyIndex - 1 == history.size())
                            //     {
                            //         history[historyIndex - 1] = line;
                            //     }
                                
                            //     std::cout << "\033[2K\r" << prompt;
                            //     line.clear();
                            //     cursorPos = line.length();
                            //     std::cout.flush();
                            //     continue;
                            // }

                        continue;
                    }

                    // Right and Left Arrows (Cursor Movement)
                    if(c == 'C' || c == 'D')
                    {
                        // Right arrow
                        if(c == 'C')
                        {
                            if(cursorPos < line.size())
                            {
                                cursorPos++;
                                std::cout << "\033[" << (0) << "C";
                                std::cout.flush();
                                continue;
                            }

                            continue;
                        }

                        // Left arrow
                        if(c == 'D')
                        {
                            if(cursorPos > 0)
                            {
                                cursorPos--;
                                std::cout << "\033[" << (0) << "D";
                                std::cout.flush();
                                continue;
                            }

                            continue;
                        }

                        continue;
                    }

                    // Ctrl+
                    if(c == '1')
                    {
                        read(STDIN_FILENO, &c, 1); // read ';'
                        read(STDIN_FILENO, &c, 1); // read '5'
                        read(STDIN_FILENO, &c, 1); // read actual command (D for left, C for right)
                    
                        // Ctrl + Left
                        if(c == 'D')
                        {
                            while(cursorPos > 0 && line[cursorPos - 1] == ' ') 
                            {
                                cursorPos--;
                            }

                            while(cursorPos > 0 && line[cursorPos - 1] != ' ') 
                            {    
                                cursorPos--;
                            }

                            std::cout << "\033[2K\r" << prompt << line;
                            std::cout << "\033[" << (cursorPos + length) << "G";
                            std::cout.flush();
                            continue;
                        }

                        // Ctrl + Right
                        if(c == 'C')
                        {
                            while(cursorPos < line.size() && line[cursorPos + 1] != ' ')
                            {
                                cursorPos++;
                            }

                            while(cursorPos < line.size() && line[cursorPos + 1] == ' ')
                            {
                                cursorPos++;
                            }

                            std::cout << "\033[2K\r" << prompt << line;
                            std::cout << "\033[" << (cursorPos + length) << "G";
                            std::cout.flush();
                            continue;
                        }
                    }

                    continue;
                }

                // Backspace
                if(c == '\x7F')
                {
                    if (cursorPos > 0)
                    {
                        line.erase(cursorPos - 1, 1);
                        cursorPos--;
                        std::cout << "\033[2K\r" << prompt << line;
                        std::cout << "\033[" << (cursorPos + length) << "G";
                        std::cout.flush();
                        continue;
                    }

                    continue;
                }

                // if CTRL+D is pressed return exit
                if(c == 4)
                {
                    return "exit";
                }

                // do nothing when these are pressed
                if(
                    c == 1 ||   // CTRL+A
                    c == 2 ||   // CTRL+B
                    c == 5 ||   // CTRL+E
                    c == 6 ||   // CTRL+F
                    c == 7 ||   // CTRL+G
                    c == 8 ||   // CTRL+H
                    c == 9 ||   // CTRL+I
                    c == 11 ||  // CTRL+K
                    c == 12 ||  // CTRL+L
                    c == 14 ||  // CTRL+N
                    c == 15 ||  // CTRL+O
                    c == 16 ||  // CTRL+P
                    c == 18 ||  // CTRL+R
                    c == 20 ||  // CTRL+T
                    c == 21 ||  // CTRL+U
                    c == 22 ||  // CTRL+V
                    c == 23 ||  // CTRL+W
                    c == 24 ||  // CTRL+X
                    c == 25 ||  // CTRL+Y
                    c == 29     /*  CTRL+] */)
                {
                    continue;
                }

                // append normal letters to line
                else
                {
                    if(cursorPos >= 0)
                    {
                        line.insert(cursorPos, 1, c);
                        cursorPos++;
                        std::cout << "\033[2K\r" << prompt << line;
                        std::cout << "\033[" << (cursorPos + length) << "G";
                        std::cout.flush();
                        continue;
                    }
                }
            }

            cout << endl;
            if(!line.empty())
            {    
                history.push_back(line);
            }

            return line;
        };

        void loadHistoryFromFile(const string &filePath)
        {
            ifstream historyFile(filePath);
            if(historyFile.is_open())
            {
                string line;
                while(getline(historyFile, line))
                {
                    history.push_back(line);  // use the history vector of your SimpleReadline object
                }

                historyFile.close();
            }
        }

        void appendHistoryToFile(const string &line, const string &filePath)
        {
            ofstream historyFile(filePath, ios::app); // Open file in append mode
            if (historyFile.is_open())
            {
                historyFile << line << endl;  // Append the line and a newline character to the file
                historyFile.close();
            }
        }
};

string __send_request__(int __socket, const string &__input_string)
{
    if(__input_string.empty())
    {
        return 0;
    }

    if(send(__socket, __input_string.c_str(), __input_string.length(), 0) < 0)
    {
        perror(__func__);
        return 0;
    }

    int __response_buffer_size(1024);
    char __response_buffer[__response_buffer_size];
    while(read(__socket, __response_buffer, __response_buffer_size) > 0);
    return __response_buffer;
}

void __assign_and_init__(long *__socket, sockaddr_in *__server, int __port, const char *__server_ip_address)
{
    if((*__socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket: ");
        close(*__socket);
        exit(EXIT_FAILURE);
    }
    __server->sin_family = AF_INET;
    __server->sin_port = htons(__port);

    if(inet_pton(AF_INET, __server_ip_address, &__server->sin_addr) < 0)
    {
        perror("inet_pton: ");
        close(*__socket);
        exit(EXIT_FAILURE);
    }

    if(connect(*__socket, (struct sockaddr *)__server, sizeof(*__server)) < 0)
    {
        perror("connect: ");
        close(*__socket);
        exit(EXIT_FAILURE);
    }
}

enum
{
    BUFFER_SIZE = 10240
};

int main(int argc, char *argv[])
{
    if(argc > 2)
    {
        return 1;
    }

    string __server_ip_address;
    if (argc == 2)
    {
        __server_ip_address = argv[1];
    }
    else
    {
        __server_ip_address = "192.168.17.228";
    }

    long __socket(0);
    uint16_t __port(8001);
    struct sockaddr_in __server;
    char buffer[BUFFER_SIZE];
    // const char *__prompt(" --> : ");
    string __msg_str__("");
    SimpleReadline srl;

    while(true)
    {
        // init socket

            if((__socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("socket: ");
                close(__socket);
                exit(EXIT_FAILURE);
            }
            __server.sin_family = AF_INET;
            __server.sin_port = htons(__port);

            if(inet_pton(AF_INET, __server_ip_address.c_str(), &__server.sin_addr) < 0)
            {
                perror("inet_pton: ");
                close(__socket);
                exit(EXIT_FAILURE);
            }

            if(connect(__socket, (struct sockaddr *)&__server, sizeof(__server)) < 0)
            {
                perror("connect: ");
                close(__socket);
                exit(EXIT_FAILURE);
            }
            
        __msg_str__ = srl.readLine("");
        __msg_str__ += '\0';

        if(send(__socket, __msg_str__.c_str(), __msg_str__.length(), 0) < 0)
        {
            perror("send: ");
            close(__socket);
            exit(EXIT_FAILURE);
        }

        // look if server responds
        char __char = '\0';
        while(read(__socket, &__char, 1) > 0)
        {
            if (__char == '\0')
            {
                break;
            }
            cout << __char;
        }
        cout.flush();
    }

    close(__socket);
    return 0;
}