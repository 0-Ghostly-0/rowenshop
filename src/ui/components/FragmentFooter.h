// Rowen Fragment — the footer. Version + site link only; Fragment has no
// gain-match style toggle worth putting here, so this is a trimmed version
// of Vocal Polish's FooterBar rather than a shared component.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../theme/RowenTheme.h"
#include "../theme/RowenTypography.h"

namespace rowen::ui
{

class FragmentFooter final : public juce::Component
{
public:
    explicit FragmentFooter (const juce::String& versionText) : version (versionText)
    {
        siteLink.setButtonText ("rowen.work");
        siteLink.setURL (juce::URL ("https://rowen.work"));
        siteLink.setColour (juce::HyperlinkButton::textColourId, rowen::theme::color::dimmer);
        siteLink.setTooltip ("Visit the Rowen site.");
        addAndMakeVisible (siteLink);
    }

    void paint (juce::Graphics& g) override
    {
        namespace c = rowen::theme::color;
        g.setColour (c::line);
        g.fillRect (getLocalBounds().withHeight (1));

        auto& type = rowen::theme::Typography::get();
        g.setColour (c::dimmer);
        g.setFont (type.mono (10.0f));
        g.drawText ("v" + version, getLocalBounds().reduced (12, 0), juce::Justification::centredLeft);
    }

    void resized() override
    {
        const int linkW = 90;
        siteLink.setFont (rowen::theme::Typography::get().body (11.0f), false, juce::Justification::centred);
        siteLink.setBounds (getLocalBounds().withSizeKeepingCentre (linkW, getHeight()));
    }

private:
    juce::String version;
    juce::HyperlinkButton siteLink;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FragmentFooter)
};

} // namespace rowen::ui
