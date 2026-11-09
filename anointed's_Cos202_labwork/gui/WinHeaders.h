#ifndef WINHEADERS_H
#define WINHEADERS_H

// windows version needed for list view colours, tooltips and file dialogs
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

#endif
