#include "WindowManager/WindowManager.hpp"
#include <iostream>
#if defined(__WIN32) || defined(_WIN64)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    if(!Initialize_Window())
        return 1;
    Execute_Window();
    Clean_Window();
    return 0;
}
#else
int main(int, char**)
{
    if(!Initialize_Window())
        return 1;
    Execute_Window();
    Clean_Window();

    return 0;
}
#endif