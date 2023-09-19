#include "quick-ftxui.hpp"
#include <iostream>
#include <string>

using namespace std;
using namespace quick_ftxui;

int main() {
  int x = 5;
  set_int_var("x", &x);
  set_str_var("y");
  string source_code = R"(DashedBorder Vertical{
        str z = "init"
        str a
        int o = 0
        Input {
            "Type something...",
            y
        }
        RedLight Slider {
            "Test: ",
            x,
            0,
            100,
            2
        }
        Magenta Button{
            "Chrome",
            System("/usr/bin/google-chrome-stable"),
            Animated,
            z
        }
        Green Menu{
            [ "Physics",  "Maths",  "Chemistry",  "Biology",],
            VerticalAnimated,
            o
        }
        Button {
            "Exit",
            "Exit"
        }
    })";

  parse_qf(source_code);

  cout << "x is: " << x << "\n";
  cout << "y is: " << get_str("y") << "\n";
  cout << "o is: " << get_int("o") << "\n";
  cout << "z is: " << get_str("z") << "\n";
}