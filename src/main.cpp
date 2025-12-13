#include "terminal.hpp"
#include "animation.hpp"
#include "arguments.hpp"
#define STB_IMAGE_IMPLEMENTATION
    #include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #define Sleep(x) usleep(x * 1000)
#endif

int main(int argc, const char** argv) {
    ArgParser argParser(argc, argv);

    if (argParser.getShouldExitFail()) {
        return -1;
    }
    if (argParser.getShouldExitSuccess()) {
        return 0;
    }

    AppConfig conf = argParser.getAppConfig();
    TerminalController& term = TerminalController::getInstance();
    Canvas& canvas = Canvas::getInstance();

    term.assignPreferredFGandBG(conf.textColor, conf.bg);
    term.clearScreen();
    float t = 0;
    static const int fps = 24;
    while (!term.shouldExit()) {
        canvas.beginDrawing(conf.bg);
        if (!conf.msg.empty()) {
            canvas.drawSceneFlagPoleAndMsg(conf.flag, conf.waveConfig, conf.ambientLight, conf.msg, t);
        }
        else if (conf.fancyScene) {
            static float hPos = conf.centeredScene ? 0.5f : 0.34f;
            static float vPos = conf.centeredScene ? 0.5f : 0.34f;
            canvas.drawSceneFlagAndPole(conf.flag, conf.waveConfig, hPos, vPos, conf.ambientLight, t);
        }
        else {
            canvas.drawSceneFlagOnly(conf.flag, conf.waveConfig, conf.centeredScene, conf.ambientLight, t);
        }
        canvas.endDrawing();
        Sleep(1000/fps);
        t += 1.0f/fps;
    }

    term.resetFGAndBG();
    term.getStream() << "\n";
    term.flush();

    return 0;
}
