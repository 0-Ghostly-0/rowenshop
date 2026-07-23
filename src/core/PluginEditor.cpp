#include "PluginEditor.h"
#include "../params/ParameterIDs.h"
#include "../presets/UndoableStateChange.h"
#include "../ui/theme/RowenAssets.h"
#include "../ui/theme/RowenTheme.h"

namespace rowen
{

using namespace rowen::fragparam;
namespace c = rowen::theme::color;

namespace
{
    constexpr float kBaseW = 640.0f;
    constexpr float kBaseH = 768.0f;

    void fillChoiceBox (juce::ComboBox& box, std::initializer_list<const char*> items)
    {
        int i = 1;
        for (const auto* item : items)
            box.addItem (item, i++);
    }

    rowen::ui::RowenKnob::ValueFormatter semitoneFormatter()
    {
        return [] (double v)
        {
            return (v >= 0.05 ? "+" : "") + juce::String (v, 1) + " st";
        };
    }

    rowen::ui::RowenKnob::ValueFormatter msFormatter()
    {
        return [] (double v)
        {
            return v >= 1000.0 ? juce::String (v / 1000.0, 2) + " s"
                                : juce::String (juce::roundToInt (v)) + " ms";
        };
    }
}

FragmentEditor::FragmentEditor (FragmentProcessor& p)
    : juce::AudioProcessorEditor (p),
      processor (p),
      topBar (p.getPresetManager(), p.getState(), p.getUndoManager(), id::bypass, "Rowen Fragment",
              // Text-entry requests from the top bar open the branded modal:
              [this] (const juce::String& title, const juce::String& initial,
                      std::function<void (const juce::String&)> submit)
              { modal.show (title, initial, std::move (submit)); },
              [this] { helpPanel.setVisible (true); }),
      footer (JucePlugin_VersionString),
      bufferDisplay (p.getVizExchange()),
      fragmentKnob (p.getState(), id::fragment, "Fragment",
                    "How much of the sound gets broken into pieces.",
                    ui::RowenKnob::percentFormatter()),
      shift      (p.getState(), id::shift,      "Shift",
                  "Pitches the fragments up or down.", semitoneFormatter()),
      reverse    (p.getState(), id::reverse,    "Reverse",
                  "How often fragments play backwards.", ui::RowenKnob::percentFormatter()),
      scatter    (p.getState(), id::scatter,    "Scatter",
                  "How far fragments reach into the recent past.", ui::RowenKnob::percentFormatter()),
      drift      (p.getState(), id::drift,      "Drift",
                  "Slow, wandering movement in pitch, position and pan.", ui::RowenKnob::percentFormatter()),
      atmosphere (p.getState(), id::atmosphere, "Atmosphere",
                  "Adds a diffuse, spacious trail behind the fragments.", ui::RowenKnob::percentFormatter()),
      movement   (p.getState(), id::movement,   "Movement",
                  "How deep Drift's modulation reaches.", ui::RowenKnob::percentFormatter()),
      mix        (p.getState(), id::mix,        "Mix",
                  "Blends the fragmented signal with your original.", ui::RowenKnob::percentFormatter()),
      outGain    (p.getState(), id::outGain,    "Output",
                  "Sets the final level leaving the plugin.", ui::RowenKnob::decibelFormatter()),
      freeLength (p.getState(), id::bufferFree, "Free Length",
                  "The rolling buffer length when Buffer Length is set to Free.", msFormatter()),
      engineADot (p.getState(), id::engineA, "Engine A", "Turns the stable Engine A on or off."),
      engineBDot (p.getState(), id::engineB, "Engine B", "Turns the textural Engine B on or off."),
      inMeter  (p.inputPeak,  "IN"),
      outMeter (p.outputPeak, "OUT")
{
    setLookAndFeel (&lookAndFeel);
    texture = theme::loadImageByToken ("floral");

    addAndMakeVisible (topBar);
    addAndMakeVisible (footer);
    addAndMakeVisible (bufferDisplay);

    for (auto* k : { &fragmentKnob, &shift, &reverse, &scatter, &drift, &atmosphere,
                     &movement, &mix, &outGain, &freeLength })
        addAndMakeVisible (*k);

    for (auto* d : { &engineADot, &engineBDot })
        addAndMakeVisible (d->dot);

    fillChoiceBox (bufferLenBox, { "1/4 Bar", "1/2 Bar", "1 Bar", "2 Bars", "4 Bars", "Free" });
    addAndMakeVisible (bufferLenBox);
    bufferLenAttachment = std::make_unique<ComboAttachment> (p.getState(), id::bufferLen, bufferLenBox);

    fillChoiceBox (shiftSnapBox, { "Semitones", "Fifths", "Octaves", "Off" });
    shiftSnapBox.setTooltip ("Snaps Shift to musical intervals as you drag.");
    addAndMakeVisible (shiftSnapBox);
    shiftSnapAttachment = std::make_unique<ComboAttachment> (p.getState(), id::shiftSnap, shiftSnapBox);

    extendedDot.setTooltip ("Extends Shift's range from +/-12 to +/-24 semitones.");
    extendedDot.setTitle ("Extended range");
    addAndMakeVisible (extendedDot);
    extendedAttachment = std::make_unique<ButtonAttachment> (p.getState(), id::shiftExt, extendedDot);

    freezeButton.setClickingTogglesState (true);
    freezeButton.setTooltip ("Locks the buffer so it stops capturing new audio and only replays what's already inside it.");
    addAndMakeVisible (freezeButton);
    freezeAttachment = std::make_unique<ButtonAttachment> (p.getState(), id::freeze, freezeButton);

    randomizeButton.setTooltip ("Nudges the main controls to a new tasteful combination. Right-click for strength.");
    randomizeButton.onClick = [this] { randomize (RandomStrength::balanced); };
    randomizeButton.onRightClick = [this] { showRandomizeMenu(); };
    addAndMakeVisible (randomizeButton);

    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);

