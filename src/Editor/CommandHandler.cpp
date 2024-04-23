#include "CommandHandler.hpp"

std::string ExecuteCommand(const std::string &command, const std::string &current_path)
{
    std::string result;
#ifdef _WIN32
    //Windows specific code
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE hRead, hWrite;
    if (CreatePipe(&hRead, &hWrite, &sa, 0)) {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        GetStartupInfo(&si);
        si.hStdError = hWrite;
        si.hStdOutput = hWrite;
        si.dwFlags |= STARTF_USESTDHANDLES;

        // Convert narrow string to wide string
        std::wstring wCommand(command.begin(), command.end());
        std::wstring wPath(current_path.begin(), current_path.end());

        if (CreateProcess(nullptr, 
                          const_cast<LPWSTR>(wCommand.c_str()), 
                          nullptr, 
                          nullptr, 
                          TRUE,
                          CREATE_NO_WINDOW, 
                          nullptr, 
                          const_cast<LPWSTR>(wPath.c_str()), 
                          &si, 
                          &pi)) 
        {
            CloseHandle(hWrite);
            WaitForSingleObject(pi.hProcess, INFINITE);

            DWORD bytesRead;
            char buffer[128];
            while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) != 0) {
                if (bytesRead == 0)
                    break;

                buffer[bytesRead] = '\0';
                result += buffer;
            }

            CloseHandle(hRead);
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        } else {
            result = "Failed to execute command";
        }
    } else {
        result = "Failed to create pipe";
    }
#else
    std::filesystem::current_path(current_path); //change the current working directory
    // Unix-like system code
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }

        pclose(pipe);
    } else {
        result = "Failed to execute command";
    }
#endif

    return result;
}
