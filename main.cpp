// #include <cstring>
// #include <cstring>
// #include <functional>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
// #include <functional>
#include <exception>
#include <iostream>
// #include <sstream>
// #include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
// #include <type_traits>
#include <thread>
#include <unistd.h>
// #include <microhttpd.h>
#include <ctime>
#include <unistd.h> // For close()
#include <stdlib.h> // For exit()
#include <sys/socket.h> // For socket(), connect()
#include <netinet/in.h> // For sockaddr_in
#include <arpa/inet.h> // For inet_addr()
#include <unistd.h> // For close()
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include <iostream>
#include <netdb.h>
#include <ifaddrs.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
// #include <vector>
#include <filesystem>
#include <sys/stat.h>


#define PORT 8001
namespace fs = std::filesystem;
using namespace std;

namespace
{
    namespace tools
    {
        string __time__()
        {
            long now(time({}));
            char buf[80];
            strftime(
                buf,
                sizeof(buf),
                "%Y-%m-%d %H:%M:%S", 
                localtime(&now)
            );

            string result(buf);
            return "\033[32m[\033[35m" + result + "\033[32m]\033[0m";
        }

        void _fatal_error(const string &__msg)
        {
            string err_msg(__time__() + " - \033[31mERROR\033[0m: " + __msg);
            perror(err_msg.c_str());
            exit(EXIT_FAILURE);
        }
    }
    using namespace tools;
    
    namespace // c_ls.
    {
        bool is_executable(const fs::directory_entry& entry)
        {
            struct stat st;
            if(lstat(entry.path().c_str(), &st) == 0)
            {
                return (st.st_mode & S_IXUSR) != 0;
            }

            return false;
        }

        bool endsWithExtension(const fs::directory_entry& entry, const std::string &extension)
        {
            string entryPath = entry.path().string();
            size_t dotPosition = entryPath.rfind('.');
            if(dotPosition != string::npos)// Compare the entry's extension with the desired extension (case-insensitive)
            {             
                string entryExtension = entryPath.substr(dotPosition);
                return entryExtension == extension;
            }

            return false;                                       // No extension found in the entry's path
        }

        string c_ls(const std::string &full_PATH_to_dir, bool list_hidden = false)
        {
            std::stringstream ss;

            ss << "Contents of directory ( " << full_PATH_to_dir << " ):\n";
            try
            {
                std::vector<fs::directory_entry> entries;       // Create a vector to store directory entries
                for(const auto &entry : fs::directory_iterator(full_PATH_to_dir))
                {
                    if(!list_hidden && entry.path().filename().string().front() == '.')// If list_hidden is false, skip hidden files and directories
                    {  
                        continue;
                    }

                    entries.push_back(entry);                   // Store the directory entries in the vector
                }

                std::sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {  // Sort the vector alphabetically by filename
                    return a.path().filename() < b.path().filename();
                });

                for(const auto &entry : entries)// Print the sorted contents with color coding for directories and executables
                {             
                    ss << (endsWithExtension(entry, ".msi")  ? "\033[32m" : "");  // Set text color to    green   for     .msi    files
                    ss << (endsWithExtension(entry, ".exe")  ? "\033[32m" : "");  // Set text color to    green   for     .exe    files
                    ss << (endsWithExtension(entry, ".sh")   ? "\033[32m" : "");  // Set text color to    green   for     .sh     files
                    ss << (endsWithExtension(entry, ".gz")   ? "\033[31m" : "");  // Set text color to    red     for     .gz     files
                    ss << (endsWithExtension(entry, ".xz")   ? "\033[31m" : "");  // Set text color to    red     for     .xz     files
                    ss << (endsWithExtension(entry, ".h")    ? "\033[35m" : "");  // Set text color to    red     for     .h      files
                    ss << (endsWithExtension(entry, ".conf") ? "\033[33m" : "");  // Set text color to    red     for     .conf   files
                    ss << (endsWithExtension(entry, ".c")    ? "\033[92m" : "");  // Set text color to    red     for     .c      files
                    ss << (endsWithExtension(entry, ".cpp")  ? "\033[92m" : "");  // Set text color to    red     for     .cpp    files
                    ss << (is_executable(entry) ? "\033[32m" : "");               // Set text color to    green   for     exec    files
                    ss << (entry.is_directory() ? "\e[1m\e[34m" : "");            // Set text color to    blue    for     dirs
                    ss << entry.path().filename();
                    ss << "\033[0m"; // Reset text color to default
                    ss << std::endl; // Print a newline
                }
            }
            catch(const std::exception& e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
            }
            