    addChildComponent (helpPanel); // hidden until requested
    addChildComponent (modal);

    setResizable (true, true);
    setResizeLimits (480, 576, 1180, 1416);
    if (auto* constrainer = getConstrainer())
        constrainer->setFixedAspectRatio (double (kBaseW) / double (kBaseH));
    setSize (int (kBaseW), int (kBaseH));
}

FragmentEditor::~FragmentEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void FragmentEditor::randomize (RandomStrength strength)
{
    // "Controlled chaos": nudge the character controls to a fresh but
    // tasteful combination rather than fully uncontrolled values. Strength
    // widens or narrows the range each control is drawn from; it never
    // touches Output, Bypass, or preset/license state, per spec.
    struct Range { float lo, hi; };
    struct Ranges
    {
        Range fragment, shift, reverse, scatter, drift, atmosphere;
    };

    static const Ranges subtle   { { 20, 45 }, { -3, 3 },  { 5, 25 },  { 10, 35 }, { 5, 25 },  { 0, 30 } };
    static const Ranges balanced { { 25, 70 }, { -7, 7 },  { 10, 60 }, { 15, 75 }, { 5, 60 },  { 0, 60 } };
    static const Ranges extreme  { { 50, 95 }, { -14, 14 }, { 30, 90 }, { 40, 95 }, { 40, 95 }, { 20, 80 } };

    const Ranges& r = strength == RandomStrength::subtle  ? subtle
                     : strength == RandomStrength::extreme ? extreme
                                                            : balanced;

    static const std::vector<juce::String> touchedParams = {
        id::fragment, id::shift, id::reverse, id::scatter, id::drift, id::atmosphere
    };

    auto& apvts = processor.getState();
    const auto before = presets::StateChangeAction::snapshot (apvts, touchedParams);

    auto set = [&] (const char* paramID, float value)
    {
        if (auto* param = apvts.getParameter (paramID))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 (value));
            param->endChangeGesture();
        }
    };
    auto pick = [this] (Range range) { return range.lo + rng.nextFloat() * (range.hi - range.lo); };

    set (id::fragment,   pick (r.fragment));
    set (id::shift,      pick (r.shift));
    set (id::reverse,    pick (r.reverse));
    set (id::scatter,    pick (r.scatter));
    set (id::drift,      pick (r.drift));
    set (id::atmosphere, pick (r.atmosphere));

    const auto after = presets::StateChangeAction::snapshot (apvts, touchedParams);
    processor.getUndoManager().beginNewTransaction ("Randomize");
    processor.getUndoManager().perform (new presets::StateChangeAction (apvts, before, after));
}

void FragmentEditor::showRandomizeMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, "Subtle");
    menu.addItem (2, "Balanced");
    menu.addItem (3, "Extreme");

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (randomizeButton),
        [this] (int choice)
        {
            switch (choice)
            {
                case 1: randomize (RandomStrength::subtle);   break;
                case 2: randomize (RandomStrength::balanced); break;
                case 3: randomize (RandomStrength::extreme);  break;
                default: break;
            }
        });
}

//==============================================================================
void FragmentEditor::paint (juce::Graphics& g)
{
    g.fillAll (c::black);

    // Same hero treatment as the website / Vocal Polish: halftone floral,
    // faded toward the lower half.
    if (texture.isValid())
    {
        const float w = float (getWidth());
        const float texH = float (getHeight()) * 0.5f;
        const float aspect = float (texture.getWidth()) / float (texture.getHeight());
        const float drawW = juce::jmax (w, texH * aspect);

        g.saveState();
        g.reduceClipRegion (juce::Rectangle<int> (0, 0, getWidth(), int (texH)));
        g.setOpacity (0.35f);
        g.drawImage (texture, { (w - drawW) * 0.5f, 0.0f, drawW, texH },
                     juce::RectanglePlacement::fillDestination);
        g.restoreState();

        g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack,
                                                 0.0f, texH * 0.4f,
                                                 c::black, 0.0f, texH, false));
        g.fillRect (juce::Rectangle<float> (0.0f, texH * 0.4f, w, texH * 0.6f));
    }

    auto& type = theme::Typography::get();

    g.setColour (c::dimmer);
    g.setFont (type.heading (11.0f));
    g.drawText ("ENGINES", engineLabelArea, juce::Justification::centredLeft);
    g.drawText ("BUFFER", bufferLabelArea, juce::Justification::centredLeft);
}

