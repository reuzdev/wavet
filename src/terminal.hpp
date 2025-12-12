#pragma once
#include <string>
#include <sstream>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif
#include <utility>
#include "image.hpp"
#include "stb_image.h"

#define ESC "\x1b"
#define CSI "\x1b["
#define OSC "\x1b]"

class TerminalController {
public:
    TerminalController(TerminalController& other) = delete;
    void operator=(const TerminalController&) = delete;
    static TerminalController& getInstance();

    std::ostringstream& getStream();
    void flush();
    bool shouldExit();

    std::pair<int, int> getSize();
    void setCursor(int x, int y);
    void setCursorHome();
    void clearScreen();
    void setFG(Color c);
    void setBG(Color c);
    void setFGAndBG(Color fg, Color bg);
    void resetFG();
    void resetBG();
    void resetFGAndBG();
    void assignPreferredFGandBG(Color fg, Color bg);
    void usePreferredFGandBG();
private:
    std::ostringstream m_outStream;
    bool m_isCtrlCPressed;
    Color m_prefFG;
    Color m_prefBG;

    TerminalController();
    ~TerminalController();
    void setupTerminal();
    void cleanupTerminal();
#ifdef _WIN32
    DWORD m_origOutMode, m_origInMode;
    std::string m_origTitle;

    static int handleCtrlC_Windows(DWORD ctrlType);
#else
    static void handleCtrlC_Linux(int signal);
#endif
};
