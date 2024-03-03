#pragma once

#include "Arduino.h"
#include "Array.h"

#define clamp(value, minValue, maxValue) (max(minValue, min(maxValue, value)))

#pragma region Enums

enum class UIFlag : uint8_t
{
	None = 0x00,
	PropertyChanged = 0x01,
	CursorChanged = 0x02,
	StateChanged = 0x04,
	LayoutChanged = 0x08,
	FocusLost = 0x10,
	FocusGot = 0x20,
	FocusChanged = 0x30,
	LocalReset = 0x40,
	GlobalDraw = 0x80,
	Any = 0xFF,
};

inline UIFlag operator | (UIFlag lhs, UIFlag rhs)
{
	return UIFlag(uint8_t(lhs) | uint8_t(rhs));
}

inline UIFlag& operator |= (UIFlag& lhs, UIFlag rhs)
{
	lhs = lhs | rhs;
	return lhs;
}

inline UIFlag operator & (UIFlag lhs, UIFlag rhs)
{
	return UIFlag(uint8_t(lhs) & uint8_t(rhs));
}

inline UIFlag& operator &= (UIFlag& lhs, UIFlag rhs)
{
	lhs = lhs & rhs;
	return lhs;
}

enum class KeyState : uint8_t {
	Down,
	Up,
	Pressed,
};

enum class KeyCode {
	Enter = 13,
	Escape = 27,
	LeftArrow = 37,
	UpArrow = 38,
	RightArrow = 39,
	DownArrow = 40,
};

inline String operator + (KeyCode lhs, KeyCode rhs)
{
	String str = "";
	return str + (char)lhs + (char)rhs;
}

inline String operator + (String lhs, KeyCode rhs)
{
	return lhs + (char)rhs;
}

enum class Alignment : uint8_t
{
	Front,
	Center,
	Back,
};

enum class FocusState : uint8_t {
	Normal = 0,
	Disabled,
	Engaged,
	Pressed,
};

enum class CursorState : uint8_t {
	PointerOver,
	PointerDisabled,
	PointerDown,
};

#pragma endregion

#pragma region Common

struct Range
{
	const int start;

	const int end;

	Range(int start, int end) : start(start), end(max(start, end)) { }

	int length() const { return max(end - start + 1, 0); }

	Range withMargin(int start, int end) const { return Range(this->start + start, this->end - end); }

	Range withLength(int length, Alignment alignment) const {
		int d = this->length() - length;
		switch (alignment)
		{
		case Alignment::Front:
			return withMargin(0, d);
		case Alignment::Center:
			return withMargin(d / 2, d - (d / 2));
		case Alignment::Back:
			return withMargin(d, 0);
		}
	}
};

class Timer
{
private:
	uint8_t _timestamp = 0;

public:
	uint8_t elapsed() { return millis() - _timestamp; }
	bool hasElapsed(uint8_t interval) { return elapsed() >= interval; }
	void reset() { _timestamp = millis(); }
};

struct FocusToken
{
	CursorState cursor = CursorState::PointerOver;
	FocusState state = FocusState::Normal;
	Timer timer = Timer();
	void update() { }
};

struct Interaction
{
	const KeyCode key;
	const KeyState state;

	Interaction(KeyCode key, KeyState state) : key(key), state(state) { }

	bool equals(KeyCode key, KeyState state) const { return this->key == key && this->state == state; }
};

struct Glyphs
{
	static char DefaultPadding;

	static String DefaultOn;

	static String DefaultOff;

	static char PasswordChar;

	static char PointerOverRight;

	static char PointerOverLeft;

	static char PointerDisabledRight;

	static char PointerDisabledLeft;

	static char PointerDownRight;

	static char PointerDownLeft;

	static char LinePadding;

	static char LoadingBar;

	static char getPointerGlyph(CursorState state);
};

#pragma endregion

#pragma region Printing

class DrawContext
{
public:
	virtual uint8_t getRemaining() const = 0;
	virtual uint8_t getPosition() const = 0;
	virtual uint8_t getTotal() const = 0;

