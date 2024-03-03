#include "Crystalline.h"

#pragma region PrinterBase

PrinterBase::PrinterBase(uint8_t width, uint8_t height) : width(width), height(height)
{
	posX = virtualX = width;
	posY = virtualY = height;
}

void PrinterBase::print(char c)
{
	if (printCore(c))
	{
		posX++;
		virtualX++;
	}
}

bool PrinterBase::ensureMove()
{
	if (virtualX != posX || virtualY != posY)
		return move(virtualX, virtualY);
	return true;
}

bool PrinterBase::move(uint8_t x, uint8_t y)
{
	if (moveCore(x, y))
	{
		posX = x;
		posY = y;
		return true;
	}
	else
	{
		posX = width;
		posY = height;
		return false;
	}
}

void PrinterBase::virtualMove(uint8_t x, uint8_t y)
{
	virtualX = x;
	virtualY = y;
}

uint8_t PrinterBase::getRemaining() const
{
	return getTotal() - getPosition();
}

uint8_t PrinterBase::getPosition() const
{
	return virtualX;
}

uint8_t PrinterBase::getTotal() const
{
	return width;
}

DrawContext* PrinterBase::begin(int row)
{
	virtualMove(0, row);
	return this;
}

void PrinterBase::write(char c)
{
	if (!ensureMove())
		return;
	print(c);
}

void PrinterBase::write(String s)
{
	if (!ensureMove())
		return;
	for (int i = 0; i < s.length(); i++)
		print(s[i]);
}

void PrinterBase::write(String s, Alignment alignment, uint8_t total, char padding)
{
	if (!ensureMove())
		return;

	uint8_t start = 0;
	uint8_t end = total;

	switch (alignment)
	{
	case Alignment::Back:
		start = total - s.length();
		break;
	case Alignment::Front:
		end = s.length();
		break;
	case Alignment::Center:
		start = max(uint8_t((total - s.length()) / 2), 0);
		end = min(uint8_t(start + s.length()), total);
		break;
	}

	for (int i = 0; i < total; i++)
	{
		if (i < start || i >= end)
			print(padding);
		else
			print(s[i - start]);
	}
}

void PrinterBase::fill(char c)
{
	if (!ensureMove())
		return;
	repeat(c, getRemaining());
}

void PrinterBase::fill(char prefix, char infix, char postfix)
{
	if (!ensureMove())
		return;
	repeat(prefix, infix, postfix, getRemaining());
}

void PrinterBase::fill(String s, Alignment aligment, char padding)
{
	if (!ensureMove())
		return;
	write(s, aligment, getRemaining(), padding);
}

void PrinterBase::repeat(char c, uint8_t count)
{
	if (!ensureMove())
		return;
	for (int i = 0; i < count; i++)
		print(c);
}

void PrinterBase::repeat(char prefix, char infix, char postfix, uint8_t count)
{
	if (!ensureMove())
		return;
	print(prefix);
	for (int i = 0; i < count - 2; i++)
		print(infix);
	print(postfix);
}

bool PrinterBase::omit(uint8_t count, bool check)
{
	if (!check) {
		virtualX += count;
		return true;
	}
	return false;
}

#pragma endregion
