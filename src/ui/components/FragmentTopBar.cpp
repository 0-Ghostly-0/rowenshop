#include "FragmentTopBar.h"
#include "../theme/RowenAssets.h"
#include "../theme/RowenTheme.h"
#include "../theme/RowenTypography.h"
#include "../../params/ParameterIDs.h"
#include "../../presets/UndoableStateChange.h"

namespace rowen::ui
{

namespace
{
    // Combo item IDs are split into three widely-spaced ranges so the same
    // preset can appear both in its category section and in Favorites
    // without ID collisions: 1..999 = favorites shortcuts, 10000+ = factory
    // (offset by factory index), 20000+ = user (offset by user-list index).
    constexpr int kFavoriteIdBase = 1;
    constexpr int kFactoryIdBase  = 10000;
    constexpr int kUserIdBase     = 20000;
}

FragmentTopBar::FragmentTopBar (rowen::presets::PresetManager& manager,
                                juce::AudioProcessorValueTreeState& apvtsToUse,
                                juce::UndoManager& undoManagerToUse,
                                const juce::String& bypassParamID,
                                const juce::String& pluginTitle,
                                TextRequest requestTextFn,
                                std::function<void()> onHelpFn)
    : presets (manager),
      apvts (apvtsToUse),
      undoManager (undoManagerToUse),
      title (pluginTitle),
      requestText (std::move (requestTextFn)),
      onHelp (std::move (onHelpFn))
{
    logo = rowen::theme::loadImageByToken ("logo-icon");

    list.setTextWhenNothingSelected ("Presets");
    list.setTooltip ("Choose a starting point, browse by category, or save your own.");
    list.onChange = [this] { listSelectionChanged(); };
    addAndMakeVisible (list);

    prevButton.setTooltip ("Previous preset");
    prevButton.onClick = [this] { step (-1); };
    addAndMakeVisible (prevButton);

    nextButton.setTooltip ("Next preset");
    nextButton.onClick = [this] { step (1); };
    addAndMakeVisible (nextButton);

    favoriteButton.setTooltip ("Mark the current preset as a favorite.");
    favoriteButton.onClick = [this] { toggleFavoriteSelected(); };
    addAndMakeVisible (favoriteButton);

    menuButton.setTooltip ("Save, rename or delete your presets.");
    menuButton.onClick = [this] { showManageMenu(); };
    addAndMakeVisible (menuButton);

    undoButton.setTooltip ("Undo the last preset load or randomize.");
    undoButton.onClick = [this] { undoManager.undo(); };
    addAndMakeVisible (undoButton);

    redoButton.setTooltip ("Redo.");
    redoButton.onClick = [this] { undoManager.redo(); };
    addAndMakeVisible (redoButton);

    helpButton.setTooltip ("A short guide to every control.");
    helpButton.onClick = [this] { if (onHelp) onHelp(); };
    addAndMakeVisible (helpButton);

    bypassButton.setClickingTogglesState (true);
    bypassButton.setInvertedToggleDisplay (true); // lit = active, not bypassed
    bypassButton.setTooltip ("Bypasses the whole plugin.");
    addAndMakeVisible (bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, bypassParamID, bypassButton);

    refreshList();
    startTimerHz (10); // enable/disable undo/redo as the stack changes
}

//==============================================================================
void FragmentTopBar::timerCallback()
{
    undoButton.setEnabled (undoManager.canUndo());
    redoButton.setEnabled (undoManager.canRedo());
}

//==============================================================================
void FragmentTopBar::paint (juce::Graphics& g)
{
    namespace c = rowen::theme::color;
    const auto bounds = getLocalBounds();

    g.setColour (c::line);
    g.fillRect (bounds.withTop (bounds.getBottom() - 1)); // bottom hairline

    const float s = float (getHeight()) / 68.0f;
    auto logoArea = bounds.reduced (juce::roundToInt (12 * s)).removeFromTop (juce::roundToInt (32 * s));

    int titleX = logoArea.getX();
    if (logo.isValid())
    {
        const float h = float (logoArea.getHeight()) * 0.72f;
        const float w = h * float (logo.getWidth()) / float (logo.getHeight());
        const juce::Rectangle<float> dest (float (logoArea.getX()),
                                           float (logoArea.getCentreY()) - h * 0.5f,
                                           w, h);
        g.drawImage (logo, dest, juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yMid
                                 | juce::RectanglePlacement::onlyReduceInSize);
        titleX = juce::roundToInt (dest.getRight() + 8.0f * s);
    }

    g.setColour (c::white);
    g.setFont (rowen::theme::Typography::get().heading (13.0f * s));
    g.drawText (title.toUpperCase(),
                juce::Rectangle<int> (titleX, logoArea.getY(), logoArea.getCentreX() - titleX, logoArea.getHeight()),
                juce::Justification::centredLeft);
}

void FragmentTopBar::resized()
{
    const float s = float (getHeight()) / 68.0f;
    auto area = getLocalBounds().reduced (juce::roundToInt (12 * s));

    // Row A: logo/title (drawn in paint, space just reserved) + help/bypass.
    auto rowA = area.removeFromTop (juce::roundToInt (32 * s));
    const int btnA = juce::roundToInt (26 * s);
    bypassButton.setBounds (rowA.removeFromRight (btnA).withSizeKeepingCentre (btnA, btnA));
    rowA.removeFromRight (juce::roundToInt (6 * s));
    helpButton.setBounds (rowA.removeFromRight (btnA).withSizeKeepingCentre (btnA, btnA));

    area.removeFromTop (juce::roundToInt (4 * s));

    // Row B: preset browser.
    auto rowB = area;
    const int btnB = juce::roundToInt (24 * s);

    auto right = rowB;
    redoButton.setBounds (right.removeFromRight (btnB).withSizeKeepingCentre (btnB, btnB));
    right.removeFromRight (juce::roundToInt (3 * s));
    undoButton.setBounds (right.removeFromRight (btnB).withSizeKeepingCentre (btnB, btnB));
    right.removeFromRight (juce::roundToInt (8 * s));
    menuButton.setBounds (right.removeFromRight (btnB).withSizeKeepingCentre (btnB, btnB));
    right.removeFromRight (juce::roundToInt (3 * s));
    favoriteButton.setBounds (right.removeFromRight (btnB).withSizeKeepingCentre (btnB, btnB));
    right.removeFromRight (juce::roundToInt (6 * s));
    nextButton.setBounds (right.removeFromRight (btnB).withSizeKeepingCentre (btnB, btnB));

    auto left = right;
    prevButton.setBounds (left.removeFromLeft (btnB).withSizeKeepingCentre (btnB, btnB));
    left.removeFromLeft (juce::roundToInt (6 * s));
    list.setBounds (left.withSizeKeepingCentre (left.getWidth(), juce::roundToInt (26 * s)));
}

//==============================================================================
void FragmentTopBar::refreshList (const juce::String& selectName)
{
    list.clear (juce::dontSendNotification);
    favoriteEntries.clear();

    numFactory = presets.getNumFactoryPresets();
    const auto userNames = presets.listUserPresetNames();
    const auto favNames = presets.listFavorites();

    if (! favNames.isEmpty())
    {
        list.addSectionHeading ("FAVORITES");
        for (const auto& favName : favNames)
        {
            int factoryIdx = -1;
            for (int i = 0; i < numFactory; ++i)
            {
                if (presets.getFactoryPresetName (i) == favName) { factoryIdx = i; break; }
            }
            const bool isUser = userNames.contains (favName);
            if (factoryIdx < 0 && ! isUser)
                continue; // stale favorite pointing at a deleted preset

            const int entryIndex = int (favoriteEntries.size());
            favoriteEntries.push_back ({ factoryIdx >= 0, factoryIdx, favName });
            list.addItem (favName, kFavoriteIdBase + entryIndex);
        }
    }

    juce::String lastCategory;
    for (int i = 0; i < numFactory; ++i)
    {
        const auto cat = presets.getFactoryPresetCategory (i);
        if (cat != lastCategory)
        {
            list.addSectionHeading (cat.toUpperCase());
            lastCategory = cat;
        }
        list.addItem (presets.getFactoryPresetName (i), kFactoryIdBase + i);
    }

    if (! userNames.isEmpty())
    {
        list.addSectionHeading ("USER");
        for (int i = 0; i < userNames.size(); ++i)
            list.addItem (userNames[i], kUserIdBase + i);
    }

    if (selectName.isNotEmpty())
    {
        for (int i = 0; i < list.getNumItems(); ++i)
        {
            if (list.getItemText (i) == selectName)
            {
                list.setSelectedItemIndex (i, juce::dontSendNotification);
                break;
            }
        }
    }

    updateFavoriteButton();
}

juce::String FragmentTopBar::selectedName() const
{
    const int idx = list.getSelectedItemIndex();
    return idx >= 0 ? list.getItemText (idx) : juce::String();
}

bool FragmentTopBar::isCurrentSelectionUserPreset() const
{
    const int id = list.getSelectedId();
    if (id >= kUserIdBase)
        return true;
    if (id >= kFactoryIdBase)
        return false;
    const int favIdx = id - kFavoriteIdBase;
    if (favIdx >= 0 && favIdx < int (favoriteEntries.size()))
        return ! favoriteEntries[size_t (favIdx)].isFactory;
    return false;
}

void FragmentTopBar::applyWithUndo (const std::function<void()>& apply, const juce::String& transactionName)
{
    auto before = rowen::presets::StateChangeAction::snapshotAll (apvts, rowen::fragparam::id::bypass);
    apply();
    auto after = rowen::presets::StateChangeAction::snapshotAll (apvts, rowen::fragparam::id::bypass);

    undoManager.beginNewTransaction (transactionName);
    undoManager.perform (new rowen::presets::StateChangeAction (apvts, std::move (before), std::move (after)));
}

void FragmentTopBar::listSelectionChanged()
{
    const int id = list.getSelectedId();
    if (id <= 0)
        return;

    if (id >= kUserIdBase)
    {
        const auto userNames = presets.listUserPresetNames();
        const int uIdx = id - kUserIdBase;
        if (uIdx >= 0 && uIdx < userNames.size())
        {
            const auto name = userNames[uIdx];
            applyWithUndo ([this, name]
            {
                const auto r = presets.loadUserPreset (name);
                if (! r.ok) showError (r.errorMessage);
            }, "Load preset: " + name);
        }
    }
    else if (id >= kFactoryIdBase)
    {
        const int fIdx = id - kFactoryIdBase;
        const auto name = presets.getFactoryPresetName (fIdx);
        applyWithUndo ([this, fIdx] { presets.applyFactoryPreset (fIdx); }, "Load preset: " + name);
    }
    else
    {
        const int favIdx = id - kFavoriteIdBase;
        if (favIdx >= 0 && favIdx < int (favoriteEntries.size()))
        {
            const auto entry = favoriteEntries[size_t (favIdx)];
            applyWithUndo ([this, entry]
            {
                if (entry.isFactory)
                    presets.applyFactoryPreset (entry.factoryIndex);
                else
                {
                    const auto r = presets.loadUserPreset (entry.userName);
                    if (! r.ok) showError (r.errorMessage);
                }
            }, "Load preset: " + entry.userName);
        }
    }

    updateFavoriteButton();
}

void FragmentTopBar::step (int direction)
{
    const int count = list.getNumItems();
    if (count == 0)
        return;
    int idx = juce::jmax (0, list.getSelectedItemIndex());
    for (int tries = 0; tries < count; ++tries)
    {
        idx = (idx + direction + count) % count;
        if (list.getItemId (idx) > 0) // skip section headings, which return id 0
        {
            list.setSelectedItemIndex (idx);
            return;
        }
    }
}

void FragmentTopBar::toggleFavoriteSelected()
{
    const auto name = selectedName();
    if (name.isEmpty())
        return;
    presets.setFavorite (name, ! presets.isFavorite (name));
    refreshList (name);
}

void FragmentTopBar::updateFavoriteButton()
{
    const auto name = selectedName();
    const bool fav = name.isNotEmpty() && presets.isFavorite (name);
    favoriteButton.setToggleState (fav, juce::dontSendNotification);
    favoriteButton.setIcon (fav ? RowenIconButton::Icon::star : RowenIconButton::Icon::starOutline);
    favoriteButton.setEnabled (name.isNotEmpty());
}

void FragmentTopBar::showManageMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, "Save as new preset...");
    menu.addSeparator();
    menu.addItem (2, "Rename selected...", isCurrentSelectionUserPreset());
    menu.addItem (3, "Delete selected", isCurrentSelectionUserPreset());
    menu.addSeparator();
    menu.addItem (4, "Open presets folder");

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (menuButton),
        [this] (int choice)
        {
            switch (choice)
            {
                case 1:
                    requestText ("Save preset", "My Fragment",
                        [this] (const juce::String& name)
                        {
                            const auto r = presets.saveUserPreset (name);
                            if (r.ok) refreshList (name.trim());
                            else showError (r.errorMessage);
                        });
                    break;

                case 2:
                    requestText ("Rename preset", selectedName(),
                        [this, oldName = selectedName()] (const juce::String& newName)
                        {
                            const auto r = presets.renameUserPreset (oldName, newName);
                            if (r.ok) refreshList (newName.trim());
                            else showError (r.errorMessage);
                        });
                    break;

                case 3:
                {
                    const auto r = presets.deleteUserPreset (selectedName());
                    if (r.ok) refreshList();
                    else showError (r.errorMessage);
                    break;
                }

                case 4:
                {
                    auto dir = presets.getUserPresetDirectory();
                    dir.createDirectory();
                    dir.revealToUser();
                    break;
                }

                default: break;
            }
        });
}

void FragmentTopBar::showError (const juce::String& message)
{
    juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                      .withIconType (juce::MessageBoxIconType::WarningIcon)
                                      .withTitle ("Presets")
                                      .withMessage (message)
                                      .withButton ("OK"),
                                  nullptr);
}

} // namespace rowen::ui
