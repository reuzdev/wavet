#include "arguments.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include "stb_image.h"
#include "image.hpp"
#include "animation.hpp"
#include "config.hpp"
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <limits.h>
#endif

ArgParser::ArgParser(int argc, const char** argv)
    : m_idx(1), m_argc(argc), m_argv(argv), m_shouldExitSuccess(false)
    , m_shouldExitFail(false), m_wasCustomWaveAdded(false) {
    setDefaults();
    setAssetsDir();
    parseAll();
    if (m_shouldExitFail || m_shouldExitSuccess) {
        return;
    }
    checkRequiredFields();
}

AppConfig ArgParser::getAppConfig() {
    return m_conf;
}

bool ArgParser::getShouldExitSuccess() {
    return m_shouldExitSuccess;
}

bool ArgParser::getShouldExitFail() {
    return m_shouldExitFail;
}

const char* ArgParser::expectArg() {
    if (m_idx < m_argc) {
        return m_argv[m_idx++];
    }
    else {
        m_shouldExitFail = true;
        std::cout << "ERROR: Expected (more) argument(s) after " << m_label << "\n";
        return nullptr;
    }
}

// TODO: Make sure the check is enough to know it is a valid float
bool ArgParser::expectFloat(float* outVal) {
    const char* arg = expectArg();
    if (arg == nullptr) {
        return false;
    }

    char* c = const_cast<char*>(arg);
    if (*c == '-') {
        c++;
    }

    bool hasFailed = false;
    bool foundDot = false;
    for (; *c != '\0'; c++) {
        if (*c == '.') {
            if (foundDot) {
                hasFailed = true;
                break;
            }
            foundDot = true;
        }
        else if (!isdigit(*c)) {
            hasFailed = true;
            break;
        }
    }

    if (hasFailed) {
        m_shouldExitFail = true;
        std::cout << "ERROR: Expected valid float after `" << m_label << "`, got `" << arg << "`\n";
        return false;
    }

    *outVal = std::stof(arg);
    return true;
}

// TODO: Make sure the check is enough to know it is a valid int
bool ArgParser::expectInt(int* outVal) {
    const char* arg = expectArg();
    if (arg == nullptr) {
        return false;
    }

    char* c = const_cast<char*>(arg);
    if (*c == '-') {
        c++;
    }

    for (; *c != '\0'; c++) {
        if (!isdigit(*c)) {
            m_shouldExitFail = true;
            std::cout << "ERROR: Expected valid integer after `"
                << m_label << "`, got `" << arg << "`\n";
            return false;
        }
    }

    *outVal = std::stoi(arg);
    return true;
}

bool ArgParser::expectColor(Color* outVal) {
    static const size_t valCount = 3;
    uint8_t values[valCount];

    for (size_t i = 0; i < valCount; i++) {
        int value;
        if (!expectInt(&value)) {
            return false;
        }
        values[i] = static_cast<uint8_t>(value);
    }

    *outVal = Color(values[0], values[1], values[2]);
    return true;
}

void ArgParser::printHelp() {
    static const char msg[] =
        "wavet is a terminal app for playing wave animation for pixel art flags "
        "made by reuzdev in Izmir, Turkiye\n\n"
        "USAGE: wavet [options...]\n"
        "OPTIONS:\n"
        "  --help, -h, -?                      Print this\n"
        "  --flag, -f {name or path}           Flag to wave\n"
        "  --list, -l                          List available flag names\n"
        "  --float, -F                         Do not fix the left side of the flag\n"
        "  --ambient, -a {0 to 1 (e.g 0.5)}    Set ambient light\n"
        "  --background, -b {r} {g} {b}        Set background color\n"
        "  --speed, -s {scale}                 Speed multiplier\n"
        "  --wave, -w {ampl.} {wavelen.} {phase} {speed}\n"
        "                                      Add wave. Can use multiple times.\n"
        "  --simple, -S                        Draw flag only\n"
        "  --center, -c                        Center flag (and pole)\n"
        "  --message, -m {text}                Print a message. Overrides -S and -c\n"
        "  --text-color, -t {r} {g} {b}        Set text color for message\n"
    ;
    std::cout << msg;
}

void ArgParser::setDefaults() {
    WaveConfig waveConfig;
    waveConfig.waves = {
        SineWave(2, 37, 60),
        SineWave(1, 49, 72),
        SineWave(1, 93, 30)
    };
    waveConfig.speedMultiplier = 1;
    waveConfig.keepLeftFixed = true;
    waveConfig.baseSize = 16;

    m_conf.flag = Image();
    m_conf.ambientLight = 0.1f;
    m_conf.bg = Color();
    m_conf.fancyScene = true;
    m_conf.centeredScene = false;
    m_conf.msg = std::string();
    m_conf.waveConfig = waveConfig;
    m_conf.textColor = Color(255, 255, 255);
}

void ArgParser::setAssetsDir() {
#ifdef _WIN32
    char pathBuff[MAX_PATH];
    GetModuleFileNameA(NULL, pathBuff, sizeof(pathBuff));
#else
    char pathBuff[PATH_MAX];
    size_t pathLen = readlink("/proc/self/exe", pathBuff, sizeof(pathBuff) - 1);
    if (pathLen == 0) {
        pathBuff[0] = '\0';
    }
    else {
        pathBuff[pathLen] = '\0';
    }
#endif

    std::filesystem::path assetsDir = std::filesystem::path(pathBuff).parent_path() / "assets";
    if (std::filesystem::exists(assetsDir)) {
        m_conf.assetsDir = assetsDir.string();
        return;
    }

#ifndef _WIN32
    assetsDir = std::filesystem::path(INSTALLED_ASSET_PATH) / "assets";
    if (std::filesystem::exists(assetsDir)) {
        m_conf.assetsDir = assetsDir.string();
        return;
    }
#endif

    m_conf.assetsDir = std::filesystem::current_path().string();
}

