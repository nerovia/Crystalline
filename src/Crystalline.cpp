#include "Crystalline.h"

#pragma region Common

char Glyphs::DefaultPadding = ' ';

String Glyphs::DefaultOn = "On";

String Glyphs::DefaultOff = "Off";

char Glyphs::PasswordChar = '*';

char Glyphs::PointerOverRight = '>';

char Glyphs::PointerOverLeft = '<';

char Glyphs::PointerDisabledRight = '-';

char Glyphs::PointerDisabledLeft = '-';

char Glyphs::PointerDownRight = '=';

char Glyphs::PointerDownLeft = '=';

char Glyphs::LoadingBar = '#';

char Glyphs::LinePadding = '.';

char Glyphs::getPointerGlyph(CursorState state)
{
    switch (state)
    {
    case CursorState::PointerOver:
        return PointerOverRight;

    case CursorState::PointerDisabled:
        return PointerDisabledRight;

    case CursorState::PointerDown:
        return PointerDownRight;

    default:
        return DefaultPadding;
    }
}

#pragma endregion

#pragma region Crystalline

void Crystalline::resolveFocus()
{
    auto* focus = getCurrentView()->resolveFocus();
    if (_focus != focus)
    {
        if (_focus != nullptr)
            _focus->invalidate(UIFlag::FocusLost);

        _focus = focus;
        _token = FocusToken();

        if (_focus != nullptr)
            _focus->invalidate(UIFlag::FocusGot);
    }
}

void Crystalline::showCore(UILayout& overlay)
{
    if (_overlay != &overlay)
        hideCore();
    _overlay = &overlay;
}

void Crystalline::hideCore()
{
    _overlay = nullptr;
}

uint8_t Crystalline::getWidth()
{
    return _printer->width;
}

uint8_t Crystalline::getHeight()
{
    return _printer->height;
}

bool Crystalline::hasOverlay()
{
    return _overlay != nullptr;
}

UILayout* Crystalline::getRoot()
{
    return _root;
}

UILayout* Crystalline::getOverlay()
{
    return _overlay;
}

UILayout* Crystalline::getCurrentView()
{
    return _overlay != nullptr ? _overlay : _root;
}

UIElement* Crystalline::getCurrentFocus()
{
    return _focus;
}

void Crystalline::invalidateView()
{
    invalidateFocus();
    _globalDrawFlag = true;
}

void Crystalline::invalidateFocus()
{
    _resolveFocusFlag = true;
}

bool Crystalline::requestToken(const UIElement& element, FocusToken*& out)
{
    if (_focus == &element)
    {
        out = &_token;
        return true;
    }
    else
    {
        out = nullptr;
        return false;
    }
}

bool Crystalline::isFocused(const UIElement& element)
{
    return _focus == &element;
}

void Crystalline::navigate(UILayout& root, bool reset)
{
    if (_root != &root)
    {
        hideCore();
        _root = &root;
        invalidateView();
        if (reset)
            _root->reset();
    }
}

void Crystalline::show(UILayout& overlay, bool reset)
{
    showCore(overlay);
    if (reset)
        overlay.reset();
    invalidateView();
}

void Crystalline::hide()
{
    hideCore();
    invalidateView();
}

void Crystalline::begin(PrinterBase* printer, UILayout& root)
{
    _printer = printer;
    _content = Array<UIContent*>::ofSize(getHeight(), nullptr);
    navigate(root);
    invalidateView();
}

void Crystalline::end()
{
    _printer = nullptr;
    _root = nullptr;
    _overlay = nullptr;
    _focus = nullptr;
    _content = Array<UIContent*>();
}

void Crystalline::update()
{
    auto globalDrawFlag = _globalDrawFlag;
    auto resolveFocusFlag = _resolveFocusFlag;
    _globalDrawFlag = false;
    _resolveFocusFlag = false;

    if (resolveFocusFlag)
        resolveFocus();

    _token.update();

    auto* view = getCurrentView();
    
    view->update();
    for (auto* content : _content)
        if (content != nullptr)
            content->update();

    view->draw(Range(0, getHeight() - 1), globalDrawFlag);
    for (int i = 0; i < _content.length(); i++)
        if (_content[i] != nullptr)
            _content[i]->draw(*_printer->begin(i), globalDrawFlag);

}

void Crystalline::interact(const Interaction& interaction)
{
    getCurrentView()->interact(interaction);
}

void Crystalline::draw(int row, UIContent& content)
{
    if (_content[row] == &content)
        return;
    content.invalidate(UIFlag::GlobalDraw);
    _content[row] = &content;
}

DrawContext& Crystalline::draw(int row)
{
    _content[row] = nullptr;
    return *_printer->begin(row);
}

UILayout* Crystalline::_root = nullptr;

UILayout* Crystalline::_overlay = nullptr;

UIElement* Crystalline::_focus = nullptr;

bool Crystalline::_globalDrawFlag = false;

bool Crystalline::_resolveFocusFlag = false;

FocusToken Crystalline::_token = FocusToken();

PrinterBase* Crystalline::_printer = nullptr;

Array<UIContent*> Crystalline::_content;

#pragma endregion
