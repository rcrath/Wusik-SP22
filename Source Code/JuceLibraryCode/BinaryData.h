/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   Background_png;
    const int            Background_pngSize = 86068;

    extern const char*   Button1_png;
    const int            Button1_pngSize = 2012;

    extern const char*   Button2_png;
    const int            Button2_pngSize = 30668;

    extern const char*   Button3_png;
    const int            Button3_pngSize = 1846;

    extern const char*   Logo_W_Apenas_Square_png;
    const int            Logo_W_Apenas_Square_pngSize = 48708;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 5;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
