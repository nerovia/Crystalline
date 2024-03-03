#pragma once

class PopupLayout;
class WarningPopup;
class DialogPopup;
class LockPopup;
class ProgressPopup;
class PopupManager;

#include "Delegate.h"
#include "Crystalline.h"

template<class... TArgs>
using PopupHandler = Delegate<bool, UILayout&, TArgs...>;

class PopupLayout : public UILayout
{
protected:
	PopupLayout(String header = "", int8_t priority = 0);

	void onDraw(Range rows) override;
	bool onInteract(const Interaction& interaction) override;
	virtual void onDrawBorder(DrawContext& context);
	virtual void onDrawContent(DrawContext& context) = 0;
	virtual bool onClose() = 0;

public:
	bool close();

	const int8_t priority;
	String header;
};

class WarningPopup : public PopupLayout
{
protected:
	void onDrawContent(DrawContext& context) override;
	bool onClose() override;
public:
	WarningPopup(String header, String message, PopupHandler<>* handler = nullptr, int8_t priority = 0);
	String message;
	PopupHandler<>* handler;
};

class DialogPopup : public PopupLayout
{
protected:
	bool onInteract(const Interaction& interaction) override;
	bool onClose() override;
	void onDrawContent(DrawContext& context) override;

public:
	DialogPopup(String header, PopupHandler<bool>* handler = nullptr, int8_t priority = 0);
	PopupHandler<bool>* handler;
};

class LockPopup : public PopupLayout
{
protected:
	int _pos;
	char* _input;

	void onReset() override;
	bool onClose() override;
	void onDrawContent(DrawContext& context) override;
	bool onInteract(const Interaction& e);

public:
	LockPopup(String header, int length, PopupHandler<String>* handler = nullptr, bool isMasked = true, int8_t priority = 0);
	~LockPopup();

	bool isMasked;
	const int length;
	PopupHandler<String>* handler;
};

class ProgressPopup : public PopupLayout
{
	float _last;
	bool onInteract(const Interaction& interaction) override;
	bool onClose() override;
	void onDrawContent(DrawContext& context) override;
	void onUpdate() override;

public:
	Getter<float>* source;
	ProgressPopup(String header, Getter<float>* source, int8_t priority = 0);
};

class PopupManager 
{
private:
	static Array<PopupLayout*> _popups;
	static PopupLayout* _current;

public:
	static bool isOpen(PopupLayout& layout)
	{
		for (auto& popup : _popups)
			if (popup == &layout)
				return true;
		return false;
	}

	static bool show(PopupLayout& popup)
	{
		PopupLayout** insert = nullptr;
		PopupLayout* top = &popup;
		for (auto& p : _popups)
		{
			if (p == &popup)
				return false;

			if (p == nullptr)
				insert = &p;
			else if (top->priority <= p->priority)
				top = p;
		}

		if (insert == nullptr)
			return false;
		else
			(*insert) = &popup;

		popup.reset();

		_current = top;
		
		Crystalline::show(*_current, false);
		
		return true;
	}

	static bool hide(PopupLayout& popup)
	{
		PopupLayout** insert = nullptr;
		PopupLayout* top = nullptr;
		for (auto& p : _popups)
		{
			if (p == &popup)
				insert = &p;
			else if (top == nullptr || p != nullptr && p->priority >= top->priority)
				top = p;
		}

		if (insert != nullptr)
			(*insert) = nullptr;

		if (top != nullptr && Crystalline::getOverlay() == _current)
			Crystalline::show(*top, false);
		else if (Crystalline::getOverlay() == &popup)
			Crystalline::hide();
		
		_current = top;

		return true;
	}
};