            return ss.str();
        }
    }

    class __Console
    {
        public:
            __Console() {}

            template<typename T>
            void out(const T& value)
            {
                cout << value; // Process the single argument, e.g., by printing it
            }

            template<typename T, typename... Args>
            void out(const T& first, const Args&... rest)
            {
                cout << first;
                out(rest...);
            }

            void out()
            {
                cout << '\n';
            }
    };
    __Console *console;
}

const char *__get_local_ip__()
{
    int32_t __socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(__socket == -1)
    {
        perror("socket");
        return nullptr;
    }

    struct sockaddr_in __google_dns;
    memset(&__google_dns, 0, sizeof(__google_dns));

    __google_dns.sin_family      = AF_INET;
    __google_dns.sin_port        = htons(80);
    __google_dns.sin_addr.s_addr = inet_addr("8.8.8.8");

    if(connect(__socket, (const struct sockaddr*)&__google_dns, sizeof(__google_dns)) < 0)
    {
        perror("connect");
        close(__socket);
        return nullptr;
    }

    struct sockaddr_in __response;
    socklen_t __response_len(sizeof(__response));
    memset(&__response, 0, __response_len);

    if(getsockname(__socket, (struct sockaddr*)&__response, &__response_len) < 0)
    {
        perror("getsockname");
        close(__socket);
        return nullptr;
    }

    char __buffer[INET_ADDRSTRLEN];
    return inet_ntop(AF_INET, &__response.sin_addr, __buffer, INET_ADDRSTRLEN);
}

std::string getLocalIPAddress() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Use UDP socket
    if (sockfd == -1) {
        std::cerr << "Could not create socket" << std::endl;
        return "";
    }

    struct sockaddr_in serv{};
    memset(&serv, 0, sizeof(serv));
    serv.sin_family      = AF_INET;
    serv.sin_addr.s_addr = inet_addr("8.8.8.8"); // Google's public DNS server
    serv.sin_port        = htons(80);

    // Connect to the server
    int err = connect(sockfd, (const struct sockaddr*)&serv, sizeof(serv));
    if (err == -1) {
        std::cerr << "Could not connect to server" << std::endl;
        close(sockfd);
        return "";
    }

    struct sockaddr_in name{};
    socklen_t namelen = sizeof(name);
    err = getsockname(sockfd, (struct sockaddr*)&name, &namelen);
    if (err == -1) {
        std::cerr << "Could not get socket name" << std::endl;
        close(sockfd);
        return "";
    }

    char buffer[INET_ADDRSTRLEN];
    const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, sizeof(buffer));
    if (p == nullptr) {
        std::cerr << "Could not convert address to string" << std::endl;
        close(sockfd);
        return "";
    }

    std::string localIP(p);
    close(sockfd);
    return localIP;
}

char** vectorToArgv(const std::vector<std::string>& vec)
{
    char** argv = new char*[vec.size() + 1];  // Allocate memory for argv array. +1 for the nullptr terminator.
    for (size_t i = 0; i < vec.size(); ++i)
    {
        argv[i] = const_cast<char*>(vec[i].c_str());  // Store pointer to string's character data in argv array.
    }
    argv[vec.size()] = nullptr;  // Add nullptr terminator to argv array.
    return argv;
}

void __check_config__()
{
    if(chdir(getenv("HOME")))
    {
        throw runtime_error("chdir: ERROR: Failed to change to home dir.\n");
    }

    string __main_folder_path(fs::current_path().string() + "/.server");

    if(!fs::exists(__main_folder_path))
    {
        try
        {
            fs::create_directory(__main_folder_path);
        }
        catch(exception &e)
        {
            throw runtime_error("Could not create main folder for server. ERROR: " + string(e.what()));
        }
    }

    if(!fs::is_directory(__main_folder_path))
    {
        throw runtime_error("main folder already exists and is not a directory");
    }
    
    if(chdir(__main_folder_path.c_str()))
    {
        throw runtime_error("chdir: Could not change into server working directory.");
    }

    try
    {
        vector<string> __entry_vector;
        for(const fs::directory_entry &__entry : fs::directory_iterator(fs::current_path()))
        {
            string __full_path(__entry.path().string());
            int __pos;
            for(int i = 0; i < __full_path.length(); ++i)
            {
                if(__full_path[i] == '/')
                {
                    __pos = i + 1;
                }
            }
            __entry_vector.push_back(string(__full_path.substr(__pos, __full_path.length() - __pos)));
        }
        
        if(__entry_vector.empty())
        {
            vector<string> __folder_vector =
            {
                "downloads",
                "tmp",
                "data",
                ".conf"
            };

            for(const string &__folder : __folder_vector)
            {
                try
                {
                    fs::create_directory(__main_folder_path + "/" + __folder);
                }
                catch(exception &__e)
                {
                    throw runtime_error(__e.what());
                }
            }
        }
    }
    catch(exception &e)
    {
        cerr << __func__ << ": ERROR: " << e.what() << "\n";
    }
}

