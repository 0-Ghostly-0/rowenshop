#include "RowenLookAndFeel.h"
#include "RowenTheme.h"
#include "RowenTypography.h"

namespace rowen::theme
{

RowenLookAndFeel::RowenLookAndFeel()
{
    // Base widget colors, mapped from the site palette.
    setColour (juce::ResizableWindow::backgroundColourId, color::black);
    setColour (juce::Label::textColourId, color::white);

    setColour (juce::Slider::textBoxTextColourId, color::white);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    setColour (juce::TextButton::buttonOnColourId, color::accentDim);
    setColour (juce::TextButton::textColourOffId, color::white);
    setColour (juce::TextButton::textColourOnId, color::white);

    setColour (juce::ComboBox::backgroundColourId, color::card);
    setColour (juce::ComboBox::textColourId, color::white);
    setColour (juce::ComboBox::outlineColourId, color::lineStrong);
    setColour (juce::ComboBox::arrowColourId, color::dim);

    setColour (juce::PopupMenu::backgroundColourId, color::card2);
    setColour (juce::PopupMenu::textColourId, color::white);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::white.withAlpha (0.08f));
    setColour (juce::PopupMenu::highlightedTextColourId, color::white);

    setColour (juce::TextEditor::backgroundColourId, color::black2);
    setColour (juce::TextEditor::textColourId, color::white);
    setColour (juce::TextEditor::outlineColourId, color::line);
    setColour (juce::TextEditor::focusedOutlineColourId, color::accent.withAlpha (0.6f));
    setColour (juce::CaretComponent::caretColourId, color::accent);

    setColour (juce::TooltipWindow::backgroundColourId, color::card2);
    setColour (juce::TooltipWindow::textColourId, color::white);
    setColour (juce::TooltipWindow::outlineColourId, color::lineStrong);
}

//==============================================================================
void RowenLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float pos, float startAngle, float endAngle,
                                         juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float> (float (x), float (y), float (width), float (height))
                            .reduced (4.0f);
    const float size    = juce::jmin (bounds.getWidth(), bounds.getHeight());
    const auto  square  = bounds.withSizeKeepingCentre (size, size);
    const auto  centre  = square.getCentre();
    const float radius  = size * 0.5f;
    const float arcW    = juce::jmax (2.0f, radius * 0.085f);
    const float arcR    = radius - arcW * 0.5f;
    const float angle   = startAngle + pos * (endAngle - startAngle);
    const bool  enabled = slider.isEnabled();
    const bool  hover   = slider.isMouseOverOrDragging() && enabled;

    // Track arc (hairline color).
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (enabled ? color::line.withAlpha (0.14f) : color::line);
    g.strokePath (track, juce::PathStrokeType (arcW, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    // Value arc (Rowen magenta) + soft glow, brand's accent-dim halo.
    if (enabled && pos > 0.001f)
    {
        juce::Path value;
        value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);

        g.setColour (color::accent.withAlpha (hover ? 0.45f : 0.30f));
        g.strokePath (value, juce::PathStrokeType (arcW * 2.2f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
        g.setColour (color::accent);
        g.strokePath (value, juce::PathStrokeType (arcW, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
    }

    // Knob face: card surface, hairline border, subtle top light.
    const float faceR = radius * 0.72f;
    const auto face = juce::Rectangle<float> (faceR * 2.0f, faceR * 2.0f).withCentre (centre);

    g.setColour (color::card2);
    g.fillEllipse (face);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.06f),
                                             centre.x, face.getY(),
                                             juce::Colours::transparentBlack,
                                             centre.x, face.getBottom(), false));
    g.fillEllipse (face);
    g.setColour (hover ? color::lineStrong.withAlpha (0.30f) : color::lineStrong);
    g.drawEllipse (face, metric::hairline);

    // Pointer.
    const float tipR  = faceR * 0.86f;
    const float baseR = faceR * 0.38f;
    const juce::Point<float> tip  (centre.x + tipR  * std::sin (angle), centre.y - tipR  * std::cos (angle));
    const juce::Point<float> base (centre.x + baseR * std::sin (angle), centre.y - baseR * std::cos (angle));
    g.setColour (enabled ? color::white : color::dimmer);
    g.drawLine ({ base, tip }, juce::jmax (2.0f, radius * 0.055f));
}

//==============================================================================
void RowenLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                             const juce::Colour&, bool highlighted, bool down)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const float r = juce::jmin (metric::pillRadius, bounds.getHeight() * 0.5f); // site pill buttons

    if (button.getToggleState())
        g.setColour (color::accentDim.withAlpha (down ? 0.5f : 0.35f));
    else if (down)
        g.setColour (juce::Colours::white.withAlpha (0.10f));
    else if (highlighted)
        g.setColour (juce::Colours::white.withAlpha (0.06f));
    else
        g.setColour (juce::Colours::transparentBlack);
    g.fillRoundedRectangle (bounds, r);

    g.setColour (button.getToggleState() ? color::accent.withAlpha (0.7f)
                                         : (highlighted ? juce::Colours::white.withAlpha (0.35f)
                                                        : color::lineStrong));
    g.drawRoundedRectangle (bounds, r, metric::hairline);
}

