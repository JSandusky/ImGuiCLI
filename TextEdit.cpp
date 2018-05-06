#include "TextEdit.h"

#include "TextEditor.h"

using namespace System;
using namespace System::Runtime::InteropServices;

inline std::string ToSTLString(System::String^ str)
{
    // need to do this because ImGui works with UTF-8, C# works with UTF-16, meshes up international characters and Font-Awesome
    array<Byte>^ bytes = System::Text::Encoding::UTF8->GetBytes(str);
    std::string stlStr;
    stlStr.resize(bytes->Length);
    Marshal::Copy(bytes, 0, IntPtr((void*)stlStr.data()), bytes->Length);
    return stlStr;
}

namespace ImGuiCLI
{

    TextEditor::TextEditor()
    {
        editor_ = new ::TextEditor();
        editor_->SetLanguageDefinition(::TextEditor::LanguageDefinition::CPlusPlus());
    }
    TextEditor::~TextEditor()
    {
        delete editor_;
    }

    System::String^ TextEditor::Text::get()
    {
        return gcnew System::String(editor_->GetText().c_str());
    }
    void TextEditor::Text::set(System::String^ txt)
    {
        editor_->SetText(ToSTLString(txt));
    }
    System::String^ TextEditor::SelectedText::get()
    {
        return gcnew System::String(editor_->GetSelectedText().c_str());
    }

    bool TextEditor::IsReadOnly::get() { return editor_->IsReadOnly(); }
    void TextEditor::IsReadOnly::set(bool value) { editor_->SetReadOnly(value); }

    void TextEditor::SetLanguage(TextEditorLang l)
    {
        switch (l)
        {
        case TextEditorLang::CPP:
            editor_->SetLanguageDefinition(::TextEditor::LanguageDefinition::CPlusPlus());
            break;
        case TextEditorLang::GLSL:
            editor_->SetLanguageDefinition(::TextEditor::LanguageDefinition::GLSL());
            break;
        case TextEditorLang::HLSL:
            editor_->SetLanguageDefinition(::TextEditor::LanguageDefinition::HLSL());
            break;
        case TextEditorLang::CClassic:
            editor_->SetLanguageDefinition(::TextEditor::LanguageDefinition::C());
            break;
        case TextEditorLang::Lua:
            editor_->SetLanguageDefinition(::TextEditor::LanguageDefinition::Lua());
            break;
        case TextEditorLang::Angelscript:
            editor_->SetLanguageDefinition(::TextEditor::LanguageDefinition::AngelScript());
            break;
        }
    }
    void TextEditor::Render(System::String^ title, Vector2 size, bool border)
    {
        editor_->Render(ToSTLString(title).c_str(), ImVec2(size.X, size.Y), border);
    }

}