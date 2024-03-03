#include <Crystalline.h>
#include <LiquidCrystal.h>

class CrystalPrinter : public PrinterBase {
public:
    LiquidCrystal& lcd;
    
    CrystalPrinter(LiquidCrystal& lcd, int width, int height) : PrinterBase(width, height), lcd(lcd) {
        lcd.begin(width, height);
        lcd.clear();
    }

protected:
    virtual bool printCore(char c) override {
        if (posX >= width)
            return false;
        lcd.print(c);
        return true;
    }

    virtual bool moveCore(uint8_t x, uint8_t y) override {
        if (x < 0 || x >= width)
            return false;
        if (y < 0 || y >= height)
            return false;
        lcd.setCursor(x, y);
        return true;
    }
};

struct Source {
    int n = 0;

    int getInt() { return n; }

    void setInt(int value) { n = value; }

    void action() { n = 0; }
};

auto lcd = LiquidCrystal(2, 3, 4, 5, 6, 7);

auto printer = CrystalPrinter(lcd, 16, 2);

auto root = MenuLayout(Array<MenuPanel*> {
    new ControlPanel("Header", Array<Control*> {
        new NumberControl<int>("Number", "", propertyOf(&Source::getInt)),
        new SwitchControl("Switch", arrayOf<String>("a", "b", "c"), propertyOf(&Source::getInt, &Source::setInt)),
        new ButtonControl("click", delegateOf(&Source::action)),
    })
});

void setup() {
    Crystalline::begin(&printer, root);
}

void update() {
    Crystalline::update();
}
