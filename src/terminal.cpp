#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <signal.h>
#endif
#include "terminal.hpp"

TerminalController& TerminalController::getInstance() {
    static TerminalController tc;
    return tc;
}

TerminalController::TerminalController()
    : m_isCtrlCPressed(false) {
    setupTerminal();
#ifdef _WIN32
    SetConsoleCtrlHandler(handleCtrlC_Windows, TRUE);
#else
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = handleCtrlC_Linux;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
#endif
}

// TODO: Do i need to reset ctrl handler?
TerminalController::~TerminalController() {
    cleanupTerminal();
#ifdef _WIN32
    SetConsoleCtrlHandler(NULL, FALSE);
#endif
}

// TODO: Switch windows terminal to UTF-8
void TerminalController::setupTerminal() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleMode(hOut, &m_origOutMode) == 0) {
        // TODO: Handle the error
    }

    DWORD targetMode = m_origOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (SetConsoleMode(hOut, targetMode)) {
        // TODO: Handle the error
    }
#endif
    std::cout << ANSI_ENTER_ALT_BUFFER << ANSI_HIDE_CURSOR << std::flush;
}

void TerminalController::cleanupTerminal() {
    std::cout << ANSI_EXIT_ALT_BUFFER << ANSI_SHOW_CURSOR << std::flush;
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(hOut, m_origOutMode);
#endif
}

#ifdef _WIN32
int TerminalController::handleCtrlC_Windows(DWORD ctrlType) {
    if (ctrlType == CTRL_C_EVENT) {
        TerminalController::getInstance().m_isCtrlCPressed = true;
        return true;
    }
    return false;
}
#else
void TerminalController::handleCtrlC_Linux(int signal) {
    // TODO: As far as i understood, we only set this callback for SIGINT, but
    // it still gives the signal as an argument so maybe that is not true. Check
    // this and remove the assert and check if signal is SIGINT if that is not
    // the case
    (void)signal;
    assert(signal == SIGINT);
    TerminalController::getInstance().m_isCtrlCPressed = true;
}
#endif

std::ostringstream& TerminalController::getStream() {
    return m_outStream;
}

void TerminalController::flush() {
    std::cout << m_outStream.str();
    m_outStream.str("");
    m_outStream.clear();
}

bool TerminalController::shouldExit() {
    return m_isCtrlCPressed;
}

std::pair<int, int> TerminalController::getSize() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    if (GetConsoleScreenBufferInfo(hOut, &bufferInfo)) {
        // TODO: Handle the error
    }
    return std::pair<int, int>(bufferInfo.dwSize.X, bufferInfo.dwSize.Y);
#else
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    return std::pair<int, int>(ws.ws_col, ws.ws_row);
#endif
}

void TerminalController::setCursor(int x, int y) {
    m_outStream << CSI << y << ";" << x << "H";
}

void TerminalController::setCursorHome() {
    m_outStream << CSI "H";
}

void TerminalController::clearScreen() {
    m_outStream << CSI "2J" CSI "H";
}

void TerminalController::setFG(Color col) {
    m_outStream << CSI "38;2;" << static_cast<int>(col.r) << ";"
                               << static_cast<int>(col.g) << ";"
                               << static_cast<int>(col.b) << "m";
}

void TerminalController::setBG(Color col) {
    m_outStream << CSI "48;2;" << static_cast<int>(col.r) << ";"
                               << static_cast<int>(col.g) << ";"
                               << static_cast<int>(col.b) << "m";
}

void TerminalController::setFGAndBG(Color fg, Color bg) {
    m_outStream << CSI "38;2;" << static_cast<int>(fg.r) << ";"
                               << static_cast<int>(fg.g) << ";"
                               << static_cast<int>(fg.b) << "m";
    m_outStream << CSI "48;2;" << static_cast<int>(bg.r) << ";"
                               << static_cast<int>(bg.g) << ";"
                               << static_cast<int>(bg.b) << "m";
}

void TerminalController::resetFG() {
    m_outStream << CSI "39m";
}

void TerminalController::resetBG() {
    m_outStream << CSI "49m";
}

void TerminalController::resetFGAndBG() {
    m_outStream << CSI "39m" CSI "49m";
}

void TerminalController::assignPreferredFGandBG(Color fg, Color bg) {
    m_prefFG = fg;
    m_prefBG = bg;
}

void TerminalController::usePreferredFGandBG() {
    if (m_prefFG == Color()) {
        resetFG();
    }
    else {
        setFG(m_prefFG);
    }

    if (m_prefBG == Color()) {
        resetBG();
    }
    else {
        setBG(m_prefBG);
    }
}
