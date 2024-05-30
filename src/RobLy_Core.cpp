#include "WindowManager/WindowManager.hpp"

int main(int, char**)
{
    if(!Initialize_Window())
        return 1;
    Execute_Window();
    Clean_Window();

    return 0;
}