juce::Font RowenLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return Typography::get().body (juce::jmin (14.0f, float (buttonHeight) * 0.55f));
}

//==============================================================================
void RowenLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                     int, int, int, int, juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<float> (0, 0, float (width), float (height)).reduced (0.5f);

    g.setColour (color::card);
    g.fillRoundedRectangle (bounds, metric::radiusSmall);
    g.setColour (box.isMouseOver() ? juce::Colours::white.withAlpha (0.35f) : color::lineStrong);
    g.drawRoundedRectangle (bounds, metric::radiusSmall, metric::hairline);

    // Chevron.
    const float cx = float (width) - float (height) * 0.5f;
    const float cy = float (height) * 0.5f;
    const float s  = juce::jmin (5.0f, float (height) * 0.18f);
    juce::Path chevron;
    chevron.startNewSubPath (cx - s, cy - s * 0.5f);
    chevron.lineTo (cx, cy + s * 0.5f);
    chevron.lineTo (cx + s, cy - s * 0.5f);
    g.setColour (color::dim);
    g.strokePath (chevron, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
}

juce::Font RowenLookAndFeel::getComboBoxFont (juce::ComboBox& box)
{
    return Typography::get().body (juce::jmin (14.0f, float (box.getHeight()) * 0.5f));
}

void RowenLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll (color::card2);
    g.setColour (color::lineStrong);
    g.drawRect (juce::Rectangle<int> (width, height), 1);
}

juce::Font RowenLookAndFeel::getPopupMenuFont()
{
    return Typography::get().body (14.0f);
}

//==============================================================================
juce::Font RowenLookAndFeel::getLabelFont (juce::Label& label)
{
    return Typography::get().body (label.getFont().getHeight());
}

//==============================================================================
void RowenLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text,
                                    int width, int height)
{
    const auto bounds = juce::Rectangle<float> (0, 0, float (width), float (height)).reduced (0.5f);
    g.setColour (color::card2);
    g.fillRoundedRectangle (bounds, metric::radiusSmall);
    g.setColour (color::lineStrong);
    g.drawRoundedRectangle (bounds, metric::radiusSmall, metric::hairline);

    g.setColour (color::white);
    g.setFont (Typography::get().body (13.0f));
    g.drawFittedText (text, juce::Rectangle<int> (width, height).reduced (10, 6),
                      juce::Justification::centredLeft, 3);
}

juce::Rectangle<int> RowenLookAndFeel::getTooltipBounds (const juce::String& tipText,
                                                         juce::Point<int> screenPos,
                                                         juce::Rectangle<int> parentArea)
{
    const auto font = Typography::get().body (13.0f);
    const int w = juce::jmin (260, juce::roundToInt (juce::GlyphArrangement::getStringWidth (font, tipText)) + 22);
    const int lines = 1 + (juce::roundToInt (juce::GlyphArrangement::getStringWidth (font, tipText)) + 21) / 238;
    const int h = 12 + lines * 16;

    return juce::Rectangle<int> (screenPos.x + 12, screenPos.y + 20, w, h)
               .constrainedWithin (parentArea);
}

} // namespace rowen::theme
