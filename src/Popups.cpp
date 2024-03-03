#include "Popups.h"

#pragma region PopupLayout

inline PopupLayout::PopupLayout(String header, int8_t priority) : header(header), priority(priority)
{
}

void PopupLayout::onDraw(Range rows)
{
	if (isDirty(UIFlag::FocusChanged))
	{
		if (rows.length() >= 4)
		{
			for (int i = rows.start; i <= rows.end; i++)
			{
				auto& context = Crystalline::draw(i);
				if (i == max(rows.length() / 2 - 1, 0))
					context.fill(header, Alignment::Center);
				else if (i == max(rows.length() / 2, 1))
					onDrawContent(context);
				else if (i == rows.start || i == rows.end)
					onDrawBorder(context);
				else
					context.fill(' ');
			}
		}
		else if (rows.length() >= 2)
		{
			Crystalline::draw(rows.start).fill(header, Alignment::Center);
			onDrawContent(Crystalline::draw(rows.end));
		}
		else
		{
			auto& context = Crystalline::draw(rows.start);
			context.write(header);
			context.write(' ');
			onDrawContent(context);
		}
	}
	else
	{
		if (rows.length() >= 4)
			onDrawContent(Crystalline::draw(max(rows.length() / 2, 1)));
		else if (rows.length() >= 2)
			onDrawContent(Crystalline::draw(rows.end));
		else
		{
			auto& context = Crystalline::draw(rows.start);
			context.omit(header.length() + 1, true);
			onDrawContent(context);
		}
	}
}

bool PopupLayout::onInteract(const Interaction& e)
{
	if (e.equals(KeyCode::Enter, KeyState::Up))
	{
		if (onClose())
			close();
		return true;
	}
	else if (e.equals(KeyCode::Escape, KeyState::Up))
	{
		close();
		return true;
	}
	return false;
}

void PopupLayout::onDrawBorder(DrawContext& context)
{
	context.fill('o', '=', 'o');
}

bool PopupLayout::close()
{
	PopupManager::hide(*this);

	return true;
}

#pragma endregion

#pragma region WarningPopup

void WarningPopup::onDrawContent(DrawContext& context)
{
	context.fill(message, Alignment::Center);
}

bool WarningPopup::onClose()
{
	if (handler != nullptr)
		return handler->invoke(*this);
	return true;
}

WarningPopup::WarningPopup(String header, String message, PopupHandler<>* handler, int8_t priority) :
	PopupLayout(header, priority), handler(handler), message(message)
{
}

#pragma endregion

#pragma region DialogPopup

bool DialogPopup::onInteract(const Interaction& e)
{
	FocusToken* token;
	if (requestToken(token))
	{
		if (e.equals(KeyCode::LeftArrow, KeyState::Down))
		{
			invalidate(token->state, FocusState::Normal, UIFlag::StateChanged);
			return true;
		}
		else if (e.equals(KeyCode::RightArrow, KeyState::Down))
		{
			invalidate(token->state, FocusState::Pressed, UIFlag::StateChanged);
			return true;
		}
		
		return PopupLayout::onInteract(e);
	}

	return false;
}

bool DialogPopup::onClose()
{
	FocusToken* token;
	if (requestToken(token))
		if (handler != nullptr)
			return handler->invoke(*this, token->state == FocusState::Pressed);
	return true;
}

void DialogPopup::onDrawContent(DrawContext& context)
{
	FocusToken* token;
	String s = Glyphs::PointerDownLeft + (requestToken(token) && token->state == FocusState::Pressed ? " Yes " : " No ") + Glyphs::PointerDownRight;
	context.fill(s, Alignment::Center);
}

DialogPopup::DialogPopup(String header, PopupHandler<bool>* handler, int8_t priority) :
	PopupLayout(header, priority), handler(handler)
{
	
}

#pragma endregion

#pragma region LockPopup

void LockPopup::onReset()
{
	invalidate(_pos, 0, UIFlag::PropertyChanged);
}

bool LockPopup::onClose()
{
	if (handler != nullptr)
	{
		if (_pos < length)
			return handler->invoke(*this, "");
		else
			return handler->invoke(*this, String(_input));
	}
	return true;
}

void LockPopup::onDrawContent(DrawContext& context)
{
	auto start = (context.getRemaining() - length) / 2;
	if (!context.omit(start, isDirty(UIFlag::FocusChanged)))
		context.repeat(' ', start);

	for (int i = 0; i < length; i++)
		context.write(i < _pos ? (isMasked ? Glyphs::PasswordChar : _input[i]) : '-');
	
	if (isDirty(UIFlag::FocusChanged))
		context.fill();
}

bool LockPopup::onInteract(const Interaction& e)
{
	if (PopupLayout::onInteract(e))
	{
		reset();
		return true;
	}
	else if (e.state != KeyState::Down)
	{
		switch (e.key)
		{
		case KeyCode::Escape:
			if (_pos > 0)
				invalidate(_pos, _pos - 1, UIFlag::PropertyChanged);
			break;

		default:
			if (_pos < length)
			{
				_input[_pos] = (char)e.key;
				invalidate(_pos, _pos + 1, UIFlag::PropertyChanged);
			}
			break;
		}
	}
	return true; // Always consume.
}

LockPopup::LockPopup(String header, int length, PopupHandler<String>* handler, bool isMasked, int8_t priority) :
	PopupLayout(header, priority), handler(handler), length(length), isMasked(isMasked)
{
	_input = new char[length];
	_pos = 0;
}

LockPopup::~LockPopup()
{
	delete[] _input;
	_input = nullptr;
}

#pragma endregion

#pragma region ProgressPopup

bool ProgressPopup::onInteract(const Interaction& interaction) 
{
	return true;
}

void ProgressPopup::onUpdate()
{
	invalidate(_last, source->invoke(), UIFlag::PropertyChanged);
	FocusToken* token;
	if (requestToken(token))
	{
		Serial.println(_last);
		if (_last < 1.0f)
			token->timer.reset();
		else if (token->timer.hasElapsed(1000))
			close();
	}
}

bool ProgressPopup::onClose() { return true; }

void ProgressPopup::onDrawContent(DrawContext& context)
{
	int threshold = context.getRemaining() * source->invoke();
	context.repeat(Glyphs::LoadingBar, threshold);
	context.fill();
}

ProgressPopup::ProgressPopup(String header, Getter<float>* source, int8_t priority = 0) : source(source), PopupLayout(header, priority) { }

#pragma endregion

#pragma region PopupManager

Array<PopupLayout*> PopupManager::_popups = Array<PopupLayout*>::ofSize(8, nullptr);

PopupLayout* PopupManager::_current = nullptr;

#pragma endregion