int __setup__socket__(struct sockaddr_in *__address)
{
    int __socket;
    uint __opt_code(1);

    if((__socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(__socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &__opt_code, sizeof(__opt_code)) < 0)
    {
        perror(string("setsockopt: ").c_str());
        close(__socket);
        exit(EXIT_FAILURE);
    }
    __address->sin_family = AF_INET;
    __address->sin_port = htons(8001);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if((__address->sin_addr.s_addr = inet_addr(getLocalIPAddress().c_str())) <= 0)
    {
        perror(string("inet_addr: ").c_str());
        close(__socket);
        exit(EXIT_FAILURE);
    }

    // Bind socket to the port 8080
    if(bind(__socket, (struct sockaddr *)__address, sizeof(*__address)) < 0)
    {
        perror(string("bind: ").c_str());
        close(__socket);
        exit(EXIT_FAILURE);
    }

    // Set up listen
    if(listen(__socket, 3) < 0)
    {
        perror(string("listen: ").c_str());
        close(__socket);
        exit(EXIT_FAILURE);
    }

    return __socket;
}

vector<string> combineArgsBetweenQuotes(const vector<string>& input)
{
    vector<string> result;
    bool insideQuotes = false;
    std::string combinedArg;

    for (const std::string &arg : input)
    {
        if (arg.front() == '"' && arg.back() == '"')
        {
            result.push_back(arg.substr(1, arg.size() - 2));
        }
        else if (arg.front() == '"' && !insideQuotes)
        {
            // Start of a quoted argument
            insideQuotes = true;
            combinedArg = arg.substr(1);
        }
        else if (arg.back() == '"' && insideQuotes)
        {
            // End of a quoted argument
            combinedArg += " " + arg.substr(0, arg.size() - 1);
            result.push_back(combinedArg);
            insideQuotes = false;
        }
        else if (insideQuotes)
        {
            // Inside a quoted argument, concatenate it
            combinedArg += " " + arg;
        }
        else
        {
            // Not inside quotes, add as is
            result.push_back(arg);
        }
    }

    return result;
}

int main()
{
    try
    {
        __check_config__();
    }
    catch(exception &e)
    {
        cerr << e.what();
    }
    
    // const char *__ip_address = __get_local_ip__();
    int32_t __sock[2];
    uint32_t __opt_code(1);

    // Setup server.
    struct sockaddr_in __address;
    socklen_t __address_len(sizeof(__address));
    memset(&__address, 0, __address_len);
    __sock[0] = __setup__socket__(&__address);
    
    console->out('\n', __time__(), " - \033[92mSuccessfully Created Socket\033[0m.\n");
    console->out(__time__(), " - \033[92mServer is listening on: \033[0m", inet_ntoa(__address.sin_addr), ":\033[94m", PORT, "\033[0m\n");

    while(true)
    {
        if((__sock[1] = accept(__sock[0], (struct sockaddr *)&__address, (socklen_t*)&__address_len)) < 0) // Wait for request.
        {
            _fatal_error("Accept failed.");
        }

        stringstream __ss;
        char __char = '\0';
        while(read(__sock[1], &__char, 1) > 0)
        {
            __ss << __char;
            if(__char == '\0')
            {
                break;
            }
        }
        vector<string> __str_vec;
        string __data(__ss.str());
        int start(0);

        for(unsigned long i = 0; i < __data.length(); ++i)
        {
            if(__data[i] == ' ')
            {
                string __sub_str(__data.substr(start, i - start));
                __str_vec.push_back(__sub_str);
                start = i + 1;
            }

            if(i == __data.length() - 1)
            {
                string __sub_str(__data.substr(start, i - start));
                __str_vec.push_back(__sub_str);
            }
        }

        __str_vec = combineArgsBetweenQuotes(__str_vec);

        // look for commands.
        for(unsigned long i = 0; i < __str_vec.size(); ++i)
        {
            if(__str_vec[i] == "_DIR:")
            {
                string __msg__= fs::current_path().string();
                if(send(__sock[1], __msg__.c_str(), __msg__.length() + 1, 0) < 0)
                {
                    perror(string(__time__() + "send: ").c_str());
                }
            }
            else if(__str_vec[i] == "SIGTERM")
            {
                console->out("Recived: 'SIGTERM', shuting down now.\n");
                close(__sock[1]);
                close(__sock[0]);
                exit(EXIT_SUCCESS);
            }
            else if(__str_vec[i] == "server_time")
            {
                string __msg__("Current server time -> " + __time__() + "\n");
                if(send(__sock[1], __msg__.c_str(), __msg__.length(), 0) < 0)
                {
                    perror(string(__time__() + "send: ").c_str());
                }
            }
            else if(__str_vec[i] == "run")
            {
                std::vector<char*> argv;
                for(unsigned long h = i + 1; h < __str_vec.size(); ++h)
                {
                    argv.push_back(const_cast<char*>(__str_vec[h].c_str()));
                }
                // Null-terminate the array
                argv.push_back(nullptr);

                pid_t pid = fork();
                if(pid == -1)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if(pid == 0) // Child process
                {
                    dup2(__sock[1], STDOUT_FILENO); // Redirect stdout to pipe
                    // dup2(__sock[1], STDIN_FILENO);
                    dup2(__sock[1], STDERR_FILENO); // Optional: Redirect stderr to pipe
                    execvp(__str_vec[i + 1].c_str(), argv.data());
                    
                    // If execl returns, it means it failed
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else if(__str_vec[i] == "run_backround")
            {
                string __args__;
                for(unsigned long u = (i + 1); u < __str_vec.size(); ++u)
                {
                    __args__ += __str_vec[u] + " ";
                }

                pid_t pid = fork();
                if(pid == -1)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if(pid == 0) // Child process
                {
                    auto __run__ = [&]()
                    {
                        execlp(__str_vec[i + 1].c_str(), __args__.c_str(), (char *)NULL);
                    };

                    thread t = thread(__run__);
                    
                    t.detach();
                    // perror("execl");
                    // exit(EXIT_FAILURE);
                }
                else {} // This is the parent process
                
                string __msg__("\n");
                if(send(__sock[1], __msg__.c_str(), __msg__.length(), 0) < 0)
                {
                    perror(string(__time__() + "send: ").c_str());
                }
            }
            else if(__str_vec[i] == "ls")
            {
                string __dir;
                if(i == __str_vec.size() - 1)
                {
                    __dir = fs::current_path().string();
                }
                else
                {
                    __dir = __str_vec[i + 1];
                }

                string __output__ = c_ls(__dir);
                if(send(__sock[1], __output__.c_str(), __output__.length(), 0) < 0)
                {
                    perror(string(__time__() + "send: ").c_str());
                }
            }
            else if(__str_vec[i] == "pipe")
            {
                string __args__;
                for(unsigned long u = (i + 1); u < __str_vec.size(); ++u)
                {
                    __args__ += __str_vec[i];
                }

                int pipefd[2]; // Create a pipe to capture the command's output
                if(pipe(pipefd) == -1)
                {
                    perror("pipe");
                }

                pid_t childPid = fork();
                if(childPid == -1)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if(childPid == 0) // Child process.
                {
                    close(pipefd[0]);
                    dup2(pipefd[1], STDOUT_FILENO);
                    dup2(pipefd[1], STDERR_FILENO);
                    const char* cmd = __str_vec[i + 1].c_str();

                    // Execute the command
                    execlp(cmd, cmd, nullptr);
                    
                    // If execlp fails, print an error message and exit
                    perror("execlp");
                    exit(EXIT_FAILURE);
                }
                else // Parent process.
                {
                    // close(pipefd[0]);                               // Close the read end of the pipe, as it's not needed in the parent
                    constexpr size_t bufferSize = 4096;             // Create a buffer to read the command's output
                    char buffer[bufferSize];
                    std::string output = "";
                    ssize_t bytesRead;
                    while((bytesRead = read(pipefd[1], buffer, bufferSize)) > 0) // Read the command's output and capture it into the 'output' string
                    { 
                        output += std::string(buffer, bytesRead);
                    }

                    close(pipefd[1]);                               // Close the write end of the pipe
                }
            }
            else if (__str_vec[i] == "cd")
            {
                try
                {
                    if (i == (__str_vec.size() - 1))
                    {
                        fs::current_path(string(getenv("HOME")) + "/.server" );
                    }
                    else
                    {
                        fs::current_path(__str_vec[i + 1].c_str());
                    }
                }
                catch(exception &__e)
                {
                    cout << __e.what() << "\n";
                }
            }
        }

        close(__sock[1]);
        console->out(__time__(), __data, '\n');
    }

    return 0;
}

