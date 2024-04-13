#pragma once

#include <string>

#include <vector>
#include <array>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <map>

#include <regex>
#include <mutex>
#include <thread>
#include <numeric>
#include <chrono>
#include <fstream>

#include "../Algorithms/Search.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"

#include "../CodeEditor/AppLog.hpp"
#include <future>

#include "Editor.hpp"

namespace ArmSimPro
{   
    // Derived Class
    class TextEditor : protected Editor
    {
    public:
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

        struct Identifier
        {
            Coordinates mLocation;
            std::string mDeclaration;
        };

        typedef std::string String;
        typedef std::unordered_map<std::string, Identifier> Identifiers;
        typedef std::unordered_set<std::string> Keywords;
        typedef std::map<int, std::string> ErrorMarkers;
        typedef std::unordered_set<int> Breakpoints;
        typedef std::array<ImU32, (unsigned)PaletteIndex::Max> Palette;
        typedef uint8_t Char;

        struct Glyph
        {
            Char mChar;
            PaletteIndex mColorIndex = PaletteIndex::Default;
            bool mComment : 1;
            bool mMultiLineComment : 1;
            bool mPreprocessor : 1;

            Glyph(Char aChar, PaletteIndex aColorIndex = PaletteIndex::Default) : mChar(aChar), mColorIndex(aColorIndex),
                mComment(false), mMultiLineComment(false), mPreprocessor(false) {}
        };

        typedef std::vector<Glyph> Line;
        typedef std::vector<Line> Lines;

        struct LanguageDefinition
        {
            typedef std::pair<std::string, PaletteIndex> TokenRegexString;
            typedef std::vector<TokenRegexString> TokenRegexStrings;
            typedef bool(*TokenizeCallback)(const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end, PaletteIndex & paletteIndex);

            std::string mName;
            Keywords mKeywords;
            Identifiers mIdentifiers;
            Identifiers mPreprocIdentifiers;
            std::string mCommentStart, mCommentEnd, mSingleLineComment;
            char mPreprocChar;
            bool mAutoIndentation;

            TokenizeCallback mTokenize;

            TokenRegexStrings mTokenRegexStrings;

            bool mCaseSensitive;

            LanguageDefinition()
                : mPreprocChar('#'), mAutoIndentation(true), mTokenize(nullptr), mCaseSensitive(true)
            {
            }
            static const LanguageDefinition& CPlusPlus();
            static const LanguageDefinition& C();
        };
        
        bool operator==(const std::string& full_path) const { return this->path == full_path; }
        TextEditor& operator=(const TextEditor& editor) { return *this; }
        TextEditor& operator=(TextEditor& editor) { return *this; }

        TextEditor();
        TextEditor(const std::string& full_path, const ImVec4& window_bg_col, ImFont* textfont, ImFont* defaultfont);
        ~TextEditor() {}
        //bool Render(const ImVec2& aSize = ImVec2(), bool aBorder = false, bool noMove = true);
        void Render(bool show_find_replace, std::string& to_find, std::string& to_replace, const ImVec2& aSize = ImVec2(), bool aBorder = false);

        std::string GetFileName() const { return file_name; }
        std::string GetFileExtension() const;
        std::string GetPath() const { return path; }

        void SetLanguageDefinition(const LanguageDefinition& aLanguageDef);
        const LanguageDefinition& GetLanguageDefinition() const { return mLanguageDefinition; }

        const Palette& GetPalette() const { return mPaletteBase; }
        void SetPalette(const Palette& aValue);

        void SetErrorMarkers(const ErrorMarkers& aMarkers) { mErrorMarkers = aMarkers; }
        void SetBreakpoints(const Breakpoints& aMarkers) { mBreakpoints = aMarkers; }

        void SetText(const std::string& aText) override;
        std::string GetText() const override;

        void SetTextLines(const std::vector<std::string>& aLines) override;
        std::vector<std::string> GetTextLines() const override;

        std::string GetSelectedText() const override;
        std::string GetCurrentLineText()const override;

        float GetReadingDuration() const {return this->mReadingFileDuration;}

