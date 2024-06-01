set(HEXE_TERMINAL_HEADERS ${HEXE_TERMINAL_HEADERS}
    "Hexe/System/Process.h"
    "Hexe/Terminal/Boxdraw.h"
    "Hexe/Terminal/PseudoTerminal.h"
    "Hexe/Terminal/TerminalDisplay.h"
    "Hexe/Terminal/TerminalEmulator.h"
    "Hexe/Terminal/Types.h"

    "src/boxdraw_data.h"
    "src/config.def.h"
    "src/ImGuiTerminal.colors.h"
    "src/ImGuiTerminal.keys.h"
    "src/nonspacing.h"
    "src/wide.h"
    "src/WindowsErrors.h"
)

set(HEXE_TERMINAL_SOURCES ${HEXE_TERMINAL_SOURCES}
    "src/AutoHandle.cpp"
    "src/Pipe.win32.cpp"
    "src/Process.cpp"
    "src/Process.win32.cpp"
    "src/ProcessFactory.cpp"
    "src/PseudoTerminal.cpp"
    "src/PseudoTerminal.win32.cpp"
    "src/TerminalDisplay.cpp"
    "src/TerminalEmulator.cpp"
)

add_library( TeminalEmulator
    ${HEXE_TERMINAL_HEADERS}
    ${HEXE_TERMINAL_SOURCES}
)