void ArgParser::checkRequiredFields() {
    if (m_conf.flag.getHeight() == 0) {
        std::cout << "ERROR: A flag must be provided with --flag"
            " <name or file path>. See available flags with --list\n";
        m_shouldExitFail = true;
    }
}

void ArgParser::parseAll() {
    while (m_idx < m_argc && !m_shouldExitFail && !m_shouldExitSuccess) {
        m_label = std::string(expectArg());

        if (m_label == "--help" || m_label == "-h" || m_label == "-?") {
            printHelp();
            m_shouldExitSuccess = true;
        }
        else if (m_label == "--flag" || m_label == "-f") {
            handleFlag();
        }
        else if (m_label == "--list" || m_label == "-l") {
            handleList();
        }
        else if (m_label == "--float" || m_label == "-F") {
            m_conf.waveConfig.keepLeftFixed = false;
        }
        else if (m_label == "--simple" || m_label == "-S") {
            m_conf.fancyScene = false;
        }
        else if (m_label == "--center" || m_label == "-c") {
            m_conf.centeredScene = true;
        }
        else if (m_label == "--ambient" || m_label == "-a") {
            handleAmbient();
        }
        else if (m_label == "--background" || m_label == "-b") {
            handleBackground();
        }
        else if (m_label == "--speed" || m_label == "-s") {
            handleSpeed();
        }
        else if (m_label == "--wave" || m_label == "-w") {
            handleWave();
        }
        else if (m_label == "--message" || m_label == "-m") {
            handleMessage();
        }
        else if (m_label == "--text-color" || m_label == "-t") {
            handleTextColor();
        }
        else {
            std::cout << "ERROR: Unexpected token " << m_label << "\n";
            m_shouldExitFail = true;
            return;
        }
    }
}

void ArgParser::handleFlag() {
    const char* arg = expectArg();
    if (arg == nullptr) {
        return;
    }

    std::string path = arg;
    if (!std::filesystem::exists(path)) {
        path = m_conf.assetsDir + "/" + path + ".png";
    }
    if (!std::filesystem::exists(path)) {
        std::string fileName = std::string(arg) + ".png";
        for (const auto& entry : std::filesystem::recursive_directory_iterator(m_conf.assetsDir)) {
            if (entry.path().filename() == fileName) {
                path = entry.path().string();
                break;
            }
        }
    }
    if (!std::filesystem::exists(path)) {
        std::cout << "ERROR: Couldn't find file `" << arg << "`\n";
        m_shouldExitFail = true;
        return;
    }

    int width, height;
    int origChannels;
    uint8_t* stbiBuffer = stbi_load(
        path.c_str(), &width, &height, &origChannels, IMG_BUFFER_CHANNELS
    );
    if (stbiBuffer == nullptr) {
        std::cout << "ERROR: Couldn't load image `" << path << "`\n";
        m_shouldExitFail = true;
        return;
    }

    m_conf.flag = Image(stbiBuffer, width, height);
}

void ArgParser::handleList() {
    for (const auto& entry : std::filesystem::directory_iterator(m_conf.assetsDir)) {
        if (!entry.is_directory()) {
            continue;
        }
        std::cout << entry.path().stem().string() << "/\n";
        for (const auto& categoryDir : std::filesystem::directory_iterator(entry)) {
            if (!categoryDir.is_directory()) {
                continue;
            }
            std::cout << "  " << categoryDir.path().stem().string() << "/\n";
            for (const auto& flagFile : std::filesystem::directory_iterator(categoryDir)) {
                if (flagFile.path().extension() == ".png") {
                    std::cout << "      * " << flagFile.path().stem().string() << "\n";
                }
            }
        }
    }
    m_shouldExitSuccess = true;
}

void ArgParser::handleAmbient() {
    float value;
    if (!expectFloat(&value)) {
        return;
    }
    if (value < 0 || value > 1) {
        std::cout << "ERROR: Ambient light value (`" << value << "` after `"
            << m_label << "`) should be a value from 0 to 1\n";
        m_shouldExitFail = true;
        return;
    }
    m_conf.ambientLight = value;
}

void ArgParser::handleBackground() {
    Color color;
    if (!expectColor(&color)) {
        return;
    }

    m_conf.bg = color;
}

void ArgParser::handleTextColor() {
    Color color;
    if (!expectColor(&color)) {
        return;
    }

    m_conf.textColor = color;
}

void ArgParser::handleSpeed() {
    float value;
    if (!expectFloat(&value)) {
        return;
    }

    m_conf.waveConfig.speedMultiplier = value;
}

void ArgParser::handleWave() {
    static const size_t valCount = 4;
    float values[valCount];

    if (!m_wasCustomWaveAdded) {
        m_conf.waveConfig.waves.clear();
        m_wasCustomWaveAdded = true;
    }

    for (size_t i = 0; i < valCount; i++) {
        float value;
        if (!expectFloat(&value)) {
            return;
        }
        values[i] = value;
    }

    m_conf.waveConfig.waves.emplace_back(SineWave(values[0], values[1], values[2], values[3]));
}

void ArgParser::handleMessage() {
    const char* valueArg = expectArg();
    if (valueArg == nullptr) {
        return;
    }

    m_conf.msg = std::string(valueArg);
}