        std::string GetTitle() const {return this->aTitle;}
        int GetTotalLines() const { return (int)this->mLines.size(); }
        bool IsOverwrite() const { return this->mOverwrite; }

        void SetReadOnly(bool aValue);
        bool IsReadOnly() const { return this->mReadOnly; }

        bool IsTextChanged() const override { return this->mTextChanged; } 
        bool IsCursorPositionChanged() const { return this->mCursorPositionChanged; }
        bool IsEditorFocused() const  { return this->isChildWindowFocus; }
        bool IsColorizerEnabled() const { return this->mColorizerEnabled; }

        void SetColorizerEnable(bool aValue);
        Coordinates GetCursorPosition() const { return GetActualCursorCoordinates(); }
        void SetCursorPosition(const Coordinates& aPosition) override;

        inline void SetHandleMouseInputs    (bool aValue){ mHandleMouseInputs    = aValue;}
        inline bool IsHandleMouseInputsEnabled() const { return mHandleKeyboardInputs; }

        inline void SetHandleKeyboardInputs (bool aValue){ mHandleKeyboardInputs = aValue;}
        inline bool IsHandleKeyboardInputsEnabled() const { return mHandleKeyboardInputs; }

        inline void SetImGuiChildIgnored(bool aValue){ mIgnoreImGuiChild     = aValue;}
        inline bool IsImGuiChildIgnored() const { return mIgnoreImGuiChild; }

        inline void SetShowWhitespaces(bool aValue) { mShowWhitespaces = aValue; }
        inline bool IsShowingWhitespaces() const { return mShowWhitespaces; }

        void SetTabSize(int aValue);
        inline int GetTabSize() const { return mTabSize; }

        void InsertText(const std::string& aValue) override;
        void InsertText(const char* aValue) override;

        void MoveUp(int aAmount = 1, bool aSelect = false) override;
        void MoveDown(int aAmount = 1, bool aSelect = false) override;
        void MoveLeft(int aAmount = 1, bool aSelect = false, bool aWordMode = false) override;
        void MoveRight(int aAmount = 1, bool aSelect = false, bool aWordMode = false) override;
        void MoveTop(bool aSelect = false) override;
        void MoveBottom(bool aSelect = false) override;
        void MoveHome(bool aSelect = false) override;
        void MoveEnd(bool aSelect = false) override;

        void SetSelectionStart(const Coordinates& aPosition) override;
        void SetSelectionEnd(const Coordinates& aPosition) override;
        void SetSelection(const Coordinates& aStart, const Coordinates& aEnd, SelectionMode aMode = SelectionMode::Normal) override;
        void SelectWordUnderCursor() override;
        void SelectAll() override;
        bool HasSelection() const override;

        void Copy() override;
        void Cut() override;
        void Paste() override;
        void Delete() override;

        bool CanUndo() const override;
        bool CanRedo() const override;
        void Undo(int aSteps = 1) override;
        void Redo(int aSteps = 1) override;

        static const Palette& GetDarkPalette();
        static const Palette& GetLightPalette();
        static const Palette& GetRetroBluePalette();

    private:
        typedef std::vector<std::pair<std::regex, PaletteIndex>> RegexList;

        typedef std::vector<UndoRecord> UndoBuffer;

        void SetRegexList(const std::string& first, const PaletteIndex& second);
        void RenderMainEditor(ImDrawList* drawList, int lineNo, ImVec2& cursorScreenPos, ImVec2& contentSize, float *longest, float scrollX, float spaceSize, char *buf, size_t buf_size = 16);

        void Colorize(int aFromLine = 0, int aCount = -1) override;
        void ColorizeRange(int aFromLine = 0, int aToLine = 0) override;
        void ColorizeInternal() override;
        float TextDistanceToLineStart(const Coordinates& aFrom) const override;
        void EnsureCursorVisible() override;
        int GetPageSize() const override;

        void SetTextAt(const Coordinates& position, const std::string& replacement) override;
        std::string GetText(const Coordinates& aStart, const Coordinates& aEnd) const override;
        Coordinates GetActualCursorCoordinates() const override;
        Coordinates SanitizeCoordinates(const Coordinates& aValue) const override;

