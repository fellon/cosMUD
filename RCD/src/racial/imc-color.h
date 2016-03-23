/* $Id: imc-color.h,v 1.4 2000/02/03 13:56:23 gmoody Exp $ */
/*
 * IMC Color Handling
 *
 * This will probably be replaced later when you can change color codes
 * from inside the mud. Suggestions? Improvements? Email them to me.
 *
 * I need to put in more for other codebases. Will do them later.
 *
 * Kratas (moon@deathmoon.com/Kratas@eternal)
 */

/* If you use Lope's color code, define this here */
//#define LOPE_COLOR

/* Colors for ACKMud */
#ifdef ACK
#define IMC_COLORCHAR    '@'
#define IMC_COLORCHAR2   '@'
//#define COLOR_CODE_CHAR  "@"
#define IMC_GREY         "@@g"
#define IMC_RED          "@@R"
#define IMC_GREEN        "@@G"
#define IMC_BROWN        "@@b"
#define IMC_BLUE         "@@B"
#define IMC_MAGENTA      "@@m"
#define IMC_CYAN         "@@c"
#define IMC_BLACK        "@@k"
#define IMC_YELLOW       "@@y"
#define IMC_WHITE        "@@W"
#define IMC_NORMAL       "@@N"
#define IMC_PURPLE       "@@p"
#define IMC_DARK_GREY    "@@d"
#define IMC_BOLD_BLUE    "@@l"
#define IMC_BOLD_GREEN   "@@r"
#define IMC_BOLD_MAGENTA "@@p"
#define IMC_BOLD_CYAN    "@@a"
#define IMC_BOLD_RED     "@@e"
#define IMC_BOLD         "@@x"

/* Colors for SMAUG */
#elif defined(SMAUG)
#define IMC_COLORCHAR    '&'
#define IMC_COLORCHAR2   '^'
//#define COLOR_CODE_CHAR  "&"
#define IMC_GREY         "&w"
#define IMC_RED          "&r"
#define IMC_GREEN        "&g"
#define IMC_BROWN        "&O"
#define IMC_BLUE         "&b"
#define IMC_CYAN         "&c"
#define IMC_BLACK        "&x"
#define IMC_MAGENTA      "&p"
#define IMC_YELLOW       "&Y"
#define IMC_WHITE        "&W"
#define IMC_NORMAL       "&w"
#define IMC_PURPLE       "&p"
#define IMC_DARK_GREY    "&z"
#define IMC_BOLD_BLUE    "&B"
#define IMC_BOLD_GREEN   "&G"
#define IMC_BOLD_MAGENTA "&P"
#define IMC_BOLD_CYAN    "&C"
#define IMC_BOLD_RED     "&R"
#define IMC_BOLD         "&W"

/* Colors for CircleMUD */
#elif defined(CIRCLE_COLORS)
/* Circle Color Codes... Easy Colour 2.2 */
#define IMC_COLORCHAR    '&'
#define IMC_COLORCHAR2   '&'
//#define COLOR_CODE_CHAR  "&"
#define IMC_BLUE         "&b"
#define IMC_GREEN        "&g"
#define IMC_RED          "&r"
#define IMC_BROWN        "&y"
#define IMC_MAGENTA      "&m"
#define IMC_CYAN         "&c"
#define IMC_GREY         "&w"
#define IMC_PURPLE       "&m"
#define IMC_DARK_GREY    "&K"
#define IMC_BOLD_BLUE    "&B"
#define IMC_BOLD_GREEN   "&G"
#define IMC_BOLD_RED     "&R"
#define IMC_YELLOW       "&Y"
#define IMC_BOLD_MAGENTA "&M"
#define IMC_BOLD_CYAN    "&C"
#define IMC_WHITE        "&W"
#define IMC_NORMAL       "&n"
#define BOLD             ""

#elif defined(ROM) || defined(ROT) || defined(ENVY) || defined(LOPE_COLOR)
/* Going to assume that these 3 will prolly use this snippet */
#define IMC_COLORCHAR    '{'
#define IMC_COLORCHAR2   '{'
//#define COLOR_CODE_CHAR  "{"
#define IMC_GREY         "{w"
#define IMC_RED          "{r"
#define IMC_GREEN        "{g"
#define IMC_BROWN        "{y"
#define IMC_BLUE         "{b"
#define IMC_CYAN         "{c"
#define IMC_BLACK        "{D"
#define IMC_MAGENTA      "{m"
#define IMC_YELLOW       "{Y"
#define IMC_WHITE        "{W"
#define IMC_NORMAL       "{x"
#define IMC_PURPLE       "{m"
#define IMC_DARK_GREY    "{D"
#define IMC_BOLD_BLUE    "{B"
#define IMC_BOLD_GREEN   "{G"
#define IMC_BOLD_MAGENTA "{M"
#define IMC_BOLD_CYAN    "{C"
#define IMC_BOLD_RED     "{R"
#define IMC_BOLD         ""

/*
 * Everything else.
 * Define your custom colors here if needed.
 */
#else
#define COLOR_CODE_CHAR  ""
#define IMC_GREY         ""
#define IMC_RED          ""
#define IMC_GREEN        ""
#define IMC_BROWN        ""
#define IMC_BLUE         ""
#define IMC_CYAN         ""
#define IMC_BLACK        ""
#define IMC_MAGENTA      ""
#define IMC_YELLOW       ""
#define IMC_WHITE        ""
#define IMC_NORMAL       ""
#define IMC_PURPLE       ""
#define IMC_DARK_GREY    ""
#define IMC_BOLD_BLUE    ""
#define IMC_BOLD_GREEN   ""
#define IMC_BOLD_MAGENTA ""
#define IMC_BOLD_CYAN    ""
#define IMC_BOLD_RED     ""
#define IMC_BOLD         ""

#endif
