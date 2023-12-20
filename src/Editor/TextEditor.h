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

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"

namespace ArmSimPro
{
    class TextEditor
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

        enum class SelectionMode
        {
            Normal,
            Word,
            Line
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

        // Represents a character coordinate from the user's point of view,
        // i. e. consider an uniform grid (assuming fixed-width font) on the
        // screen as it is rendered, and each cell has its own coordinate, starting from 0.
        // Tabs are counted as [1..mTabSize] count empty spaces, depending on
        // how many space is necessary to reach the next tab stop.
        // For example, coordinate (1, 5) represents the character 'B' in a line "\tABC", when mTabSize = 4,
        // because it is rendered as "    ABC" on the screen.
        struct Coordinates
        {
            int mLine, mColumn;
            Coordinates() : mLine(0), mColumn(0) {}
            Coordinates(int aLine, int aColumn) : mLine(aLine), mColumn(aColumn)
            {
                assert(aLine >= 0);
                assert(aColumn >= 0);
            }
            static Coordinates Invalid() { static Coordinates invalid(-1, -1); return invalid; }

            bool operator ==(const Coordinates& o) const
            {
                return
                    mLine == o.mLine &&
                    mColumn == o.mColumn;
            }

            bool operator !=(const Coordinates& o) const
            {
                return
                    mLine != o.mLine ||
                    mColumn != o.mColumn;
            }

            bool operator <(const Coordinates& o) const
            {
                if (mLine != o.mLine)
                    return mLine < o.mLine;
                return mColumn < o.mColumn;
            }

            bool operator >(const Coordinates& o) const
            {
                if (mLine != o.mLine)
                    return mLine > o.mLine;
                return mColumn > o.mColumn;
            }

            bool operator <=(const Coordinates& o) const
            {
                if (mLine != o.mLine)
                    return mLine < o.mLine;
                return mColumn <= o.mColumn;
            }

            bool operator >=(const Coordinates& o) const
            {
                if (mLine != o.mLine)
                    return mLine > o.mLine;
                return mColumn >= o.mColumn;
            }
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
        TextEditor(const std::string& full_path, const ImVec4& window_bg_col);
        ~TextEditor() {}
        //bool Render(const ImVec2& aSize = ImVec2(), bool aBorder = false, bool noMove = true);
        void Render(const ImVec2& aSize = ImVec2(), bool aBorder = false);

        std::string GetFileName() const { return file_name; }
        std::string GetFileExtension() const;
        std::string GetPath() const { return path; }

        void SetLanguageDefinition(const LanguageDefinition& aLanguageDef);
        const LanguageDefinition& GetLanguageDefinition() const { return mLanguageDefinition; }

        const Palette& GetPalette() const { return mPaletteBase; }
        void SetPalette(const Palette& aValue);

        void SetErrorMarkers(const ErrorMarkers& aMarkers) { mErrorMarkers = aMarkers; }
        void SetBreakpoints(const Breakpoints& aMarkers) { mBreakpoints = aMarkers; }

        void SetText(const std::string& aText);
        std::string GetText() const;

        void SetTextLines(const std::vector<std::string>& aLines);
        std::vector<std::string> GetTextLines() const;

        std::string GetSelectedText() const;
        std::string GetCurrentLineText()const;

        float GetReadingDuration() const {return this->mReadingFileDuration;}

        std::string GetTitle() const {return this->aTitle;}
        int GetTotalLines() const { return (int)this->mLines.size(); }
        bool IsOverwrite() const { return this->mOverwrite; }

        void SetReadOnly(bool aValue);
        bool IsReadOnly() const { return this->mReadOnly; }

        bool IsTextChanged() const { return this->mTextChanged; } 
        bool IsCursorPositionChanged() const { return this->mCursorPositionChanged; }
        bool IsChildWindowFocused() const  { return isChildWindowFocus; }
        bool IsColorizerEnabled() const { return this->mColorizerEnabled; }

        void SetColorizerEnable(bool aValue);
        Coordinates GetCursorPosition() const { return GetActualCursorCoordinates(); }
        void SetCursorPosition(const Coordinates& aPosition);

        inline void SetHandleMouseInputs    (bool aValue){ mHandleMouseInputs    = aValue;}
        inline bool IsHandleMouseInputsEnabled() const { return mHandleKeyboardInputs; }

        inline void SetHandleKeyboardInputs (bool aValue){ mHandleKeyboardInputs = aValue;}
        inline bool IsHandleKeyboardInputsEnabled() const { return mHandleKeyboardInputs; }

