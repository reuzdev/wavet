#include "animation.hpp"
#include <cmath>
#include <utility>
#include "image.hpp"
#include "terminal.hpp"

SineWave::SineWave(float amplitude, float wavelength, float speed, float phase)
    : amplitude(amplitude), wavelength(wavelength), speed(speed), phase(phase) {}

int WaveConfig::getTotalAmpl() const {
    float totalAmpl = 0;
    for (size_t i = 0; i < waves.size(); i++) {
        totalAmpl += waves.at(i).amplitude;
    }
    return static_cast<int>(ceil(totalAmpl));
}

Canvas& Canvas::getInstance() {
    static Canvas canvas = Canvas();
    return canvas;
}

Canvas::Canvas()
    : m_prevCanvas(0, 0), m_currCanvas(0, 0), m_term(TerminalController::getInstance()) {
    std::pair<int, int> termSize = m_term.getSize();
    m_prevCanvas.resize(termSize.first, termSize.second * ROWS_PER_CHAR);
    m_currCanvas.resize(termSize.first, termSize.second * ROWS_PER_CHAR);
}

void Canvas::beginDrawing(Color bg) {
    m_prevCanvas = m_currCanvas;
    std::pair<int, int> termSize = m_term.getSize();
    m_currCanvas.resize(termSize.first, termSize.second * ROWS_PER_CHAR);
    m_currCanvas.clear(bg);
    m_term.setCursorHome();
}

void Canvas::endDrawing() {
    if (m_prevCanvas.getSize() != m_currCanvas.getSize()) {
        m_term.clearScreen();
        m_term.setCursorHome();
        for (size_t y = 0; y < m_currCanvas.getHeight(); y += 2) {
            m_term.setCursor(1, static_cast<int>(y)/2 + 1);
            for (size_t x = 0; x < m_currCanvas.getWidth(); x++) {
                outputPixelPair(std::pair<size_t, size_t>(x, y));
            }
        }
    }
    else {
        for (size_t y = 0; y < m_currCanvas.getHeight(); y += 2) {
            for (size_t x = 0; x < m_currCanvas.getWidth(); x++) {
                bool topsEqual = m_currCanvas.getPixel(x, y) == m_prevCanvas.getPixel(x, y);
                bool bottomsEqual = y + 1 >= m_currCanvas.getHeight()
                    || (m_currCanvas.getPixel(x, y + 1) == m_prevCanvas.getPixel(x, y + 1));
                if (!topsEqual || !bottomsEqual) {
                    m_term.setCursor(static_cast<int>(x) + 1, static_cast<int>(y)/2 + 1);
                    outputPixelPair(std::pair<size_t, size_t>(x, y));
                }
            }
        }
    }
    m_term.flush();
}

void Canvas::drawRect(std::pair<int, int> origin, std::pair<size_t, size_t> size, Color fill) {
    for (size_t yOff = 0; yOff < size.second; yOff++) {
        for (size_t xOff = 0; xOff < size.first; xOff++) {
            int targetX = origin.first + static_cast<int>(xOff);
            int targetY = origin.second + static_cast<int>(yOff);
            if (static_cast<int>(m_currCanvas.getHeight()) > targetY && targetY >= 0
                && static_cast<int>(m_currCanvas.getWidth()) > targetX && targetX >= 0) {
                m_currCanvas.setPixel(origin.first + xOff, origin.second + yOff, fill);
            }
        }
    }
}

// TODO: Make it so parameters of the sinewave are scaled according to the base
// size. For example if base size is 16 and flag is 32 pixels wide, then
// everything should be twice as large. Also make base size 2D.
void Canvas::drawWavedImage(
    const Image& img,
    std::pair<int, int> origin,
    const WaveConfig& waveConfig,
    float ambientLight,
    float time
) {
    int yPadding = waveConfig.getTotalAmpl();

    float yShiftPrev = 0;
    float yShiftCurr = 0;
    float yShiftNext = 0;
    float yShiftSecNext = 0;

    // NOTE: Start from -3 so that when we get to 0, necessary values of yShifts are ready
    for (int x = -3; x < static_cast<int>(img.getWidth()); x++) {
        yShiftPrev = yShiftCurr;
        yShiftCurr = yShiftNext;
        yShiftNext = yShiftSecNext;
        yShiftSecNext = 0;

        for (size_t i = 0; i < waveConfig.waves.size(); i++) {
            const SineWave& w = waveConfig.waves.at(i);
            float secNextAngle = 2 * static_cast<float>(PI)
                * (x + 2 - time * waveConfig.speedMultiplier * w.speed) / w.wavelength + w.phase;
            yShiftSecNext += w.amplitude * sin(secNextAngle);
        }

        if (waveConfig.keepLeftFixed) {
            yShiftSecNext *= static_cast<float>(x + 2) / img.getWidth() * (x + 2 < 0 ? -1 : 1);
        }

        if (x < 0) {
            continue;
        }

        float prevSlope = (yShiftCurr - yShiftPrev) / 2;
        float currSlope = (yShiftNext - yShiftCurr) / 2;
        float nextSlope = (yShiftSecNext - yShiftNext) / 2;
        float slope = 0.15f * prevSlope + 0.7f * currSlope + 0.15f * nextSlope;
        int yStart = yPadding + static_cast<int>(round(yShiftCurr));
        float tangentLen = sqrt(1 + slope*slope);
        float normalX = -1 * slope / tangentLen;
        float normalY = 1 / tangentLen;
        float lightX = 1 / sqrt(2.0f);
        float lightY = -1 / sqrt(2.0f);
        float lightLevel = fmax(ambientLight, normalX * -lightX + normalY * -lightY);

        for (size_t y = 0; y < img.getHeight(); y++) {
            Color color = img.getPixel(x, y) * lightLevel;
            int targetX = origin.first + x;
            int targetY = origin.second + yStart + static_cast<int>(y);
            if (static_cast<int>(m_currCanvas.getHeight()) > targetY && targetY >= 0
                && static_cast<int>(m_currCanvas.getWidth()) > targetX && targetX >= 0) {
                m_currCanvas.setPixel(origin.first + x, origin.second + yStart + y, color);
            }
        }
    }
}

