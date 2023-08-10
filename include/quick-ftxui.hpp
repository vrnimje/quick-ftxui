#ifndef QUICK_FTXUI_HPP
#define QUICK_FTXUI_HPP

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/phoenix/function.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/recursive_variant.hpp>

#include "ftxui/component/component.hpp"      // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp" // for ComponentBase
#include "ftxui/component/component_options.hpp" // for InputOption
#include "ftxui/component/mouse.hpp"             // for ftxui
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp" // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/screen/screen.hpp"
#include "ftxui/util/ref.hpp" // for Ref

#include <filesystem>
#include <iostream>
#include <string>

namespace client {
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

namespace interpreter {
///////////////////////////////////////////////////////////////////////////
//  Variables (Wooahhh)
//////////////////////////////////////////////////////////////////////////

std::map<std::string, int> numbers;
std::map<std::string, std::string> strings;

}

namespace quick_ftxui_ast {
///////////////////////////////////////////////////////////////////////////
//  The AST
///////////////////////////////////////////////////////////////////////////
struct nil {};
struct button;
struct expression;
struct input;
struct slider;
struct menu;
struct toggle;
struct int_variable_decl;
struct str_variable_decl;

enum block_alignment { VERTICAL, HORIZONTAL };
enum button_option { Ascii, Animated, Simple, NoOpt };

typedef boost::variant<
    nil, boost::recursive_wrapper<button>, boost::recursive_wrapper<input>,
    boost::recursive_wrapper<slider>, boost::recursive_wrapper<menu>,
    boost::recursive_wrapper<toggle>, boost::recursive_wrapper<expression>,
    boost::recursive_wrapper<int_variable_decl>, 
    boost::recursive_wrapper<str_variable_decl>> node;

struct button {
    std::string placeholder;
    std::string func;
    button_option opt = button_option::NoOpt;
};

struct input {
    std::string placeholder;
    std::string value;
};

struct slider {
    std::string label;
    std::string value;
    int min;
    int max;
    int increment;
};

struct menu {
    std::vector<std::string> entries;
    int selected = 0;
};

struct toggle {
    std::vector<std::string> entries;
    int selected;
};

struct int_variable_decl {
    std::string identifier;
    int value = 0;
};

struct str_variable_decl {
    std::string identifier;
    std::string value = "";
};

struct expression {
    block_alignment align;
    std::list<node> expr;
};

// print function for debugging
inline std::ostream &operator<<(std::ostream &out, nil) {
    out << "nil";
    return out;
}

// print function for debugging
inline std::ostream &operator<<(std::ostream &out, button b) {
    out << "Placeholder: " << b.placeholder << " | Func: " << b.func;
    return out;
}

// print function for debugging
inline std::ostream &operator<<(std::ostream &out, input b) {
    out << "Placeholder: " << b.placeholder << " | Temp: " << b.value;
    return out;
}

inline std::ostream &operator<<(std::ostream &out, slider b) {
    out << "Label: " << b.label << " | Value: " << b.value
        << " | Min: " << b.min << " | Max: " << b.max
        << " | Increment: " << b.increment;
    return out;
}

inline std::ostream &operator<<(std::ostream &out, int_variable_decl b) {
    out << "Data Type: int" << " | Identifier: "
        << b.identifier << " | Value: " << b.value ;
    return out;
}

inline std::ostream &operator<<(std::ostream &out, str_variable_decl b) {
    out << "Data Type: string" << " | Identifier: "
        << b.identifier << " | Value: " << b.value ;
    return out;
}

} // namespace quick_ftxui_ast
} // namespace client

// clang-format off

BOOST_FUSION_ADAPT_STRUCT(client::quick_ftxui_ast::button,
                          (std::string, placeholder)
                          (std::string, func)
                          (client::quick_ftxui_ast::button_option, opt)
)

BOOST_FUSION_ADAPT_STRUCT(client::quick_ftxui_ast::input,
                          (std::string, placeholder)
                          (std::string, value)
)