        void Advance(Coordinates& aCoordinates) const override;
        void DeleteRange(const Coordinates& aStart, const Coordinates& aEnd) override;
        int InsertTextAt(Coordinates& aWhere, const char* aValue) override;
        void AddUndo(UndoRecord& aValue) override;

        Coordinates ScreenPosToCoordinates(const ImVec2& aPosition) const override;
        Coordinates FindWordStart(const Coordinates& aFrom) const override;
        Coordinates FindWordEnd(const Coordinates& aFrom) const override;
        Coordinates FindNextWord(const Coordinates& aFrom) const override;

        int GetCharacterIndex(const Coordinates& aCoordinates) const override;
        int GetCharacterColumn(int aLine, int aIndex) const override;
        int GetLineCharacterCount(int aLine) const override;
        int GetLineMaxColumn(int aLine) const override;

        bool IsOnWordBoundary(const Coordinates& aAt) const;
        void RemoveLine(int aStart, int aEnd);
        void RemoveLine(int aIndex);
        Line& InsertLine(int aIndex);
        void EnterCharacter(ImWchar aChar, bool aShift);
        void Backspace();
        void DeleteSelection();

        std::string GetWordUnderCursor() const;
        std::string GetWordAt(const Coordinates& aCoords) const;
        ImU32 GetGlyphColor(const Glyph& aGlyph) const;

        void HandleKeyboardInputs();
        void HandleMouseInputs();
        void RenderEditor(bool show_find_replace, std::string& to_find, std::string& to_replace);
        
    private:
        std::string aTitle;
        std::string path, file_name;

        float mLineSpacing;
        float mLastClick;
        float mTextStart;  // position (in pixels) where a code line starts relative to the left of the TextEditor.
        float mReadingFileDuration;

        int mUndoIndex;
        int mTabSize;
        int  mLeftMargin;
        int mColorRangeMin, mColorRangeMax;

        bool mOverwrite;
        bool mReadOnly;
        bool mWithinRender;
        bool mScrollToCursor;
        bool mScrollToTop;
        bool mTextChanged;
        bool mColorizerEnabled;
        bool mCursorPositionChanged;
        bool mHandleKeyboardInputs;
        bool mHandleMouseInputs;
        bool mIgnoreImGuiChild;
        bool mShowWhitespaces;
        bool mCheckComments;
        bool isChildWindowFocus;

        ImVec2 ChildWindow_Size;
        ImVec2 ChildWindow_Pos;

        const std::string _file_name;
        SelectionMode mSelectionMode;
        Palette mPaletteBase;
        Palette mPalette;
        LanguageDefinition mLanguageDefinition;
        RegexList mRegexList;

        Lines mLines;          //lines of codes
        EditorState mState;
        UndoBuffer mUndoBuffer;
        
        Breakpoints mBreakpoints;
        ErrorMarkers mErrorMarkers;
        ImVec2 mCharAdvance;
        Coordinates mInteractiveStart, mInteractiveEnd;

        std::string mLineBuffer;  //handles the colorized texts being displayed
        uint64_t mStartTime;
        const ImVec4 _window_bg_col;
    };

    struct TextEditorState
    {
        TextEditor editor;
        bool Open, IsModified, WantClose;
        TextEditorState(const TextEditor& txt_editor) : editor(txt_editor), Open(true), IsModified(false), WantClose(false) {}
        void DoQueueClose() {WantClose = true;}
        void DoForceClose() {Open = false; IsModified = false;}
        void SaveChanges()
        {
            auto TextLines = this->editor.GetTextLines();
            
            std::ofstream writer(this->editor.GetPath().c_str(), std::ios::trunc);
            if(!writer.good())
                return;
            
            for(auto it = TextLines.begin(); it != TextLines.end(); ++it)
                writer << *it << std::endl;
            writer.close();
        }
        bool operator==(const std::string& path) const { return this->editor.GetPath() == path; }
    };
};