void Canvas::drawSceneFlagOnly(
    const Image& img,
    const WaveConfig& waveConfig,
    bool centered,
    float ambientLight,
    float time
) {
    std::pair<int, int> origin(0, 0);
    if (centered) {
        origin.first = static_cast<int>(m_currCanvas.getWidth()/2 - img.getWidth()/2);
        origin.second = static_cast<int>(
            m_currCanvas.getHeight()/2 - img.getHeight()/2 - waveConfig.getTotalAmpl()
        );
    }
    drawWavedImage(img, origin, waveConfig, ambientLight, time);
}

void Canvas::drawSceneFlagAndPole(
    const Image& img,
    const WaveConfig& WaveConfig,
    float hPosNormal,
    float vPosNormal,
    float ambientLight,
    float time
) {
    std::pair<int, int> origin(
        static_cast<int>(m_currCanvas.getWidth() * hPosNormal - img.getWidth()/2),
        static_cast<int>(
            m_currCanvas.getHeight() * vPosNormal - img.getHeight()/2 - WaveConfig.getTotalAmpl()
        )
    );
    drawWavedImage(img, origin, WaveConfig, ambientLight, time);
    drawRect(
        std::pair<int, int>(origin.first - 1, origin.second),
        std::pair<int, int>(1, static_cast<int>(m_currCanvas.getHeight()) - origin.second),
        Color(240, 240, 240)
    );
    drawRect(
        std::pair<int, int>(origin.first - 2, origin.second),
        std::pair<int, int>(1, static_cast<int>(m_currCanvas.getHeight()) - origin.second),
        Color(220, 220, 220)
    );
}

void Canvas::drawSceneFlagPoleAndMsg(
    const Image& img,
    const WaveConfig& waveConfig,
    float ambientLight,
    const std::string& msg,
    float time
) {
    size_t maxLineLen = m_term.getSize().first / 3 * 2 - 2;
    std::vector<std::pair<size_t, size_t>> lnBounds;

    size_t lineStart = 0;
    while (true) {
        bool lineExists = false;
        for (size_t i = lineStart; i < msg.size(); i++) {
            if (!isspace(msg.at(i))) {
                lineExists = true;
                lineStart = i;
                break;
            }
        }
        if (!lineExists) {
            break;
        }

        size_t lineEnd = lineStart + maxLineLen;
        if (lineEnd <= msg.size()) {
            for (int i = static_cast<int>(lineEnd) - 1; i >= static_cast<int>(lineStart); i--) {
                if (isspace(msg.at(i))) {
                    lineEnd = i;
                    break;
                }
            }
        }
        else {
            lineEnd = msg.size();
        }

        lnBounds.emplace_back(std::pair<size_t, size_t>(lineStart, lineEnd));
        lineStart = lineEnd;
    }

    drawSceneFlagAndPole(img, waveConfig, 0.34f, 0.34f, ambientLight, time);
    m_term.usePreferredFGandBG();

    std::pair<size_t, size_t> termSize = m_term.getSize();
    int textOriginY = static_cast<int>(termSize.second / 3 * 2 - lnBounds.size() / 2 + 1);
    if (textOriginY + lnBounds.size() > termSize.second) {
        textOriginY = static_cast<int>(termSize.second - lnBounds.size() - 1);
    }
    if (textOriginY < 1) {
        textOriginY = 1;
    }
    int textCenterX = static_cast<int>(termSize.first / 3 * 2 + 1);

    for (size_t i = 0; i < lnBounds.size(); i++) {
        size_t lineLen = lnBounds.at(i).second - lnBounds.at(i).first;
        int lineXCursor = textCenterX - static_cast<int>(lineLen) / 2;
        int lineYCursor = textOriginY + static_cast<int>(i);
        m_term.setCursor(lineXCursor, lineYCursor);
        m_term.getStream() << msg.substr(lnBounds.at(i).first, lineLen);
    }
}

void Canvas::outputPixelPair(std::pair<size_t, size_t> topPixel) {
    Color topColor = m_currCanvas.getPixel(topPixel.first, topPixel.second);
    Color bottomColor = Color();
    if (m_currCanvas.getHeight() > topPixel.second + 1) {
        bottomColor = m_currCanvas.getPixel(topPixel.first, topPixel.second + 1);
    }

    if (topColor.a && bottomColor.a) {
        m_term.setFGAndBG(topColor, bottomColor);
        m_term.getStream() << TOP_HALF_CHAR;
    }
    else if (topColor.a) {
        m_term.resetBG();
        m_term.setFG(topColor);
        m_term.getStream() << TOP_HALF_CHAR;
    }
    else if (bottomColor.a) {
        m_term.resetBG();
        m_term.setFG(bottomColor);
        m_term.getStream() << BOTTOM_HALF_CHAR;
    }
    else {
        m_term.resetBG();
        m_term.getStream() << " ";
    }
}
