#include "Crystalline.h"

#pragma region UIElement

bool UIElement::isDirty(UIFlag flag) const
{
    return (_flags & (flag | UIFlag::GlobalDraw)) > UIFlag::None;
}

bool UIElement::isFocused() const
{
    return Crystalline::isFocused(*this);
}

UIFlag UIElement::getFlags() const
{
    return _flags;
}

void UIElement::invalidate(UIFlag flag)
{
    _flags |= flag;
}


void UIElement::reset()
{
    invalidate(UIFlag::LocalReset);
    auto* source = focusSource();
    if (source != nullptr)
        source->reset();
    onReset();
}

void UIElement::update()
{
    onUpdate();
}

bool UIElement::interact(const Interaction& interaction)
{
    auto* source = focusSource();
    if (source != nullptr && source->interact(interaction))
        return true;
    return onInteract(interaction);
}

UIElement* UIElement::resolveFocus()
{
    auto* source = focusSource();
    if (source == nullptr)
        return this;
    return source->resolveFocus();
}

bool UIElement::requestToken(FocusToken*& out) const
{
    return Crystalline::requestToken(*this, out);
}

#pragma endregion

#pragma region UILayout


bool UILayout::handleUpdate(UILayout* layout)
{
    if (layout == nullptr)
        return false;
    layout->update();
    if (layout->isDirty())
        invalidate(UIFlag::LayoutChanged);
    return true;
}

bool UILayout::handleDraw(UILayout* layout, Range rows)
{
    if (layout == nullptr)
        return false;
    if (isDirty(UIFlag::LayoutChanged))
        layout->draw(rows, isDirty(UIFlag::GlobalDraw));
    return true;
}

void UILayout::handleFocus(bool redraw, bool reset)
{
    auto* focus = focusSource();
    if (focus == nullptr)
        focus = this;
    if (redraw)
        focus->invalidate(UIFlag::GlobalDraw);
    if (reset)
        focus->reset();
    Crystalline::invalidateFocus();
}

void UILayout::draw(Range rows, bool redraw)
{
    if (rows.length() <= 0)
        return;

    if (redraw)
        invalidate(UIFlag::GlobalDraw);

    if (isDirty())
    {
        onValidate();
        onDraw(rows);
        _flags = UIFlag::None;
    }
}

#pragma endregion

#pragma region UIContent

void UIContent::draw(DrawContext& context, bool redraw)
{
    if (redraw)
        invalidate(UIFlag::GlobalDraw);
    if (isDirty())
    {
        onValidate();
        onDraw(context);
        _flags = UIFlag::None;
    }
}

#pragma endregion
