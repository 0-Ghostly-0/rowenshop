// Rowen icon button — circular hairline button with a procedurally drawn icon
// (no icon fonts or image assets). Used for help, bypass power, preset arrows
// and the preset "more" menu. Reusable across Rowen plugins.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../theme/RowenTheme.h"
#include "../theme/RowenTypography.h"

namespace rowen::ui
{

class RowenIconButton : public juce::Button
{
public:
    enum class Icon { help, power, dots, chevronLeft, chevronRight, close, undo, redo, star, starOutline };

    RowenIconButton (const juce::String& name, Icon iconType)
        : juce::Button (name), icon (iconType)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    // For parameters like "bypass" where toggle ON should look inactive:
    // the lit (accent) state is shown when the toggle is OFF instead.
    void setInvertedToggleDisplay (bool shouldInvert) { invertDisplay = shouldInvert; }

    // Lets a single button swap glyphs at runtime (e.g. the favorite star
    // switching between outline and filled) instead of needing two buttons.
    void setIcon (Icon newIcon) { icon = newIcon; repaint(); }

    void paintButton (juce::Graphics& g, bool highlighted, bool down) override
    {
        namespace c = rowen::theme::color;
        const auto b = getLocalBounds().toFloat().reduced (1.0f);
        const auto circle = b.withSizeKeepingCentre (juce::jmin (b.getWidth(), b.getHeight()),
                                                     juce::jmin (b.getWidth(), b.getHeight()));

        const bool on = invertDisplay ? ! getToggleState() : getToggleState();
        if (down)             g.setColour (juce::Colours::white.withAlpha (0.10f));
        else if (highlighted) g.setColour (juce::Colours::white.withAlpha (0.06f));
        else                  g.setColour (juce::Colours::transparentBlack);
        g.fillEllipse (circle);

        g.setColour (on ? c::accent.withAlpha (0.8f)
                        : highlighted ? juce::Colours::white.withAlpha (0.35f) : c::lineStrong);
        g.drawEllipse (circle, 1.0f);

        const auto iconColor = on ? c::accent : (highlighted ? c::white : c::dim);
        g.setColour (iconColor);
        drawIcon (g, circle.reduced (circle.getWidth() * 0.28f));
    }

private:
    void drawIcon (juce::Graphics& g, juce::Rectangle<float> a)
    {
        const float t = juce::jmax (1.4f, a.getWidth() * 0.14f);
        juce::Path p;

        switch (icon)
        {
            case Icon::help:
            {
                g.setFont (rowen::theme::Typography::get().heading (a.getHeight() * 1.5f));
                g.drawText ("?", a.expanded (4.0f), juce::Justification::centred);
                return;
            }
            case Icon::power:
            {
                const auto c = a.getCentre();
                const float r = a.getWidth() * 0.5f;
                p.addCentredArc (c.x, c.y + r * 0.08f, r * 0.85f, r * 0.85f, 0.0f,
                                 0.6f, juce::MathConstants<float>::twoPi - 0.6f, true);
                p.startNewSubPath (c.x, c.y - r);
                p.lineTo (c.x, c.y - r * 0.15f);
                break;
            }
            case Icon::dots:
            {
                const float r = juce::jmax (1.2f, a.getWidth() * 0.09f);
                for (int i = -1; i <= 1; ++i)
                    g.fillEllipse (a.getCentreX() + float (i) * r * 3.4f - r,
                                   a.getCentreY() - r, r * 2.0f, r * 2.0f);
                return;
            }
            case Icon::chevronLeft:
            case Icon::chevronRight:
            {
                const float dir = icon == Icon::chevronLeft ? -1.0f : 1.0f;
                const auto c = a.getCentre();
                const float s = a.getWidth() * 0.32f;
                p.startNewSubPath (c.x - dir * s * 0.5f, c.y - s);
                p.lineTo (c.x + dir * s * 0.5f, c.y);
                p.lineTo (c.x - dir * s * 0.5f, c.y + s);
                break;
            }
            case Icon::close:
            {
                const auto c = a.getCentre();
                const float s = a.getWidth() * 0.4f;
                p.startNewSubPath (c.x - s, c.y - s); p.lineTo (c.x + s, c.y + s);
                p.startNewSubPath (c.x + s, c.y - s); p.lineTo (c.x - s, c.y + s);
                break;
            }
            case Icon::undo:
            case Icon::redo:
            {
                // A curved arrow: an open arc plus a small arrowhead at one end.
                const auto c = a.getCentre();
                const float r = a.getWidth() * 0.42f;
                const float dir = icon == Icon::undo ? -1.0f : 1.0f;
                const float startAngle = dir > 0.0f ? juce::MathConstants<float>::pi * 1.15f
                                                    : -juce::MathConstants<float>::pi * 0.15f;
                const float endAngle = dir > 0.0f ? juce::MathConstants<float>::pi * 2.35f
                                                  : juce::MathConstants<float>::pi * 1.15f;
                p.addCentredArc (c.x, c.y, r, r, 0.0f, startAngle, endAngle, true);

                const auto tip = juce::Point<float> (c.x + r * std::cos (endAngle),
                                                     c.y + r * std::sin (endAngle));
                const float headAngle = endAngle + dir * juce::MathConstants<float>::halfPi * 0.5f;
                const float headLen = r * 0.62f;
                const auto perp = juce::Point<float> (c.x + r * std::cos (endAngle - dir * 0.5f),
                                                      c.y + r * std::sin (endAngle - dir * 0.5f));
                p.startNewSubPath (tip.x - headLen * std::cos (headAngle),
                                   tip.y - headLen * std::sin (headAngle));
                p.lineTo (tip);
                p.lineTo (perp);
                break;
            }
            case Icon::star:
            case Icon::starOutline:
            {
                const auto c = a.getCentre();
                const float outerR = a.getWidth() * 0.5f;
                const float innerR = outerR * 0.42f;
                juce::Path star;
                for (int i = 0; i < 10; ++i)
                {
                    const float ang = float (i) * juce::MathConstants<float>::pi / 5.0f
                                          - juce::MathConstants<float>::halfPi;
                    const float rad = (i % 2 == 0) ? outerR : innerR;
                    const juce::Point<float> pt (c.x + rad * std::cos (ang), c.y + rad * std::sin (ang));
                    if (i == 0) star.startNewSubPath (pt); else star.lineTo (pt);
                }
                star.closeSubPath();

                if (icon == Icon::star)
                    g.fillPath (star);
                else
                    g.strokePath (star, juce::PathStrokeType (juce::jmax (1.2f, a.getWidth() * 0.09f),
                                                              juce::PathStrokeType::curved,
                                                              juce::PathStrokeType::rounded));
                return;
            }
        }

        g.strokePath (p, juce::PathStrokeType (t, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    Icon icon;
    bool invertDisplay { false };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RowenIconButton)
};

} // namespace rowen::ui
