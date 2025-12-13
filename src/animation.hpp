#pragma once
#include <utility>
#include "terminal.hpp"
#include "image.hpp"

#define PI 3.14159265358979323846
#define ROWS_PER_CHAR 2

#ifdef _WIN32
    #define FULL_BLOCK_CHAR  static_cast<char>(219)
    #define TOP_HALF_CHAR    static_cast<char>(223)
    #define BOTTOM_HALF_CHAR static_cast<char>(220)
#else
    #define FULL_BLOCK_CHAR  "\u2588"
    #define TOP_HALF_CHAR    "\u2580"
    #define BOTTOM_HALF_CHAR "\u2584"
#endif

struct SineWave {
    float amplitude;
    float wavelength;
    float speed;
    float phase;

    SineWave(float amplitude, float wavelength, float speed = 1, float phase = 0);
};

struct WaveConfig {
    std::vector<SineWave> waves;
    float speedMultiplier;
    size_t baseSize;
    bool keepLeftFixed;

    int getTotalAmpl() const;
};

class Canvas {
public:
    Canvas(Canvas& other) = delete;
    void operator=(const Canvas&) = delete;
    static Canvas& getInstance();

    void beginDrawing(Color bg = Color());
    void endDrawing();
    void drawRect(std::pair<int, int> origin, std::pair<size_t, size_t>, Color fill);
    void drawWavedImage(
        const Image& img,
        std::pair<int, int> origin,
        const WaveConfig& waveConfig,
        float ambientLight,
        float time
    );
    void drawSceneFlagOnly(
        const Image& img,
        const WaveConfig& waveConfig,
        bool centered,
        float ambientLight,
        float time
    );
    void drawSceneFlagAndPole(
        const Image& img,
        const WaveConfig& waveConfig,
        float hPosNormal,
        float vPosNormal,
        float ambientLight,
        float time
    );
    void drawSceneFlagPoleAndMsg(
        const Image& img,
        const WaveConfig& waveConfg,
        float ambientLight,
        const std::string& msg,
        float time
    );
    void outputPixelPair(std::pair<size_t, size_t> topPixel);
private:
    Image m_prevCanvas;
    Image m_currCanvas;
    TerminalController& m_term;

    Canvas();
    ~Canvas() = default;
};
