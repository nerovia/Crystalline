#include "Panels.h"

#pragma	region MenuLayout

void MenuLayout::onUpdate()
{
	handleUpdate(selectedPanel());
}

void MenuLayout::onDraw(Range rows)
{
	handleDraw(selectedPanel(), rows);
}

bool MenuLayout::onInteract(const Interaction& e)
{
	if (e.state == KeyState::Up)
		return false;
	switch (e.key)
	{
	case KeyCode::LeftArrow:
		setSelection(_selection - 1);
		return true;

	case KeyCode::RightArrow:
		setSelection(_selection + 1);
		return true;

	default:
		return false;
	}
}

void MenuLayout::onReset()
{
	setSelection(0);
}

UIElement* MenuLayout::focusSource() const
{
	return _selection <= -1 ? nullptr : selectedPanel();
}

MenuLayout::MenuLayout() : MenuLayout(Array<MenuPanel*>())
{
}

MenuLayout::MenuLayout(Array<MenuPanel*> panels) : panels(panels), _selection(0)
{
}

MenuPanel* MenuLayout::selectedPanel() const
{
	return panels[_selection];
}

int8_t MenuLayout::getSelection() const
{
	return _selection;
}

void MenuLayout::setSelection(int8_t value)
{
	if (value < 0)
		value = panels.length() - 1;
	else if (value >= panels.length())
		value = 0;
	if (invalidate(_selection, value, UIFlag::PropertyChanged))
		handleFocus(true, true);
}

#pragma endregion

#pragma region MenuPanel

void MenuPanel::onDrawHeader(DrawContext& context)
{
	context.fill(isFocused() ? 
		(Glyphs::PointerDownLeft + header + Glyphs::PointerDownRight) : 
		(Glyphs::PointerOverLeft + header + Glyphs::PointerOverRight), Alignment::Center);
}

void MenuPanel::onDrawContent(Range rows)
{
	for (uint8_t i = 0; i < rows.length(); i++)
		Crystalline::draw(i).fill();
}

void MenuPanel::onDraw(Range rows)
{
	if (isDirty(UIFlag::FocusChanged))
		onDrawHeader(Crystalline::draw(rows.start));
	onDrawContent(rows.withMargin(1, 0));
}

#pragma endregion

#pragma region ControlPanel

void ControlPanel::onReset()
{
	setSelection(-1);
}

UIElement* ControlPanel::focusSource() const
{
	if (_selection < 0)
		return nullptr;
	return selectedControl();
}

void ControlPanel::onDrawContent(Range rows)
{
	auto s = max(_selection, 0);
	auto offset = _offset;

	if (_offset > s)
		offset = s;
	else if (_offset + rows.length() - 1 <= s)
		offset = s - rows.length() + 1;

	if (offset == _offset && !isDirty(UIFlag::GlobalDraw))
		return;

	_offset = offset;

	for (int i = rows.start, n = 0; i <= rows.end; i++)
	{
		if (_offset + n < controls.length())
			Crystalline::draw(i, *controls[_offset + n++]);
		else
			Crystalline::draw(i).fill();
	}
}

bool ControlPanel::onInteract(const Interaction& e)
{
	if (e.state == KeyState::Up)
		return false;

	switch (e.key)
	{
	case KeyCode::Escape:
		reset();
		break;

	case KeyCode::DownArrow:
		setSelection(_selection + 1);
		return true;

	case KeyCode::UpArrow:
		setSelection(_selection - 1);
		return true;

	default:
		return _selection >= 0;
	}
}

ControlPanel::ControlPanel() : ControlPanel("", Array<Control*>())
{
}

ControlPanel::ControlPanel(String header, Array<Control*> controls)
{
	this->header = header;
	this->controls = controls;
	_selection = -1;
}

Control* ControlPanel::selectedControl() const
{
	return controls[_selection];
}

int8_t ControlPanel::getSelection() const
{
	return _selection;
}

void ControlPanel::setSelection(int8_t value)
{
	if (invalidate(_selection, (int8_t)clamp(value, -1, controls.length() - 1), UIFlag::PropertyChanged))
		handleFocus();
}

#pragma endregion

#pragma region NavigationPanel

void NavigationPanel::onClick()
{
	if (handler != nullptr)
		handler->invoke();
}

void NavigationPanel::onDrawContent(Range rows)
{
	auto body = rows.withLength(1, Alignment::Center);
	for (int i = rows.start; i < body.end; i++)
		Crystalline::draw(i).fill();
	for (int i = body.start; i <= body.end; i++)
		Crystalline::draw(i).fill("...", Alignment::Center);
	for (int i = body.end + 1; i <= rows.end; i++)
		Crystalline::draw(i).fill();
}

bool NavigationPanel::onInteract(const Interaction& e)
{
	bool result;
	if (result = e.equals(KeyCode::Enter, KeyState::Down))
		onClick();
	return result;
}

NavigationPanel::NavigationPanel() : NavigationPanel("", nullptr)
{
}

NavigationPanel::NavigationPanel(String header, Action* handler)
{
	this->header = header;
	this->handler = handler;
}

#pragma endregion
