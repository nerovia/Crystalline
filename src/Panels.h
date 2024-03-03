#pragma once

class MenuLayout;
class MenuPanel;
class ControlPanel;
class NavigationPanel;

#include "Arduino.h"
#include "Array.h"
#include "Delegate.h"
#include "Controls.h"
#include "Crystalline.h"

class MenuLayout : public UILayout 
{
private:
	int8_t _selection;

protected:
	void onUpdate() override;
	void onDraw(Range rows) override;
	bool onInteract(const Interaction& interaction) override;
	void onReset() override;
	UIElement* focusSource() const override;
	
public:
	MenuLayout();
	MenuLayout(Array<MenuPanel*> panels);

	Array<MenuPanel*> panels;

	MenuPanel* selectedPanel() const;
	int8_t getSelection() const;
	void setSelection(int8_t value);
};

class MenuPanel : public UILayout
{
protected:
	virtual void onDrawHeader(DrawContext& context);
	virtual void onDrawContent(Range rows);
	virtual void onDraw(Range rows) override;

public:
	String header;
};

class ControlPanel : public MenuPanel
{
private:
	int8_t _selection = -1;
	int8_t _offset = 0;

protected:
	void onReset() override;
	UIElement* focusSource() const override;
	void onDrawContent(Range rows) override;
	bool onInteract(const Interaction& interaction) override;

public:
	ControlPanel();
	ControlPanel(String header, Array<Control*> controls);

	Array<Control*> controls;

	Control* selectedControl() const;
	int8_t getSelection() const;
	void setSelection(int8_t value);

};

class NavigationPanel : public MenuPanel
{
protected:
	virtual void onClick();
	void onDrawContent(Range rows) override;
	bool onInteract(const Interaction& interaction) override;

public:
	NavigationPanel();
	NavigationPanel(String header, Action* handler);

	Action* handler;
};
