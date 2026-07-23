// Rowen Fragment — the buffer display. This is Fragment's visual centerpiece
// and the thing that gives it its own personality next to Vocal Polish's
// calmer meters: a fragmented, block-based waveform (not a continuous line)
// that quietly shimmers even when the source is static, with small marker
// "sparks" that fly across it — one per active grain voice, sized and faded
// by envelope age, shaped by engine (circle = Engine A, diamond = Engine B),
// trailed in their direction of travel (forward/reverse).
//
// Purely decorative animation (the shimmer phase) is layered on top of real
// engine data (VizExchange) on the UI thread only — it never feeds back into
// the audio path, so it cannot affect DSP determinism or tests.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../theme/RowenTheme.h"
#include "../theme/RowenTypography.h"
#include "../../viz/VizSnapshot.h"

namespace rowen::ui
{

class BufferDisplay final : public juce::Component, private juce::Timer
{
public:
    explicit BufferDisplay (rowen::frag::VizExchange& exchangeToWatch)
        : exchange (exchangeToWatch)
    {
        startTimerHz (30);
    }

    void paint (juce::Graphics& g) override
    {
        namespace c = rowen::theme::color;
        namespace m = rowen::theme::metric;
        auto& type = rowen::theme::Typography::get();

        const auto bounds = getLocalBounds().toFloat();

        g.setColour (c::card);
        g.fillRoundedRectangle (bounds, m::radius);

        // A faint frozen tint reads as "this is holding still", not an error.
        if (data.frozen != 0)
        {
            g.setColour (c::accent.withAlpha (0.05f));
            g.fillRoundedRectangle (bounds, m::radius);
        }

        g.saveState();
        g.reduceClipRegion (bounds.getSmallestIntegerContainer());

        auto area = bounds.reduced (m::radius * 0.8f);
        drawWaveform (g, area);
        drawMarkers (g, area);

        g.restoreState();

        g.setColour (data.frozen != 0 ? c::accent.withAlpha (0.5f) : c::lineStrong);
        g.drawRoundedRectangle (bounds.reduced (0.5f), m::radius, m::hairline);

        // Status chip, bottom-left: window length + state.
        juce::String status = juce::String (data.windowSeconds, 2) + " s";
        if (data.frozen != 0)       status += "  FROZEN";
        else if (data.playing == 0) status += "  IDLE";

        const auto chipArea = bounds.reduced (12.0f, 8.0f).removeFromBottom (16.0f);
        g.setColour (c::dimmer);
        g.setFont (type.mono (11.0f));
        g.drawText (status, chipArea.getSmallestIntegerContainer(), juce::Justification::bottomLeft);
    }

    void resized() override {}

private:
    void timerCallback() override
    {
        exchange.read (data); // keeps the previous frame on a rare torn read
        shimmerPhase += 0.045f;
        if (shimmerPhase > juce::MathConstants<float>::twoPi)
            shimmerPhase -= juce::MathConstants<float>::twoPi;
        repaint();
    }

    // A block waveform, not a continuous line: each bin is its own little
    // fragment with a gap beside it, and a slow per-bin shimmer nudges each
    // block's height a little independently of its neighbours. Reads as
    // restless / granular rather than a calm mirrored waveform.
    void drawWaveform (juce::Graphics& g, juce::Rectangle<float> area)
    {
        namespace c = rowen::theme::color;
        const float midY = area.getCentreY();
        const int bins = rowen::frag::VizData::kBins;
        const float gap = 1.0f;
        const float slot = area.getWidth() / float (bins);
        const float blockW = juce::jmax (1.0f, slot - gap);

        for (int b = 0; b < bins; ++b)
        {
            const float raw = juce::jlimit (0.0f, 1.0f, data.peaks[b]);
            // Deterministic per-bin shimmer offset so blocks don't pulse in
            // lockstep — a fast golden-ratio-spaced phase per index.
            const float shimmer = 0.90f + 0.10f * std::sin (shimmerPhase + float (b) * 0.618f);
            const float h = juce::jmax (2.0f, raw * shimmer * area.getHeight() * 0.46f);
            const float x = area.getX() + slot * float (b);

            const float alpha = data.frozen != 0 ? 0.35f : (0.55f + 0.35f * raw);
            g.setColour (c::white.withAlpha (alpha));
            g.fillRoundedRectangle (x, midY - h * 0.5f, blockW, h, blockW * 0.4f);
        }

        g.setColour (c::line);
        g.drawHorizontalLine (int (midY), area.getX(), area.getRight());
    }

    void drawMarkers (juce::Graphics& g, juce::Rectangle<float> area)
    {
        namespace c = rowen::theme::color;

        for (int i = 0; i < rowen::frag::VizData::kMaxMarkers; ++i)
        {
            const auto& mk = data.markers[i];
            if (mk.active == 0)
                continue;

            const float x = area.getX() + area.getWidth() * juce::jlimit (0.0f, 1.0f, mk.position01);
            // Spread markers vertically by slot so simultaneous grains don't
            // stack directly on top of each other; engine B sits slightly
            // lower than engine A so the two voices read as distinct layers.
            const float lane = float (i % 5) / 4.0f - 0.5f;
            const float engineOffset = mk.engine == 0 ? -1.0f : 1.0f;
            const float y = area.getCentreY() + lane * area.getHeight() * 0.32f
                                + engineOffset * area.getHeight() * 0.09f;

            const float age = juce::jlimit (0.0f, 1.0f, mk.age01);
            const float fade = 1.0f - age;
            const float size = juce::jmap (fade, 0.0f, 1.0f, 3.0f, 8.0f);

            const auto centre = juce::Point<float> (x, y);
            const auto dot = juce::Rectangle<float> (size, size).withCentre (centre);

            g.setColour (c::accent.withAlpha (0.15f + 0.35f * fade));
            g.fillEllipse (dot.expanded (size * 0.6f));

            g.setColour (c::accent.withAlpha (0.5f + 0.5f * fade));
            if (mk.engine == 0)
                g.fillEllipse (dot); // Engine A: circle
            else
            {
                juce::Path diamond;
                diamond.addQuadrilateral (centre.x, centre.y - size * 0.5f,
                                          centre.x + size * 0.5f, centre.y,
                                          centre.x, centre.y + size * 0.5f,
                                          centre.x - size * 0.5f, centre.y);
                g.fillPath (diamond); // Engine B: diamond
            }

            // A short trail in the direction of travel.
            const float trail = size * 1.8f * float (mk.direction);
            g.setColour (c::accent.withAlpha (0.25f * fade));
            g.drawLine (centre.x, centre.y, centre.x - trail, centre.y, 1.2f);
        }
    }

    rowen::frag::VizExchange& exchange;
    rowen::frag::VizData data;
    float shimmerPhase { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferDisplay)
};

} // namespace rowen::ui
