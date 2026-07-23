// Rowen embedded asset access. Resources are embedded from assets/ at build
// time (see CMakeLists juce_add_binary_data). Lookup is by name token so the
// exact generated symbol names never matter, and a missing asset degrades
// gracefully (empty image) instead of breaking the build.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#if __has_include("BinaryData.h")
 #include "BinaryData.h"
 #define ROWEN_HAS_BINARY_DATA 1
#else
 #define ROWEN_HAS_BINARY_DATA 0
#endif

namespace rowen::theme
{

inline juce::Image loadImageByToken (const char* nameToken)
{
   #if ROWEN_HAS_BINARY_DATA
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
    {
        const juce::String name (BinaryData::namedResourceList[i]);
        if (name.containsIgnoreCase (nameToken))
        {
            int dataSize = 0;
            if (const char* data = BinaryData::getNamedResource (BinaryData::namedResourceList[i], dataSize))
            {
                auto img = juce::ImageCache::getFromMemory (data, dataSize);
                if (img.isValid())
                    return img;
            }
        }
    }
   #else
    juce::ignoreUnused (nameToken);
   #endif
    return {};
}

} // namespace rowen::theme