	virtual void write(char c) = 0;
	virtual void write(String s) = 0;
	virtual void write(String s, Alignment alignment, uint8_t total, char padding = Glyphs::DefaultPadding) = 0;
	virtual void fill(char c = Glyphs::DefaultPadding) = 0;
	virtual void fill(char prefix, char infix, char postfix) = 0;
	virtual void fill(String s, Alignment aligment = Alignment::Front, char padding = Glyphs::DefaultPadding) = 0;
	virtual void repeat(char c, uint8_t count) = 0;
	virtual void repeat(char prefix, char infix, char postfix, uint8_t count) = 0;
	virtual bool omit(uint8_t count, bool check) = 0;
};

class PrinterBase : public DrawContext
{
protected:
	uint8_t posX, posY, virtualX, virtualY;

	PrinterBase(uint8_t width, uint8_t height);

	virtual bool printCore(char c) = 0;
	virtual bool moveCore(uint8_t x, uint8_t y) = 0;

	void print(char c);
	bool ensureMove();
	bool move(uint8_t x, uint8_t y);
	void virtualMove(uint8_t x, uint8_t y);

public:
	const uint8_t width;
	const uint8_t height;

	uint8_t getRemaining() const override;
	uint8_t getPosition() const override;
	uint8_t getTotal() const override;

	DrawContext* begin(int row);

	void write(char c) override;
	void write(String s) override;
	void write(String s, Alignment alignment, uint8_t total, char padding = Glyphs::DefaultPadding) override;
	void fill(char c = Glyphs::DefaultPadding) override;
	void fill(char prefix, char infix, char postfix) override;
	void fill(String s, Alignment aligment, char padding = Glyphs::DefaultPadding) override;
	void repeat(char c, uint8_t count) override;
	void repeat(char prefix, char infix, char postfix, uint8_t count) override;
	bool omit(uint8_t count, bool check) override;
};

#pragma endregion

#pragma region UIBase

class UIElement
{
protected:
	UIFlag _flags = UIFlag::None;

	virtual UIElement* focusSource() const { return nullptr; }

	virtual void onReset() { }
	virtual void onUpdate() { }
	virtual void onValidate() { }
	virtual bool onInteract(const Interaction& interaction) { return false; }

public:
	bool isDirty(UIFlag flag = UIFlag::Any) const;
	bool isFocused() const;
	UIFlag getFlags() const;


	void invalidate(UIFlag flag);
	template<class T> bool invalidate(T& field, T value, UIFlag flag)
	{
		if (field == value)
			return false;
		field = value;
		invalidate(flag);
		return true;
	}

	void reset();
	void update();
	bool interact(const Interaction& interaction);

	UIElement* resolveFocus();
	bool requestToken(FocusToken*& out) const;
};

class UILayout : public UIElement
{
protected:
	virtual void onDraw(Range rows) { }

	bool handleUpdate(UILayout* layout);
	bool handleDraw(UILayout* layout, Range rows);
	void handleFocus(bool redraw = false, bool reset = false);

public:
	void draw(Range rows, bool redraw = false);
};

class UIContent : public UIElement
{
protected:
	virtual void onDraw(DrawContext& context) { }

public:
	void draw(DrawContext& context, bool redraw = false);
};

#pragma endregion

class Crystalline
{
private:
	static UILayout* _root;
	static UILayout* _overlay;
	static UIElement* _focus;
	static bool _globalDrawFlag;
	static bool _resolveFocusFlag;
	static FocusToken _token;
	static PrinterBase* _printer;
	static Array<UIContent*> _content;

	static void resolveFocus();
	static void showCore(UILayout& overlay);
	static void hideCore();

public:
	static uint8_t getWidth();
	static uint8_t getHeight();
	static bool hasOverlay();
	static UILayout* getRoot();
	static UILayout* getOverlay();
	static UILayout* getCurrentView();
	static UIElement* getCurrentFocus();

	static void invalidateView();
	static void invalidateFocus();
	static bool requestToken(const UIElement& element, FocusToken*& out);
	static bool isFocused(const UIElement& element);
	static void navigate(UILayout& root, bool reset = true);
	static void show(UILayout& overlay, bool reset = true);
	static void hide();
	static void begin(PrinterBase* printer, UILayout& root);
	static void end();
	static void update();
	static void interact(const Interaction& interaction);
	static void draw(int row, UIContent& content);
	static DrawContext& draw(int draw);
};

#include "Panels.h"
#include "Controls.h"
#include "Popups.h"
