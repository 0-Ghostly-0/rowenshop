#include "PresetManager.h"
#include "FactoryPresets.h"
#include "../params/ParameterIDs.h"

namespace rowen::presets
{

namespace
{
    constexpr const char* kRootTag       = "RowenPreset";
    constexpr const char* kParamTag      = "Param";
    constexpr const char* kAttrPlugin    = "plugin";
    constexpr const char* kAttrVersion   = "presetFormatVersion";
    constexpr const char* kAttrName      = "name";
    constexpr const char* kAttrAuthor    = "author";
    constexpr const char* kAttrDesc      = "description";
    constexpr const char* kAttrCategory  = "category";
    constexpr const char* kAttrId        = "id";
    constexpr const char* kAttrValue     = "value";
    constexpr const char* kPluginTag     = "Fragment";
    constexpr int         kFormatVersion = 1;
}

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& stateToManage)
    : apvts (stateToManage)
{
    loadFavorites();
}

//==============================================================================
int PresetManager::getNumFactoryPresets() const
{
    return int (getFactoryPresets().size());
}

juce::String PresetManager::getFactoryPresetName (int index) const
{
    const auto& presets = getFactoryPresets();
    if (index < 0 || index >= int (presets.size()))
        return {};
    return presets[size_t (index)].name;
}

juce::String PresetManager::getFactoryPresetCategory (int index) const
{
    const auto& presets = getFactoryPresets();
    if (index < 0 || index >= int (presets.size()))
        return {};
    return presets[size_t (index)].category;
}

void PresetManager::applyFactoryPreset (int index)
{
    const auto& presets = getFactoryPresets();
    if (index < 0 || index >= int (presets.size()))
        return;

    for (const auto& [paramId, value] : presets[size_t (index)].values)
        setParameter (paramId, value);
}

//==============================================================================
juce::File PresetManager::getUserPresetDirectory() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("Rowen")
               .getChildFile ("Fragment")
               .getChildFile ("Presets");
}

juce::File PresetManager::presetFileFor (const juce::String& name) const
{
    // File::createLegalFileName strips characters that are invalid on any OS.
    return getUserPresetDirectory().getChildFile (juce::File::createLegalFileName (name)
                                                  + kPresetFileExtension);
}

juce::StringArray PresetManager::listUserPresetNames() const
{
    juce::StringArray names;
    for (const auto& f : getUserPresetDirectory().findChildFiles (juce::File::findFiles, false,
                                                                  "*" + juce::String (kPresetFileExtension)))
    {
        if (auto xml = juce::XmlDocument::parse (f))
            if (xml->hasTagName (kRootTag))
                names.add (xml->getStringAttribute (kAttrName, f.getFileNameWithoutExtension()));
    }
    names.sortNatural();
    return names;
}

PresetManager::Result PresetManager::saveUserPreset (const juce::String& name,
                                                     const juce::String& author,
                                                     const juce::String& description)
{
    if (name.trim().isEmpty())
        return Result::failure ("Please enter a preset name.");

    const auto dir = getUserPresetDirectory();
    if (! dir.createDirectory())
        return Result::failure ("Could not create the preset folder:\n" + dir.getFullPathName());

    juce::XmlElement root (kRootTag);
    root.setAttribute (kAttrPlugin, kPluginTag);
    root.setAttribute (kAttrVersion, kFormatVersion);
    root.setAttribute (kAttrName, name.trim());
    root.setAttribute (kAttrCategory, "User");
    if (author.isNotEmpty())      root.setAttribute (kAttrAuthor, author);
    if (description.isNotEmpty()) root.setAttribute (kAttrDesc, description);

    // Store natural (display-range) values so preset files stay meaningful even
    // if a parameter's internal normalisation changes in a future version.
    for (auto* p : apvts.processor.getParameters())
    {
        if (auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*> (p))
        {
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p))
            {
                auto* el = root.createNewChildElement (kParamTag);
                el->setAttribute (kAttrId, withId->paramID);
                el->setAttribute (kAttrValue,
                                  double (ranged->convertFrom0to1 (ranged->getValue())));
            }
        }
    }

    const auto file = presetFileFor (name);
    if (! root.writeTo (file))
        return Result::failure ("Could not write the preset file:\n" + file.getFullPathName());

    return Result::success();
}

PresetManager::Result PresetManager::loadUserPreset (const juce::String& name)
{
    const auto file = presetFileFor (name);
    if (! file.existsAsFile())
        return Result::failure ("Preset not found: " + name);

    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr)
        return Result::failure ("This preset file is damaged and could not be read.");

    return applyPresetXml (*xml);
}