BOOST_FUSION_ADAPT_STRUCT(client::quick_ftxui_ast::slider,
                          (std::string, label)
                          (std::string, value)
                          (int, min)
                          (int, max)
                          (int, increment)
)

BOOST_FUSION_ADAPT_STRUCT(client::quick_ftxui_ast::menu,
                          (std::vector<std::string>, entries)
                          (int, selected)
)

BOOST_FUSION_ADAPT_STRUCT(client::quick_ftxui_ast::toggle,
                          (std::vector <std::string> , entries)
                          (int, selected)
)

BOOST_FUSION_ADAPT_STRUCT(client::quick_ftxui_ast::int_variable_decl,
                          (std::string, identifier)
                          (int, value)
)

BOOST_FUSION_ADAPT_STRUCT(client::quick_ftxui_ast::str_variable_decl,
                          (std::string, identifier)
                          (std::string, value)
)

BOOST_FUSION_ADAPT_STRUCT(client::quick_ftxui_ast::expression,
                          (client::quick_ftxui_ast::block_alignment, align)
                          (std::list<client::quick_ftxui_ast::node>, expr)
)



// clang-format on

namespace client {
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

namespace quick_ftxui_parser {
using boost::phoenix::function;

int const tabsize = 4;

void tab(int indent) {
    for (int i = 0; i < indent; ++i)
        std::cout << ' ';
}

struct component_meta_data {
    ftxui::ScreenInteractive *screen;
    ftxui::Components components;
    ftxui::ButtonOption *options;
};

struct ast_printer {
    ast_printer(component_meta_data *data_, int indent = 0)
        : indent(indent), data(data_) {}
    ast_printer(component_meta_data *data_) : data(data_) {}

    void operator()(quick_ftxui_ast::expression const &) const;

    int indent;

    component_meta_data *data;
};

struct node_printer : boost::static_visitor<> {
    node_printer(component_meta_data *data_, int indent = 0)
        : indent(indent), data(data_) {}

    void operator()(client::quick_ftxui_ast::expression const &expr) const {
        component_meta_data data_nest{data->screen};
        ast_printer(&data_nest, indent + tabsize)(expr);

        switch (expr.align) {
        case quick_ftxui_ast::block_alignment::VERTICAL: {
            data->components.push_back(
                ftxui::Container::Vertical({(data_nest).components}));
            break;
        }
        case quick_ftxui_ast::block_alignment::HORIZONTAL: {
            data->components.push_back(
                ftxui::Container::Horizontal({(data_nest).components}));
            break;
        }
        default:
            throw std::runtime_error("Should never reach here");
            break;
        }
    }

    void operator()(quick_ftxui_ast::button const &text) const {
        tab(indent + tabsize);
        std::cout << "button: " << text << std::endl;

        if (text.func == "Exit") {
            switch (text.opt) {
            case quick_ftxui_ast::button_option::Ascii: {
                data->components.push_back(ftxui::Button(
                    text.placeholder, data->screen->ExitLoopClosure(),
                    data->options->Ascii()));
                break;
            }
            case quick_ftxui_ast::button_option::Animated: {
                data->components.push_back(ftxui::Button(
                    text.placeholder, data->screen->ExitLoopClosure(),
                    data->options->Animated()));
                break;
            }
            case quick_ftxui_ast::button_option::Simple: {
                data->components.push_back(ftxui::Button(
                    text.placeholder, data->screen->ExitLoopClosure(),
                    data->options->Simple()));
                break;
            }
            case quick_ftxui_ast::button_option::NoOpt: {
                data->components.push_back(ftxui::Button(
                    text.placeholder, data->screen->ExitLoopClosure(),
                    data->options->Simple()));
                break;
            }
            default:
                throw std::runtime_error("Should never reach here");
                break;
            }
        } else {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            data->components.push_back(ftxui::Button(text.placeholder, [&] {
                int pid = _getpid();
                std::string temp_path =
                    std::filesystem::temp_directory_path().string();
                std::string x = text.func + " 2>>" + temp_path +
                                "/quick-ftxui-" + std::to_string(pid) +
                                ".txt 1>&2";
                const char *str = x.c_str();
                std::unique_ptr<FILE, decltype(&_pclose)> _pipe(
                    _popen(str, "r"), _pclose);
            }));
#elif defined(__linux__) || defined(__APPLE__)
            data->components.push_back(ftxui::Button(text.placeholder, [&] {
                int pid = getpid();
                std::string temp_path =
                    std::filesystem::temp_directory_path().string();
                std::string x = text.func + " 2>>" + temp_path +
                                "/quick-ftxui-" + std::to_string(pid) +
                                ".txt 1>&2";
                const char *str = x.c_str();
                std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(str, "r"),
                                                              pclose);
            }));
#else 
            throw std::runtime_error("System Architecture not detected, system calls unavailable");
#endif
        }
    }

