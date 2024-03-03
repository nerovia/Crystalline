#pragma once

#include "Arduino.h"
#include "Delegate.h"

class Control;
class ButtonControl;
class LabelControl;
template<class T> class DataControl;
template<class T> class NumberControl;
class SwitchControl;
template<String& ON, String& OFF> class ToggleControl;

#include "Crystalline.h"

class Control : public UIContent 
{
protected:
	enum State : uint8_t {
		Normal,
		Selected,
	};

	void onValidate() override;
	void onDraw(DrawContext& context) override;
	bool onInteract(const Interaction& interaction);
public:
	virtual bool isInteractable() const = 0;
};

class ButtonControl : public Control
{
protected:
	virtual void onClick();
	void onDraw(DrawContext& context) override;
	bool onInteract(const Interaction& interaction) override;

public:
	ButtonControl();
	ButtonControl(String content, Action* handler = nullptr, bool isEnabled = true);

	bool isEnabled = true;
	String content;
	Action* handler;
	
	bool isInteractable() const override;
};

class LabelControl : public Control
{
protected:
	void onDraw(DrawContext& context) override;

public:
	LabelControl();
	LabelControl(String content);

	bool isInteractable() const override;

	String content;
};

template<class T>
class DataControl : public Control
{

public:
	bool isEnabled = true;
	String header = "";
	Property<T>* content = nullptr;

	bool isInteractable() const override
	{
		return isEnabled && !content->isReadonly();
	}

protected:
	T _lastValue = -1;

	virtual void onDrawContent(DrawContext& context) = 0;
	virtual void onManipulate(int sign, KeyState state) = 0;
	void onUpdate() override
	{
		invalidate(_lastValue, content->get(), UIFlag::PropertyChanged);
	}
	void onDraw(DrawContext& context) override
	{
		Control::onDraw(context);
		if (!context.omit(header.length(), isDirty(UIFlag::FocusChanged)))
			context.write(header);
		onDrawContent(context);
	}
	bool onInteract(const Interaction& e) override
	{
		if (Control::onInteract(e))
			return true;

		FocusToken* token;
		if (requestToken(token) && token->state == FocusState::Engaged)
		{
			switch (e.key)
			{
			case KeyCode::LeftArrow:
			case KeyCode::DownArrow:
				onManipulate(-1, e.state);
				return true;

			case KeyCode::RightArrow:
			case KeyCode::UpArrow:
				onManipulate(1, e.state);
				return true;;
			}
		}

		return false;
	}


};

template<class T>
class NumberControl : public DataControl<T>
{
public:
	String suffix;

	NumberControl() : NumberControl<T>("", "", nullptr)
	{
	}

	NumberControl(String header, String suffix, Property<T>* content)
	{
		this->header = header;
		this->suffix = suffix;
		this->content = content;
	}

	void onDrawContent(DrawContext& context) override;
	void onManipulate(int sign, KeyState state) override;
};

class SwitchControl : public DataControl<int>
{
protected:
	void onDrawContent(DrawContext& context) override;
	void onManipulate(int sign, KeyState state) override;

public:
	SwitchControl();
	SwitchControl(String header, Array<String> options, Property<int>* content);

	Array<String> options;
};

template<String& ON = Glyphs::DefaultOn, String& OFF = Glyphs::DefaultOff>
class ToggleControl : public DataControl<bool>
{
protected:
	void onDrawContent(DrawContext& context) override
	{
		if (isDirty(UIFlag::PropertyChanged | UIFlag::FocusChanged))
		{
			context.fill(content->get() ? ON : OFF, Alignment::Back, Glyphs::LinePadding);
		}
	}

	void onManipulate(int sign, KeyState state) override
	{
		{
			if (state == KeyState::Down)
				content->set(!content->get());
		}
	}

public:
	ToggleControl() : ToggleControl<ON, OFF>("", nullptr) { }
	ToggleControl(String header, Property<bool>* content)
	{
		this->header = header;
		this->content = content;
	}
};