PresetManager::Result PresetManager::applyPresetXml (const juce::XmlElement& root)
{
    if (! root.hasTagName (kRootTag))
        return Result::failure ("This is not a Rowen preset file.");

    if (root.getStringAttribute (kAttrPlugin) != kPluginTag)
        return Result::failure ("This preset belongs to a different Rowen plugin.");

    if (root.getIntAttribute (kAttrVersion, 1) > kFormatVersion)
        return Result::failure ("This preset was made with a newer version of Fragment. "
                                "Please update the plugin.");

    int applied = 0;
    for (const auto* el : root.getChildWithTagNameIterator (kParamTag))
    {
        const auto paramId = el->getStringAttribute (kAttrId);
        if (! el->hasAttribute (kAttrValue) || paramId.isEmpty())
            continue;

        // Unknown IDs are skipped silently: presets from newer plugin versions
        // still load the parameters this version understands. Bypass is
        // deliberately never touched by a preset, even if a file somehow
        // contains it (matches the spec: presets must not change bypass).
        if (paramId == rowen::fragparam::id::bypass)
            continue;

        if (apvts.getParameter (paramId) != nullptr)
        {
            setParameter (paramId, float (el->getDoubleAttribute (kAttrValue)));
            ++applied;
        }
    }

    if (applied == 0)
        return Result::failure ("This preset file contains no usable settings.");

    return Result::success();
}

PresetManager::Result PresetManager::deleteUserPreset (const juce::String& name)
{
    const auto file = presetFileFor (name);
    if (! file.existsAsFile())
        return Result::failure ("Preset not found: " + name);
    if (! file.deleteFile())
        return Result::failure ("Could not delete the preset file.");
    setFavorite (name, false);
    return Result::success();
}

PresetManager::Result PresetManager::renameUserPreset (const juce::String& oldName,
                                                       const juce::String& newName)
{
    if (newName.trim().isEmpty())
        return Result::failure ("Please enter a new preset name.");

    const auto oldFile = presetFileFor (oldName);
    if (! oldFile.existsAsFile())
        return Result::failure ("Preset not found: " + oldName);

    auto xml = juce::XmlDocument::parse (oldFile);
    if (xml == nullptr || ! xml->hasTagName (kRootTag))
        return Result::failure ("This preset file is damaged and could not be renamed.");

    xml->setAttribute (kAttrName, newName.trim());
    const auto newFile = presetFileFor (newName);
    if (newFile.existsAsFile())
        return Result::failure ("A preset with that name already exists.");

    if (! xml->writeTo (newFile))
        return Result::failure ("Could not write the renamed preset.");

    oldFile.deleteFile();

    if (isFavorite (oldName))
    {
        setFavorite (oldName, false);
        setFavorite (newName, true);
    }

    return Result::success();
}

//==============================================================================
bool PresetManager::isFavorite (const juce::String& name) const
{
    return favorites.contains (name);
}

void PresetManager::setFavorite (const juce::String& name, bool shouldBeFavorite)
{
    const bool already = favorites.contains (name);
    if (shouldBeFavorite && ! already)
        favorites.add (name);
    else if (! shouldBeFavorite && already)
        favorites.removeString (name);
    else
        return; // no change

    saveFavorites();
}

juce::File PresetManager::favoritesFile() const
{
    return getUserPresetDirectory().getChildFile ("favorites.txt");
}

void PresetManager::loadFavorites()
{
    favorites.clear();
    const auto file = favoritesFile();
    if (! file.existsAsFile())
        return;

    juce::StringArray lines;
    file.readLines (lines);
    for (const auto& line : lines)
        if (line.trim().isNotEmpty())
            favorites.add (line.trim());
}

void PresetManager::saveFavorites() const
{
    const auto dir = getUserPresetDirectory();
    if (! dir.createDirectory())
        return; // favorites are a convenience; fail silently rather than crash

    favoritesFile().replaceWithText (favorites.joinIntoString ("\n"));
}

//==============================================================================
void PresetManager::setParameter (const juce::String& paramID, float naturalValue)
{
    if (auto* p = apvts.getParameter (paramID))
    {
        const float norm = p->convertTo0to1 (naturalValue);
        p->beginChangeGesture();
        p->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, norm));
        p->endChangeGesture();
    }
}

} // namespace rowen::presets
