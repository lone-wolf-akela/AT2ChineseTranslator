#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "console.h"

namespace console
{
  void clear() noexcept
  {
    COORD topLeft = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written = 0;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacter(
      console, L' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    FillConsoleOutputAttribute(
      console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
      screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    SetConsoleCursorPosition(console, topLeft);
  }
}
