#include <windows.h>
#include <tchar.h>

#include <string>
#include <memory>
#include <functional>

#define APPNAME_ "hiderun"
#define APPVERSION_ "1.0.3"

#define TO_STR2(x) L ## x
#define TO_STR(x) TO_STR2(x)

#define APPNAME TO_STR(APPNAME_)
#define APPVERSION TO_STR(APPVERSION_)

#define APPNAME_AND_VERSION (TO_STR(APPNAME_) TO_STR(" ver ") TO_STR(APPVERSION_))

