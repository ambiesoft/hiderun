#include <windows.h>
#include <tchar.h>

#include <string>
#include <memory>
#include <functional>

#define APPNAME_ "hiderun"
#define APPVERSION_ "1.0.8"

#define TO_LSTRINNER(x) L ## x
#define TO_LSTR(x) TO_LSTRINNER(x)

#define APPNAME TO_LSTR(APPNAME_)
#define APPVERSION TO_LSTR(APPVERSION_)
#define APPNAME_AND_VERSION (TO_LSTR(APPNAME_) TO_LSTR(" v") TO_LSTR(APPVERSION_))

#define I18N(s) (s)