    void operator()(quick_ftxui_ast::slider const &text) const {
        tab(indent + tabsize);
        std::cout << "slider" << text << std::endl;

        if (auto It = interpreter::numbers.find(std::string(text.value)); It != interpreter::numbers.end()) {
            data->components.push_back(ftxui::Slider(text.label,
                                                 (int *)(&It->second), text.min,
                                                 text.max, text.increment));
        } else {
            throw std::runtime_error("Variable " + text.value + " not found");
        }
    }

    void operator()(quick_ftxui_ast::input const &text) const {
        tab(indent + tabsize);
        std::cout << "input: " << text << std::endl;
        if (auto It = interpreter::strings.find(std::string(text.value)); It != interpreter::strings.end()) {
            data->components.push_back(ftxui::Input(&It->second, text.placeholder));
        } else {
            throw std::runtime_error("Variable " + text.value + " not found");
        }
    }

    void operator()(quick_ftxui_ast::menu const &text) const {
        tab(indent + tabsize);
        data->components.push_back(
            ftxui::Menu(&text.entries, (int *)&text.selected));
    }

    void operator()(quick_ftxui_ast::toggle const &text) const {
        tab(indent + tabsize);
        data->components.push_back(
            ftxui::Toggle(&text.entries, (int *)&text.selected));
    }

    void operator()(quick_ftxui_ast::nil const &text) const {
        tab(indent + tabsize);
        std::cout << "nil: \"" << text << '"' << std::endl;
    }

    void operator()(quick_ftxui_ast::int_variable_decl const &text) const {
        tab(indent + tabsize);
        interpreter::numbers.insert_or_assign(text.identifier, text.value);
        std::cout << "Integer variable decl: " << text << std::endl;
    }

    void operator()(quick_ftxui_ast::str_variable_decl const &text) const {
        tab(indent + tabsize);
        interpreter::strings.insert_or_assign(text.identifier, text.value);
        std::cout << "String variable decl: " << text << std::endl;
    }