void FragmentEditor::resized()
{
    const float s = float (getWidth()) / kBaseW;
    auto scaled = [s] (float v) { return juce::roundToInt (v * s); };

    auto area = getLocalBounds();
    topBar.setBounds (area.removeFromTop (scaled (68)));
    footer.setBounds (area.removeFromBottom (scaled (26)));
    helpPanel.setBounds (getLocalBounds());
    modal.setBounds (getLocalBounds());

    area.reduce (scaled (14), scaled (8));

    // --- Control strip: engine toggles (left), buffer length (right) --------
    auto strip = area.removeFromTop (scaled (24));
    auto stripLeft = strip.removeFromLeft (scaled (140));
    engineLabelArea = stripLeft.removeFromLeft (scaled (58));
    engineADot.dot.setBounds (stripLeft.removeFromLeft (scaled (22)).withSizeKeepingCentre (scaled (16), scaled (16)));
    engineBDot.dot.setBounds (stripLeft.removeFromLeft (scaled (22)).withSizeKeepingCentre (scaled (16), scaled (16)));

    auto stripRight = strip.removeFromRight (scaled (250));
    freeLength.setBounds (stripRight.removeFromRight (scaled (86)));
    bufferLenBox.setBounds (stripRight.removeFromRight (scaled (96)).withSizeKeepingCentre (scaled (96), scaled (22)));
    bufferLabelArea = stripRight.removeFromRight (scaled (56));

    area.removeFromTop (scaled (8));

    // --- The centerpiece display ---------------------------------------------
    bufferDisplay.setBounds (area.removeFromTop (scaled (152)));
    area.removeFromTop (scaled (12));

    // --- Fragment: the dominant control --------------------------------------
    auto fragmentArea = area.removeFromTop (scaled (170));
    fragmentKnob.setBounds (fragmentArea.withSizeKeepingCentre (scaled (168), scaled (170)));

    area.removeFromTop (scaled (6));

    // --- Character row: Shift / Reverse / Scatter / Drift / Atmosphere ------
    const int subRowH = scaled (18);
    auto subRow = area.removeFromTop (subRowH);
    auto knobRow = area.removeFromTop (scaled (108));

    struct Placement { ui::RowenKnob* knob; float w; };
    Placement row[] = { { &shift, 100.0f }, { &reverse, 100.0f }, { &scatter, 100.0f },
                        { &drift, 100.0f }, { &atmosphere, 100.0f } };

    float totalW = 0.0f;
    for (const auto& pl : row) totalW += pl.w;
    const int gap = juce::jmax (0, (knobRow.getWidth() - scaled (totalW)) / 4);

    int x = knobRow.getX();
    juce::Rectangle<int> shiftCell;
    for (size_t i = 0; i < sizeof (row) / sizeof (row[0]); ++i)
    {
        const int w = scaled (row[i].w);
        auto cell = juce::Rectangle<int> (x, knobRow.getY(), w, knobRow.getHeight());
        row[i].knob->setBounds (cell);
        if (i == 0) shiftCell = juce::Rectangle<int> (x, subRow.getY(), w, subRow.getHeight());
        x += w + gap;
    }

    // Shift's snap combo + extended-range dot ride in the sub-row, under Shift.
    auto shiftSub = shiftCell.reduced (scaled (2), 0);
    extendedDot.setBounds (shiftSub.removeFromRight (subRowH).withSizeKeepingCentre (
        scaled (14), scaled (14)));
    shiftSub.removeFromRight (scaled (4));
    shiftSnapBox.setBounds (shiftSub);

    area.removeFromTop (scaled (10));

    // --- Flanks: meters + Freeze/Randomize, with Movement/Mix/Output between -
    auto bottomRow = area;
    auto leftCol  = bottomRow.removeFromLeft (scaled (60));
    auto rightCol = bottomRow.removeFromRight (scaled (60));

    freezeButton.setBounds (leftCol.removeFromBottom (scaled (28)));
    leftCol.removeFromBottom (scaled (6));
    inMeter.setBounds (leftCol.reduced (scaled (6), 0));

    randomizeButton.setBounds (rightCol.removeFromBottom (scaled (28)));
    rightCol.removeFromBottom (scaled (6));
    outMeter.setBounds (rightCol.reduced (scaled (6), 0));

    bottomRow.reduce (scaled (10), 0);

    Placement mixRow[] = { { &movement, 110.0f }, { &mix, 110.0f }, { &outGain, 110.0f } };
    float mixTotalW = 0.0f;
    for (const auto& pl : mixRow) mixTotalW += pl.w;
    const int mixGap = juce::jmax (0, (bottomRow.getWidth() - scaled (mixTotalW)) / 2);

    int mx = bottomRow.getX();
    for (auto& pl : mixRow)
    {
        const int w = scaled (pl.w);
        pl.knob->setBounds (juce::Rectangle<int> (mx, bottomRow.getY(), w, bottomRow.getHeight()));
        mx += w + mixGap;
    }
}

} // namespace rowen
