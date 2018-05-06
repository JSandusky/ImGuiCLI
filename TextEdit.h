#pragma once

class TextEditor;

using namespace Microsoft::Xna::Framework;

namespace ImGuiCLI
{

    public enum class TextEditorLang
    {
        GLSL,
        HLSL,
        CPP,
        CClassic,
        Lua,
        Angelscript
    };

    public ref class TextEditor
    {
    public:
        TextEditor();
        ~TextEditor();

        property System::String^ Text { System::String^ get(); void set(System::String^); }
        property System::String^ SelectedText { System::String^ get(); }
        property bool IsReadOnly { bool get(); void set(bool); }

        void SetLanguage(TextEditorLang);
        void Render(System::String^ title, Vector2 size, bool border);

    private:
        ::TextEditor* editor_;
    };

}