    int indent;
    component_meta_data *data;
};

void ast_printer::operator()(
    client::quick_ftxui_ast::expression const &expr) const {
    tab(indent);
    std::cout << "tag: "
              << "Node | Alignment: " << expr.align << std::endl;
    tab(indent);
    std::cout << '{' << std::endl;

    for (quick_ftxui_ast::node const &node : expr.expr) {
        boost::apply_visitor(node_printer(data, indent), node);
    }

    tab(indent);
    std::cout << '}' << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
//  The error handler
///////////////////////////////////////////////////////////////////////////////
struct error_handler_ {
    template <typename, typename, typename> struct result {
        typedef void type;
    };

    template <typename Iterator>
    void operator()(qi::info const &what, Iterator err_pos,
                    Iterator last) const {
        std::cout << "Error! Expecting " << what // what failed?
                  << " here: \""
                  << std::string(err_pos,
                                 last) // iterators to error-pos, end
                  << "\"" << std::endl;
    }
};

function<error_handler_> const error_handler = error_handler_();

template <typename Iterator>
struct parser
    : qi::grammar<Iterator, quick_ftxui_ast::expression(), ascii::space_type> {
    parser() : parser::base_type(expression) {
        qi::char_type char_;
        qi::uint_type uint_;
        qi::raw_type raw;
        qi::alpha_type alpha;
        qi::alnum_type alnum;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_4_type _4;

        using qi::fail;
        using qi::on_error;

        // clang-format off
        alignment_kw
          .add
          ("Vertical", quick_ftxui_ast::block_alignment::VERTICAL)
          ("Horizontal", quick_ftxui_ast::block_alignment::HORIZONTAL)
          ;

        buttonopt_kw
          .add
          ("Ascii", quick_ftxui_ast::button_option::Ascii)
          ("Animated", quick_ftxui_ast::button_option::Animated)
          ("Simple", quick_ftxui_ast::button_option::Simple)
          ;
        // clang-format on

        quoted_string %= qi::lexeme['"' >> +(char_ - '"') >> '"'];

        identifier =
                !qi::lexeme[!(alnum | '_')]
            >>  raw[qi::lexeme[(alpha | '_') >> *(alnum | '_')]]
            ;

        button_function =
            qi::lit("System") >> "(" >> quoted_string >> ")" | quoted_string;

        button_comp %= qi::lit("Button") >> '{' >> quoted_string >> ',' >>
                       button_function >> -(',' >> buttonopt_kw) >> '}';

        input_comp %= qi::lit("Input") >> '{' >> quoted_string >> ',' >>
                      identifier >> '}';

        slider_comp %= qi::lit("Slider") >> '{' >> quoted_string >> ',' >>
                       identifier >> ',' >> qi::int_ >> ',' >> qi::int_ >> ',' >>
                       qi::int_ >> '}';

        menu_comp %= qi::lit("Menu") >> '{' >> '[' >> *quoted_string >> ']' >>
                     ',' >> qi::int_ >> '}';

        toggle_comp %= qi::lit("Toggle") >> '{' >> '[' >> *quoted_string >>
                       ']' >> ',' >> qi::int_ >> '}';

        int_var_decl %= qi::lit("int") >> identifier >> -('=' > qi::int_);

        str_var_decl %= qi::lit("str") >> identifier >> -('=' > quoted_string); 

        node = button_comp | input_comp | slider_comp | menu_comp |
               toggle_comp | int_var_decl | str_var_decl | expression;

        expression = alignment_kw >> '{' >> *node >> '}';

        // Debugging and error handling and reporting support.
        BOOST_SPIRIT_DEBUG_NODES((button_comp)(expression));

        // Error handling
        on_error<fail>(button_comp, error_handler(_4, _3, _2));
    }
    qi::rule<Iterator, quick_ftxui_ast::expression(), ascii::space_type>
        expression;
    qi::rule<Iterator, quick_ftxui_ast::node(), ascii::space_type> node;
    qi::rule<Iterator, quick_ftxui_ast::button(), ascii::space_type>
        button_comp;
    qi::rule<Iterator, quick_ftxui_ast::input(), ascii::space_type> input_comp;
    qi::rule<Iterator, quick_ftxui_ast::menu(), ascii::space_type> menu_comp;
    qi::rule<Iterator, quick_ftxui_ast::toggle(), ascii::space_type>
        toggle_comp;
    qi::rule<Iterator, std::string(), ascii::space_type> quoted_string;
    qi::rule<Iterator, std::string(), ascii::space_type> button_function;
    qi::rule<Iterator, quick_ftxui_ast::slider(), ascii::space_type>
        slider_comp;
    qi::rule<Iterator, std::string(), ascii::space_type> identifier;
    qi::rule<Iterator, quick_ftxui_ast::int_variable_decl(), ascii::space_type> int_var_decl;
    qi::rule<Iterator, quick_ftxui_ast::str_variable_decl(), ascii::space_type> str_var_decl;
    qi::symbols<char, quick_ftxui_ast::block_alignment> alignment_kw;
    qi::symbols<char, quick_ftxui_ast::button_option> buttonopt_kw;
    
};
} // namespace quick_ftxui_parser

} // namespace client

#endif // QUICK_FTXUI_HPP