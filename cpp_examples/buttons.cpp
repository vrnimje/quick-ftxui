#include "quick-ftxui.hpp"
#include <iostream>
#include <string>

using namespace std;
using namespace quick_ftxui;

int main() {
    add_int_var("x", 5);
    add_str_var("y", "");
    string source_code = R"(Vertical{
        str z = "init"
        str a
        Input {
            "Type something...",
            y
        }
        Slider {
            "Test: ",
            x,
            0,
            100,
            2
        }
        Button{
            "ls",
            System("ls -a"),
            Ascii,
            z
        }
        Button {
            "Exit",
            "Exit"
        }
    })";

    parse_qf(source_code);

    cout << "x is: " << get_int("x") << "\n";
    cout << "y is: " << get_str("y") << "\n";
    cout << "z is: \n" << get_str("z") << "\n";
}