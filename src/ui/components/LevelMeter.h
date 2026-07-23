// Rowen level meter — slim vertical peak meter in the site's language: a
// recessed hairline track, white level fill that turns magenta near the top,
// red only at clip, plus a latching clip indicator. Reads a std::atomic<float>
// published by the audio thread; all drawing happens on a 30 Hz UI timer.
// Click the meter to clear a latched clip light.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../theme/RowenTheme.h"
#include "../theme/RowenTypography.h"

namespace rowen::ui
{

class LevelMeter : public juce::Component,
                   public juce::SettableTooltipClient,
                   private juce::Timer
{
public:
    LevelMeter (std::atomic<float>& peakSource, const juce::String& caption)
        : source (peakSource), label (caption)
    {
        startTimerHz (30);
        setTooltip ("Shows your level. The red light means the signal clipped - click it to reset.");
    }

    void mouseDown (const juce::MouseEvent&) override { clipLatched = false; }

    void paint (juce::Graphics& g) override
    {
        namespace c = rowen::theme::color;
        auto area = getLocalBounds().toFloat();

        // Caption (bottom).
        const float capH = juce::jmax (12.0f, area.getHeight() * 0.07f);
        const auto capArea = area.removeFromBottom (capH);
        g.setColour (c::dimmer);
        g.setFont (rowen::theme::Typography::get().heading (capH * 0.85f));
        g.drawText (label, capArea, juce::Justification::centred);

        // Clip LED (top).
        const float ledH = juce::jmax (6.0f, area.getHeight() * 0.035f);
        auto ledArea = area.removeFromTop (ledH + 4.0f).withTrimmedBottom (4.0f);
        const float ledD = juce::jmin (ledArea.getWidth() * 0.6f, ledArea.getHeight());
        const auto led = ledArea.withSizeKeepingCentre (ledD, ledD);
        if (clipLatched)
        {
            g.setColour (c::bad);
            g.fillEllipse (led);
        }
        else
        {
            g.setColour (c::line);
            g.drawEllipse (led.reduced (0.5f), 1.0f);
        }
        area.removeFromTop (2.0f);

        // Track.
        const float trackW = juce::jmin (8.0f, area.getWidth() * 0.5f);
        const float corner = trackW * 0.5f;
        const auto track = area.withSizeKeepingCentre (trackW, area.getHeight());
        g.setColour (c::black2);
        g.fillRoundedRectangle (track, corner);
        g.setColour (c::line);
        g.drawRoundedRectangle (track, corner, 1.0f);

        // Fill: white body, magenta above -9 dB.
        const float pos = positionForDb (displayDb);
        if (pos > 0.002f)
        {
            const float accentFrom = positionForDb (-9.0f);
            const float whiteH = track.getHeight() * juce::jmin (pos, accentFrom);
            g.setColour (c::white.withAlpha (0.85f));
            g.fillRoundedRectangle (track.withTop (track.getBottom() - whiteH), corner);

            if (pos > accentFrom)
            {
                const float fillTop   = track.getBottom() - track.getHeight() * pos;
                const float accentTop = track.getBottom() - whiteH;
                g.setColour (c::accent);
                g.fillRoundedRectangle ({ track.getX(), fillTop, track.getWidth(), accentTop - fillTop },
                                        corner * 0.6f);
            }
        }

        // Peak-hold hairline.
        if (holdDb > kFloorDb + 1.0f)
        {
            const float holdY = track.getBottom() - track.getHeight() * positionForDb (holdDb);
            g.setColour (c::white.withAlpha (0.7f));
            g.fillRect (juce::Rectangle<float> (track.getX() - 2.0f, holdY - 0.75f,
                                                track.getWidth() + 4.0f, 1.5f));
        }
    }

private:
    static constexpr float kFloorDb = -60.0f;

    static float positionForDb (float db) noexcept
    {
        return juce::jlimit (0.0f, 1.0f, (db - kFloorDb) / -kFloorDb);
    }

    void timerCallback() override
    {
        const float peak = source.exchange (0.0f); // consume; audio thread re-publishes each block
        const float peakDb = peak > 1.0e-6f ? juce::Decibels::gainToDecibels (peak) : kFloorDb;

        // Fast rise, smooth fall (~27 dB/s).
        displayDb = peakDb > displayDb ? peakDb : juce::jmax (displayDb - 0.9f, kFloorDb);

        if (peakDb > holdDb || ++holdAge > 45) { holdDb = peakDb; holdAge = 0; }

        if (peak >= 0.999f) { clipLatched = true; clipAge = 0; }
        else if (clipLatched && ++clipAge > 90) clipLatched = false; // ~3 s latch

        repaint();
    }

    std::atomic<float>& source;
    juce::String label;
    float displayDb { kFloorDb }, holdDb { kFloorDb };
    int holdAge { 0 }, clipAge { 0 };
    bool clipLatched { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};

} // namespace rowen::ui
