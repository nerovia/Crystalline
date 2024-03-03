#include "Controls.h"


#pragma region Control

void Control::onValidate()
{
	FocusToken* token;
	if (!isInteractable() && requestToken(token))
		invalidate(token->cursor, CursorState::PointerDisabled, UIFlag::CursorChanged);
}

void Control::onDraw(DrawContext& context)
{
	FocusToken* token;
	if (requestToken(token))
	{
		if (!context.omit(2, isDirty(UIFlag::CursorChanged | UIFlag::FocusChanged)))
		{
			context.write(Glyphs::getPointerGlyph(token->cursor));
			context.write(' ');
		}
	}
}

bool Control::onInteract(const Interaction& e)
{
	if (!isInteractable())
		return false;

	if (e.state != KeyState::Down)
		return true;


	FocusToken* token;
	if (requestToken(token))
	{
		switch (token->state)
		{
		case FocusState::Normal:
			if (e.key != KeyCode::Enter)
				return false;
			token->state = FocusState::Engaged;
			invalidate(token->cursor, CursorState::PointerDown, UIFlag::CursorChanged);
			return true;

		case FocusState::Engaged:
			if (e.key != KeyCode::Enter && e.key != KeyCode::Escape)
				return false;
			token->state = FocusState::Normal;
			invalidate(token->cursor, CursorState::PointerOver, UIFlag::CursorChanged);
			return true;
		}
	}

	return false;
}

#pragma endregion

#pragma region ButtonControl

void ButtonControl::onClick()
{
	if (handler != nullptr)
		handler->invoke();
}

void ButtonControl::onDraw(DrawContext& context)
{
	FocusToken* token;
	bool isPressed = requestToken(token) && token->state == FocusState::Pressed;
	
	Control::onDraw(context);
	
	context.write(isPressed ? '(' : '[');
	context.write(content);
	context.write(isPressed ? ')' : ']');
	context.fill();
}

bool ButtonControl::onInteract(const Interaction& e)
{
	FocusToken* token;
	if (!requestToken(token))
		return false;

	switch (token->state)
	{
	case FocusState::Normal:
		if (isInteractable() && e.equals(KeyCode::Enter, KeyState::Down))
		{
			invalidate(token->state, FocusState::Pressed, UIFlag::StateChanged);
			return true;
		}
		break;
		
	case FocusState::Pressed:
		if (e.equals(KeyCode::Enter, KeyState::Up))
		{
			onClick();
			invalidate(token->state, FocusState::Normal, UIFlag::StateChanged);
			return true;
		}
		break;
	}

	return false;
}

bool ButtonControl::isInteractable() const
{
	return isEnabled && handler != nullptr;
}

ButtonControl::ButtonControl() : ButtonControl("", nullptr)
{
}

ButtonControl::ButtonControl(String content, Action* handler, bool isEnabled)
{
	this->content = content;
	this->handler = handler;
	this->isEnabled = isEnabled;
}

#pragma endregion

#pragma region LabelControl

void LabelControl::onDraw(DrawContext& context) 
{
	Control::onDraw(context);
	context.write(content);
	context.fill();
}

LabelControl::LabelControl() : LabelControl(content)
{

}

LabelControl::LabelControl(String content)
{
	this->content = content;
}

bool LabelControl::isInteractable() const
{
	return false;
}

#pragma endregion

#pragma region DataControl

#pragma endregion

#pragma region NumberControl


template<>
void NumberControl<float>::onDrawContent(DrawContext& context)
{
	if (isDirty(UIFlag::PropertyChanged | UIFlag::FocusChanged))
	{

		String s = String(content->get(), 1);
		if (s == "-0.0")
			s = "0.0";

		auto spacing = suffix.length() > 0 ? suffix.length() + 1 : 0;
		context.write(s, Alignment::Back, context.getRemaining() - spacing, Glyphs::LinePadding);
	}

	if (isDirty(UIFlag::GlobalDraw))
	{
		if (suffix.length() > 0)
			context.write(' ');
		context.write(suffix);
	}
}

template<>
void NumberControl<float>::onManipulate(int sign, KeyState state)
{
	if (state == KeyState::Down || state == KeyState::Pressed)
		content->set(content->get() + sign * 0.1f);
}

template<>
void NumberControl<int>::onDrawContent(DrawContext& context)
{
	if (isDirty(UIFlag::PropertyChanged | UIFlag::FocusChanged))
	{
		auto spacing = suffix.length() > 0 ? suffix.length() + 1 : 0;
		context.write(String(content->get()), Alignment::Back, context.getRemaining() - spacing, Glyphs::LinePadding);
	}

	if (isDirty(UIFlag::GlobalDraw))
	{
		if (suffix.length() > 0)
			context.write(' ');
		context.write(suffix);
	}
}

template<class T>
void NumberControl<T>::onManipulate(int sign, KeyState state) { }

template<class T>
void NumberControl<T>::onDrawContent(DrawContext& context) { }

template<>
void NumberControl<int>::onManipulate(int sign, KeyState state)
{
	if (state == KeyState::Down || state == KeyState::Pressed)
		content->set(content->get() + sign);
}


#pragma endregion

#pragma region SwitchControl

void SwitchControl::onDrawContent(DrawContext& context)
{
	if (isDirty(UIFlag::PropertyChanged | UIFlag::FocusChanged))
	{
		auto value = content->get();
		auto s = value >= 0 && value < options.length() ? options[value] : String(value);
		context.fill(s, Alignment::Back, Glyphs::LinePadding);
	}
}

void SwitchControl::onManipulate(int sign, KeyState state)
{
	if (state == KeyState::Down)
		content->set(clamp(content->get() + sign, 0, options.length() - 1));
}

SwitchControl::SwitchControl() : SwitchControl("", Array<String>(), nullptr)
{
}

SwitchControl::SwitchControl(String header, Array<String> options, Property<int>* content)
{
	this->header = header;
	this->options = options;
	this->content = content;
}

#pragma endregion
