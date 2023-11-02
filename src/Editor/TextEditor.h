#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

namespace ArmSimPro
{
    class CodeEditor
    {   
        enum class PaletteIndex
        {
            Default,
            Keyword,
            Number,
            String,
            CharLiteral,
            Punctuation,
            Preprocessor,
            Identifier,
            KnownIdentifier,
            PreprocIdentifier,
            Comment,
            MultiLineComment,
            Background,
            Cursor,
            Selection,
            ErrorMarker,
            Breakpoint,
            LineNumber,
            CurrentLineFill,
            CurrentLineFillInactive,
            CurrentLineEdge,
            Max
        };

        struct Breakpoint
        {
            int mLine;
            bool mEnabled;
            std::string mCondition;

            Breakpoint()
                : mLine(-1)
                , mEnabled(false)
            {}
        };
    private:
        float _dockspace_width;
        float _dockspace_height;

        std::vector<std::string> OpenFiles;
    public:
        CodeEditor();
        ~CodeEditor();
        
        //should be called at run time inside loop
        void AppendOpenFile(const std::string& file_name);
        void SetCodeEditor(float right_margin, float top_margin,  float bottom_margin);

        void SetDockSpaceWidth(float val)  {_dockspace_width = val;}
        void SetDockSpaceHeight(float val) {_dockspace_height = val;}
        
    private:

    };
};