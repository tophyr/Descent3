/*
* Descent 3 
* Copyright (C) 2024 Parallax Software
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LNXAPP_H
#define LNXAPP_H

// if no-display/input specifier is given, it will use defaults
#define APPFLAG_USESERVICE 0x00000100     // console (run no output/input)
#define APPFLAG_USESVGA 0x00000200        // console (use svgalib for input/output. NOTE: not used in code)
#define APPFLAG_NOMOUSECAPTURE 0x00000400 // don't capture the mouse in x-win
#define APPFLAG_NOSHAREDMEMORY 0x00000800 // force no-shared memory
#define APPFLAG_WINDOWEDMODE 0x00001000   // run in a window
#define APPFLAG_DGAMOUSE 0x00002000       // use DGA for Mouse

/*	Basic Application Linux data types */

//	This structure is used to retrieve and set
struct tLnxAppInfo {
  unsigned flags;                 // Application Flags
  int wnd_x, wnd_y, wnd_w, wnd_h; // Window dimensions
};

/*	Linux Application Object
                This object entails initialization and cleanup of all operating system
                elements, as well as data that libraries may need to initialize their
                systems.

        We also allow the option of setting these handles from outside the Application object.
*/

/*	Callbacks return a 0 if we don't want to call the default action for the message, otherwise return 1
 */
// typedef int (*tOELnxMsgCallback)(HWnd,unsigned,unsigned,unsigned);

class oeLnxApplication : public oeApplication {
  bool m_WasCreated;              // Tells us if this app set graphics or not.
  void (*m_DeferFunc)(bool idle); // function to call when deffering to OS
  static bool os_initialized;     // is the OS check initialized?
  static bool first_time;         // first time init?
public:
  //	Creates the application object
  oeLnxApplication(unsigned flags);
  //	Create object with a premade info
  oeLnxApplication(tLnxAppInfo *appinfo);
  virtual ~oeLnxApplication();
  //	initializes the object
  virtual void init();
  //	Function to retrieve information from object through a platform defined structure.
  virtual void get_info(void *appinfo);
  //	defer returns some flags.   essentially this function defers program control to OS.
  virtual unsigned defer();
  virtual const char *get_window_name(void);
  virtual void clear_window(void);
  //	set a function to run when deferring to OS.
  virtual void set_defer_handler(void (*func)(bool isactive));
  //	delays app for a certain amount of time
  virtual void delay(float secs);
  //	Sizes the displayable region of the app (the window)
  void set_sizepos(int x, int y, int w, int h);

  virtual int flags(void) const;

  unsigned m_Flags;
  int m_X, m_Y, m_W, m_H;   // window dimensions.
private:
  void os_init(); // initializes OS components.
};

#endif
