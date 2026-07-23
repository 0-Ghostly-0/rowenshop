#include "RowenTypography.h"

#if __has_include("BinaryData.h")
 #include "BinaryData.h"
 #define ROWEN_HAS_BINARY_DATA 1
#else
 #define ROWEN_HAS_BINARY_DATA 0
#endif

namespace rowen::theme
{

namespace
{
    // Finds an embedded font whose resource name contains the given token
    // (BinaryData names are derived from filenames, e.g. SpaceGrotesekMedium_ttf).
    juce::Typeface::Ptr loadEmbeddedFont (const char* nameToken)
    {
       #if ROWEN_HAS_BINARY_DATA
        for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
        {
            const juce::String resourceName (BinaryData::namedResourceList[i]);
            if (resourceName.containsIgnoreCase (nameToken)
                && (resourceName.endsWithIgnoreCase ("_ttf") || resourceName.endsWithIgnoreCase ("_otf")))
            {
                int dataSize = 0;
                if (const char* data = BinaryData::getNamedResource (BinaryData::namedResourceList[i], dataSize))
                    return juce::Typeface::createSystemTypefaceFor (data, size_t (dataSize));
            }
        }
       #else
        juce::ignoreUnused (nameToken);
       #endif
        return nullptr;
    }
}

Typography::Typography()
{
    headingTypeface = loadEmbeddedFont ("SpaceGrotesk");

    // Body: Inter is the site's stack; Poppins is the shipped alternative.
    bodyTypeface = loadEmbeddedFont ("Inter");
    if (bodyTypeface == nullptr)
        bodyTypeface = loadEmbeddedFont ("Poppins");

    monoTypeface = loadEmbeddedFont ("JetBrainsMono");
}

} // namespace rowen::theme
