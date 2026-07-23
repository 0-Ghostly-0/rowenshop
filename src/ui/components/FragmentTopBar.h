// Rowen Fragment — the top bar: logo, plugin name, a category-grouped preset
// browser with a Favorites shortcut section, undo/redo, help, and global
// bypass. Modeled on Vocal Polish's TopBar but extended for categories,
// favorites, and undo -- none of which Vocal Polish's flat preset list needed.
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h> // juce::UndoManager
#include <juce_gui_basics/juce_gui_basics.h>

#include "RowenIconButton.h"
#include "../../presets/PresetManager.h"

namespace rowen::ui
{

class FragmentTopBar final : public juce::Component, private juce::Timer
{
public:
    // requestText: host shows a RowenModal and calls back with the entered text.
    using TextRequest = std::function<void (const juce::String& title,
                                            const juce::String& initial,
                                            std::function<void (const juce::String&)> submit)>;

    FragmentTopBar (rowen::presets::PresetManager& manager,
                    juce::AudioProcessorValueTreeState& apvtsToUse,
                    juce::UndoManager& undoManagerToUse,
                    const juce::String& bypassParamID,
                    const juce::String& pluginTitle,
                    TextRequest requestTextFn,
                    std::function<void()> onHelpFn);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override; // keeps undo/redo enablement in sync

    void refreshList (const juce::String& selectName = {});
    void listSelectionChanged();
    void step (int direction);
    void showManageMenu();
    void showError (const juce::String& message);
    juce::String selectedName() const;
    bool isCurrentSelectionUserPreset() const;
    void toggleFavoriteSelected();
    void updateFavoriteButton();
    void applyWithUndo (const std::function<void()>& apply, const juce::String& transactionName);

    rowen::presets::PresetManager& presets;
    juce::AudioProcessorValueTreeState& apvts;
    juce::UndoManager& undoManager;
    juce::String title;
    TextRequest requestText;
    std::function<void()> onHelp;

    juce::Image logo;
    juce::ComboBox list;
    RowenIconButton prevButton     { "Previous preset", RowenIconButton::Icon::chevronLeft };
    RowenIconButton nextButton     { "Next preset",     RowenIconButton::Icon::chevronRight };
    RowenIconButton menuButton     { "Manage presets",  RowenIconButton::Icon::dots };
    RowenIconButton favoriteButton { "Favorite",        RowenIconButton::Icon::starOutline };
    RowenIconButton undoButton     { "Undo",            RowenIconButton::Icon::undo };
    RowenIconButton redoButton     { "Redo",            RowenIconButton::Icon::redo };
    RowenIconButton helpButton     { "Help",            RowenIconButton::Icon::help };
    RowenIconButton bypassButton   { "Bypass",          RowenIconButton::Icon::power };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    struct FavoriteEntry { bool isFactory; int factoryIndex; juce::String userName; };
    std::vector<FavoriteEntry> favoriteEntries;

    int numFactory { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FragmentTopBar)
};

} // namespace rowen::ui
