// Rowen Fragment — preset management. Message-thread only (file I/O must
// never touch the audio thread). Presets are XML files validated before
// loading; a damaged or incompatible file fails safely with an error message
// instead of corrupting plugin state. Same architecture as Vocal Polish's
// PresetManager, extended with categories (factory presets only) and a small
// favorites list that spans both factory and user presets.
//
// Storage location (via juce::File::userApplicationDataDirectory):
//   Windows: %APPDATA%\Rowen\Fragment\Presets
//   macOS:   ~/Library/Rowen/Fragment/Presets
//   Linux:   ~/.config/Rowen/Fragment/Presets
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace rowen::presets
{

class PresetManager
{
public:
    explicit PresetManager (juce::AudioProcessorValueTreeState& stateToManage);

    struct Result
    {
        bool ok { false };
        juce::String errorMessage;
        static Result success() { return { true, {} }; }
        static Result failure (juce::String msg) { return { false, std::move (msg) }; }
    };

    // Factory presets ---------------------------------------------------------
    int getNumFactoryPresets() const;
    juce::String getFactoryPresetName (int index) const;
    juce::String getFactoryPresetCategory (int index) const;
    void applyFactoryPreset (int index);

    // User presets ------------------------------------------------------------
    juce::StringArray listUserPresetNames() const;
    Result saveUserPreset   (const juce::String& name, const juce::String& author = {},
                             const juce::String& description = {});
    Result loadUserPreset   (const juce::String& name);
    Result deleteUserPreset (const juce::String& name);
    Result renameUserPreset (const juce::String& oldName, const juce::String& newName);

    juce::File getUserPresetDirectory() const;

    // Favorites -----------------------------------------------------------------
    // Keyed by preset name (factory + user share one namespace here; a user
    // preset named identically to a factory preset is a rare edge case we
    // don't try to disambiguate further).
    bool isFavorite (const juce::String& name) const;
    void setFavorite (const juce::String& name, bool shouldBeFavorite);
    juce::StringArray listFavorites() const { return favorites; }

    static constexpr const char* kPresetFileExtension = ".rowenfragmentpreset";

private:
    Result applyPresetXml (const juce::XmlElement& root);
    juce::File presetFileFor (const juce::String& name) const;
    void setParameter (const juce::String& paramID, float naturalValue);

    juce::File favoritesFile() const;
    void loadFavorites();
    void saveFavorites() const;

    juce::AudioProcessorValueTreeState& apvts;
    juce::StringArray favorites;
};

} // namespace rowen::presets
