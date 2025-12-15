#pragma once
#include <iostream>
#include <filesystem>
#include <string>
#include "image.hpp"
#include "animation.hpp"

struct AppConfig {
    std::string assetsDir;
    Image flag;
    float ambientLight;
    Color bg;
    Color textColor;
    WaveConfig waveConfig;
    bool fancyScene;
    std::pair<float, float> normalPos;
    std::string msg;
};

class ArgParser {
public:
    ArgParser(int argc, const char** argv);
    AppConfig getAppConfig();
    bool getShouldExitSuccess();
    bool getShouldExitFail();
private:
    size_t m_idx;
    size_t m_argc;
    const char** m_argv;
    AppConfig m_conf;
    std::string m_label;
    bool m_shouldExitSuccess;
    bool m_shouldExitFail;
    bool m_wasCustomWaveAdded;

    void setAssetsDir();
    void setDefaults();
    void checkRequiredFields();
    const char* expectArg();
    bool expectFloat(float* outVal);
    bool expectInt(int* outVal);
    bool expectColor(Color* outVal);
    void printHelp();
    void parseAll();
    void handleFlag();
    void handleList();
    void handleWave();
};