        inline void SetImGuiChildIgnored    (bool aValue){ mIgnoreImGuiChild     = aValue;}
        inline bool IsImGuiChildIgnored() const { return mIgnoreImGuiChild; }

        inline void SetShowWhitespaces(bool aValue) { mShowWhitespaces = aValue; }
        inline bool IsShowingWhitespaces() const { return mShowWhitespaces; }

        void SetTabSize(int aValue);
        inline int GetTabSize() const { return mTabSize; }

        void InsertText(const std::string& aValue);
        void InsertText(const char* aValue);

        void MoveUp(int aAmount = 1, bool aSelect = false);
        void MoveDown(int aAmount = 1, bool aSelect = false);
        void MoveLeft(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
        void MoveRight(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
        void MoveTop(bool aSelect = false);
        void MoveBottom(bool aSelect = false);
        void MoveHome(bool aSelect = false);
        void MoveEnd(bool aSelect = false);

        void SetSelectionStart(const Coordinates& aPosition);
        void SetSelectionEnd(const Coordinates& aPosition);
        void SetSelection(const Coordinates& aStart, const Coordinates& aEnd, SelectionMode aMode = SelectionMode::Normal);
        void SelectWordUnderCursor();
        void SelectAll();
        bool HasSelection() const;

        void Copy();
        void Cut();
        void Paste();
        void Delete();

        bool CanUndo() const;
        bool CanRedo() const;
        void Undo(int aSteps = 1);
        void Redo(int aSteps = 1);

        static const Palette& GetDarkPalette();
        static const Palette& GetLightPalette();
        static const Palette& GetRetroBluePalette();

    private:
        typedef std::vector<std::pair<std::regex, PaletteIndex>> RegexList;

        struct EditorState
        {
            Coordinates mSelectionStart;
            Coordinates mSelectionEnd;
            Coordinates mCursorPosition;
        };

        class UndoRecord
        {
        public:
            UndoRecord() {}
            ~UndoRecord() {}

            UndoRecord(
                const std::string& aAdded,
                const TextEditor::Coordinates aAddedStart,
                const TextEditor::Coordinates aAddedEnd,

                const std::string& aRemoved,
                const TextEditor::Coordinates aRemovedStart,
                const TextEditor::Coordinates aRemovedEnd,

                TextEditor::EditorState& aBefore,
                TextEditor::EditorState& aAfter);

            void Undo(TextEditor* aEditor);
            void Redo(TextEditor* aEditor);

            std::string mAdded;
            Coordinates mAddedStart;
            Coordinates mAddedEnd;

            std::string mRemoved;
            Coordinates mRemovedStart;
            Coordinates mRemovedEnd;

            EditorState mBefore;
            EditorState mAfter;
        };

        typedef std::vector<UndoRecord> UndoBuffer;

        void SetRegexList(const std::string& first, const PaletteIndex& second);
        void RenderMainEditor(ImDrawList* drawList, int lineNo, ImVec2& cursorScreenPos, ImVec2& contentSize, float *longest, float scrollX, float spaceSize, char *buf, size_t buf_size = 16);

        void ProcessInputs();
        void Colorize(int aFromLine = 0, int aCount = -1);
        void ColorizeRange(int aFromLine = 0, int aToLine = 0);
        void ColorizeInternal();
        float TextDistanceToLineStart(const Coordinates& aFrom) const;
        void EnsureCursorVisible();
        int GetPageSize() const;

        std::string GetText(const Coordinates& aStart, const Coordinates& aEnd) const;
        Coordinates GetActualCursorCoordinates() const;
        Coordinates SanitizeCoordinates(const Coordinates& aValue) const;

        void Advance(Coordinates& aCoordinates) const;
        void DeleteRange(const Coordinates& aStart, const Coordinates& aEnd);
        int InsertTextAt(Coordinates& aWhere, const char* aValue);
        void AddUndo(UndoRecord& aValue);

        Coordinates ScreenPosToCoordinates(const ImVec2& aPosition) const;
        Coordinates FindWordStart(const Coordinates& aFrom) const;
        Coordinates FindWordEnd(const Coordinates& aFrom) const;
        Coordinates FindNextWord(const Coordinates& aFrom) const;

        int GetCharacterIndex(const Coordinates& aCoordinates) const;
        int GetCharacterColumn(int aLine, int aIndex) const;
        int GetLineCharacterCount(int aLine) const;
        int GetLineMaxColumn(int aLine) const;
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
        void RenderEditor();
        
    private:
        std::string aTitle;
        std::string path, file_name;
        bool isChildWindowFocus;
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