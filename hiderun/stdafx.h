#include <windows.h>
#include <tchar.h>

#include <string>
#include <memory>
#include <functional>

#define APPNAME_ "hiderun"

#define TO_LSTRINNER(x) L ## x
#define TO_LSTR(x) TO_LSTRINNER(x)

#define APPNAME TO_LSTR(APPNAME_)

#define I18N(s) (s)