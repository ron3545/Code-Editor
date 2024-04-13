#pragma once
#include "../Algorithms/Search.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"

#include "../CodeEditor/AppLog.hpp"
#include <future>
#include <vector>
#include "Coordinate.hpp"

namespace ArmSimPro
{   
    
    //Base class
    class Editor
    {
    protected:
        enum class SelectionMode
        {
            Normal,
            Word,
            Line
        };

        enum class SearchNavMode
        {
            SearchNavMode_UP,
            SearchNaveMode_Down
        };

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
                const Coordinates aAddedStart,
                const Coordinates aAddedEnd,

                const std::string& aRemoved,
                const Coordinates aRemovedStart,
                const Coordinates aRemovedEnd,

                EditorState& aBefore,
                EditorState& aAfter);

            void Undo(Editor* aEditor, EditorState* mState);
            void Redo(Editor* aEditor, EditorState* mState);

            std::string mAdded;
            Coordinates mAddedStart;
            Coordinates mAddedEnd;

            std::string mRemoved;
            Coordinates mRemovedStart;
            Coordinates mRemovedEnd;

            EditorState mBefore;
            EditorState mAfter;
        };
        
        Editor(ImFont* textfont,  ImFont* defaultfont) : TextFont(textfont), DefaultFont(defaultfont){}
        virtual ~Editor() {}
        
        //For Debugging purposes only
        void ShowAppLog(bool* p_open, const Search::KeyInstances_Position& positions);
        void ShowAppLog(bool* p_open, const std::vector<Coordinates>& positions);
        void ShowAppLog(bool* p_open, int lineNo, const ImVec2& start, const ImVec2& end);

        void Show_Search_Panel(std::string& to_find, std::string& to_replace, const ImVec2& offset, const ImVec2& pos_offset, bool show_panel, const ImVec4& bg_col, ImVec2& cursorScreenPos, ImVec2 mCharAdvance, float mTextStart);
        void DrawHighlightsFromSearch(int to_find_size, ImVec2& cursorScreenPos, ImVec2 mCharAdvance, float mTextStart);

        virtual void Advance(Coordinates& aCoordinates) const = 0;
        virtual void DeleteRange(const Coordinates& aStart, const Coordinates& aEnd)= 0;
        virtual int InsertTextAt(Coordinates& aWhere, const char* aValue)= 0;
        virtual void AddUndo(UndoRecord& aValue)= 0;
        virtual bool IsTextChanged() const = 0;

        virtual Coordinates ScreenPosToCoordinates(const ImVec2& aPosition) const = 0;
        virtual Coordinates FindWordStart(const Coordinates& aFrom) const = 0;
        virtual Coordinates FindWordEnd(const Coordinates& aFrom) const = 0;
        virtual Coordinates FindNextWord(const Coordinates& aFrom) const = 0;
        
        virtual int GetCharacterIndex(const Coordinates& aCoordinates) const = 0;
        virtual int GetCharacterColumn(int aLine, int aIndex) const = 0;
        virtual int GetLineCharacterCount(int aLine) const = 0;
        virtual int GetLineMaxColumn(int aLine) const = 0;

        virtual void Colorize(int aFromLine = 0, int aCount = -1)= 0;
        virtual void ColorizeRange(int aFromLine = 0, int aToLine = 0)= 0;
        virtual void ColorizeInternal()= 0;
        virtual float TextDistanceToLineStart(const Coordinates& aFrom) const= 0;
        virtual void EnsureCursorVisible()= 0;
        virtual int GetPageSize() const= 0;

        virtual void InsertText(const std::string& aValue) = 0;
        virtual void InsertText(const char* aValue) = 0;
 
        virtual void MoveUp(int aAmount = 1, bool aSelect = false) = 0;
        virtual void MoveDown(int aAmount = 1, bool aSelect = false) = 0;
        virtual void MoveLeft(int aAmount = 1, bool aSelect = false, bool aWordMode = false) = 0;
        virtual void MoveRight(int aAmount = 1, bool aSelect = false, bool aWordMode = false) = 0;
        virtual void MoveTop(bool aSelect = false) = 0;
        virtual void MoveBottom(bool aSelect = false) = 0;
        virtual void MoveHome(bool aSelect = false) = 0;
        virtual void MoveEnd(bool aSelect = false) = 0;

        virtual void SetSelectionStart(const Coordinates& aPosition) = 0;
        virtual void SetSelectionEnd(const Coordinates& aPosition) = 0;
        virtual void SetSelection(const Coordinates& aStart, const Coordinates& aEnd, SelectionMode aMode = SelectionMode::Normal) = 0;
        virtual void SelectWordUnderCursor() = 0;
        virtual void SelectAll() = 0;
        virtual bool HasSelection() const = 0;

        virtual void Copy() = 0;
        virtual void Cut() = 0;
        virtual void Paste() = 0;
        virtual void Delete() = 0;
 
        virtual bool CanUndo() const = 0;
        virtual bool CanRedo() const = 0;
        virtual void Undo(int aSteps = 1) = 0;
        virtual void Redo(int aSteps = 1) = 0;

        virtual void SetText(const std::string& aText) = 0;
        virtual std::string GetText() const = 0;

        virtual void SetCursorPosition(const Coordinates& aPosition) = 0;

        virtual void SetTextLines(const std::vector<std::string>& aLines) = 0;
        virtual std::vector<std::string> GetTextLines() const = 0;

        virtual std::string GetSelectedText() const = 0;
        virtual std::string GetCurrentLineText()const = 0;

        virtual Coordinates SanitizeCoordinates(const Coordinates& aValue) const = 0;
        virtual Coordinates GetActualCursorCoordinates() const = 0;

    private:
        int prev_lineNo = 0;
        unsigned int coordinate_index = 0;

        unsigned int current_index_found_keys = 1; //current index should be starting at 1
        unsigned int total_index_found_keys = 0;
        
        Coordinates curr_coord;
        SearchNavMode nav_mode = SearchNavMode::SearchNaveMode_Down;

        ImFont* TextFont;
        ImFont* DefaultFont;
    private:
        void Show_Find_Replace_Panel(std::string* to_find, std::string* to_replace, unsigned int* panel_height);
    protected:
        Search::KeyInstances_Position found_keys; //For searching
        std::vector<Coordinates> coordinates;
    };
}