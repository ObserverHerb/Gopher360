#include "Gopher.h"
#include "ConfigFile.h"

// Description:
//   Send a keyboard input to the system based on the key value
//     and its event type.
//
// Params:
//   cmd    The value of the key to send(see http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx)
//   flag   The KEYEVENT for the key
void inputKeyboard(std::vector<WORD> cmds, DWORD flag)
{
  std::vector<INPUT> inputs;
  for (const WORD cmd : cmds)
  {
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = 0;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    input.ki.wVk = cmd;
    input.ki.dwFlags = flag;
    inputs.push_back(input);
  }
  SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
}

// Description:
//   Send a keyboard input based on the key value with the "pressed down" event.
//
// Params:
//   cmd    The value of the keys to send
void inputKeyboardDown(std::vector<WORD> cmds)
{
  inputKeyboard(cmds, 0);
}

// Description:
//   Send a keyboard input based on the key value with the "released" event
//
// Params:
//   cmds    The value of the keys to send
void inputKeyboardUp(std::vector<WORD> cmds)
{
  inputKeyboard(cmds, KEYEVENTF_KEYUP);
}

// Description:
//   Send a mouse input based on a mouse event type.
//   See https://msdn.microsoft.com/en-us/library/windows/desktop/ms646310(v=vs.85).aspx
//
// Params:
//   dwFlags    The mouse event to send
//   mouseData  Additional information needed for certain mouse events (Optional)
void mouseEvent(DWORD dwFlags, DWORD mouseData = 0)
{
  INPUT input;
  input.type = INPUT_MOUSE;

  // Only set mouseData when using a supported dwFlags type
  if (dwFlags == MOUSEEVENTF_WHEEL ||
      dwFlags == MOUSEEVENTF_XUP   ||
      dwFlags == MOUSEEVENTF_XDOWN ||
      dwFlags == MOUSEEVENTF_HWHEEL)
  {
    input.mi.mouseData = mouseData;
  }
  else
  {
    input.mi.mouseData = 0;
  }

  input.mi.dwFlags = dwFlags;
  input.mi.time = 0;
  SendInput(1, &input, sizeof(INPUT));
}

std::vector<WORD> stringsToShorts(const std::vector<std::string> &strings)
{
  std::vector<WORD> shorts;
  for (const std::string &string : strings) shorts.push_back(std::stoi(string.c_str(), 0, 0));
  return shorts;
}

Gopher::Gopher(CXBOXController * controller)
  : _controller(controller)
{
}

// Description:
//   Reads and parses the configuration file, assigning values to the 
//     configuration variables.
void Gopher::loadConfigFile()
{
  ConfigFile cfg("config.ini");
  
  //--------------------------------
  // Configuration bindings
  //--------------------------------
  CONFIG_MOUSE_LEFT = stringsToShorts(cfg.getValuesOfKey("CONFIG_MOUSE_LEFT")).front();
  CONFIG_MOUSE_RIGHT = stringsToShorts(cfg.getValuesOfKey("CONFIG_MOUSE_RIGHT")).front();
  CONFIG_MOUSE_MIDDLE = stringsToShorts(cfg.getValuesOfKey("CONFIG_MOUSE_MIDDLE")).front();
  CONFIG_HIDE = stringsToShorts(cfg.getValuesOfKey("CONFIG_HIDE")).front();
  CONFIG_DISABLE = stringsToShorts(cfg.getValuesOfKey("CONFIG_DISABLE")).front();
  CONFIG_DISABLE_VIBRATION = stringsToShorts(cfg.getValuesOfKey("CONFIG_DISABLE_VIBRATION")).front();
  CONFIG_SPEED_CHANGE = stringsToShorts(cfg.getValuesOfKey("CONFIG_SPEED_CHANGE")).front();
  CONFIG_OSK = stringsToShorts(cfg.getValuesOfKey("CONFIG_OSK")).front();

  //--------------------------------
  // Controller bindings
  //--------------------------------
  GAMEPAD_DPAD_UP = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_DPAD_UP"));
  GAMEPAD_DPAD_DOWN = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_DPAD_DOWN"));
  GAMEPAD_DPAD_LEFT = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_DPAD_LEFT"));
  GAMEPAD_DPAD_RIGHT = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_DPAD_RIGHT"));
  GAMEPAD_START = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_START"));
  GAMEPAD_BACK = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_BACK"));
  GAMEPAD_LEFT_THUMB = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_LEFT_THUMB"));
  GAMEPAD_RIGHT_THUMB = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_RIGHT_THUMB"));
  GAMEPAD_LEFT_SHOULDER = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_LEFT_SHOULDER"));
  GAMEPAD_RIGHT_SHOULDER = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_RIGHT_SHOULDER"));
  GAMEPAD_A = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_A"));
  GAMEPAD_B = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_B"));
  GAMEPAD_X = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_X"));
  GAMEPAD_Y = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_Y"));
  GAMEPAD_TRIGGER_LEFT = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_TRIGGER_LEFT"));
  GAMEPAD_TRIGGER_RIGHT = stringsToShorts(cfg.getValuesOfKey("GAMEPAD_TRIGGER_RIGHT"));

  //--------------------------------
  // Advanced settings
  //--------------------------------

  // Acceleration factor
  acceleration_factor = strtof(cfg.getValuesOfKey("ACCELERATION_FACTOR").at(0).c_str(), 0);

  // Dead zones
  DEAD_ZONE = strtol(cfg.getValuesOfKey("DEAD_ZONE").at(0).c_str(), 0, 0);
  if (DEAD_ZONE == 0)
  {
    DEAD_ZONE = 6000;
  }

  SCROLL_DEAD_ZONE = strtol(cfg.getValuesOfKey("SCROLL_DEAD_ZONE").at(0).c_str(), 0, 0);
  if (SCROLL_DEAD_ZONE == 0)
  {
    SCROLL_DEAD_ZONE = 5000;
  }

  SCROLL_SPEED = strtof(cfg.getValuesOfKey("SCROLL_SPEED").at(0).c_str(), 0);
  if (SCROLL_SPEED < 0.00001f)
  {
    SCROLL_SPEED = 0.1f;
  }

  // Variable cursor speeds
  std::istringstream cursor_speed = std::istringstream(cfg.getValuesOfKey("CURSOR_SPEED").at(0).c_str());
  int cur_speed_idx = 1;
  const float CUR_SPEED_MIN = 0.0001f;
  const float CUR_SPEED_MAX = 1.0f;
  for (std::string cur_speed; std::getline(cursor_speed, cur_speed, ',');)
  {
    std::istringstream cursor_speed_entry = std::istringstream(cur_speed);
    std::string cur_name, cur_speed_s;
    // Check to see if we are at the string that includes the equals sign.
    if (cur_speed.find_first_of('=') != std::string::npos)
    {
      std::getline(cursor_speed_entry, cur_name, '=');
    }
    else
    {
      std::ostringstream tmp_name;
      tmp_name << cur_speed_idx++;
      cur_name = tmp_name.str();
    }
    std::getline(cursor_speed_entry, cur_speed_s);
    float cur_speedf = strtof(cur_speed_s.c_str(), 0);
    // Ignore speeds that are not within the allowed range.
    if (cur_speedf > CUR_SPEED_MIN && cur_speedf <= CUR_SPEED_MAX)
    {
      speeds.push_back(cur_speedf);
      speed_names.push_back(cur_name);
    }
  }

  // If no cursor speeds were defined, add a set of default speeds.
  if (speeds.size() == 0)
  {
    speeds.push_back(0.005f);
    speeds.push_back(0.015f);
    speeds.push_back(0.025f);
    speeds.push_back(0.004f);
    speed_names.push_back("ULTRALOW");
    speed_names.push_back("LOW");
    speed_names.push_back("MED");
    speed_names.push_back("HIGH");
  }
  speed = speeds[0];  // Initialize the speed to the first speed stored. TODO: Set the speed to a saved speed that was last used when the application was closed last.

  // Swap stick functions
  SWAP_THUMBSTICKS = strtol(cfg.getValuesOfKey("SWAP_THUMBSTICKS").at(0).c_str(), 0, 0);

  // Set the initial window visibility
  setWindowVisibility(_hidden);
}

// Description:
//   The main program loop. Handles the gamepad inputs and converts them
//     to system inputs based on the mapping provided by the configuration
//     file.
void Gopher::loop()
{
  Sleep(SLEEP_AMOUNT);

  _currentState = _controller->GetState();

  // Disable Gopher
  handleDisableButton();
  if (_disabled)
  {
    return;
  }

  // Vibration
  handleVibrationButton();

  // Mouse functions
  handleMouseMovement();
  handleScrolling();

  if (CONFIG_MOUSE_LEFT)
  {
    mapMouseClick(CONFIG_MOUSE_LEFT, MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP);
  }
  if (CONFIG_MOUSE_RIGHT)
  {
    mapMouseClick(CONFIG_MOUSE_RIGHT, MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP);
  }
  if (CONFIG_MOUSE_MIDDLE)
  {
    mapMouseClick(CONFIG_MOUSE_MIDDLE, MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP);
  }

  // Hides the console
  if (CONFIG_HIDE)
  {
    setXboxClickState(CONFIG_HIDE);
    if (_xboxClickIsDown[CONFIG_HIDE])
    {
      toggleWindowVisibility();
    }
  }

  // Toggle the on-screen keyboard
  if (CONFIG_OSK)
  {
    setXboxClickState(CONFIG_OSK);
    if (_xboxClickIsDown[CONFIG_OSK])
    {
      // Get the otk window
      HWND otk_win = getOskWindow();
      if (otk_win == NULL)
      {
        printf("Please start the On-screen keyboard first\n");
      }
      else if(IsIconic(otk_win))
      {
        ShowWindow(otk_win, SW_RESTORE);
      }
      else
      {
        ShowWindow(otk_win, SW_MINIMIZE);
      }
    }
  }

  // Will change between the current speed values
  setXboxClickState(CONFIG_SPEED_CHANGE);
  if (_xboxClickIsDown[CONFIG_SPEED_CHANGE])
  {
    const int CHANGE_SPEED_VIBRATION_INTENSITY = 65000;   // Speed of the vibration motors when changing cursor speed.
    const int CHANGE_SPEED_VIBRATION_DURATION = 450;      // Duration of the cursor speed change vibration in milliseconds.

    speed_idx++;
    if (speed_idx >= speeds.size())
    {
      speed_idx = 0;
    }
    speed = speeds[speed_idx];
    printf("Setting speed to %f (%s)...\n", speed, speed_names[speed_idx].c_str());
    pulseVibrate(CHANGE_SPEED_VIBRATION_DURATION, CHANGE_SPEED_VIBRATION_INTENSITY, CHANGE_SPEED_VIBRATION_INTENSITY);
  }

  // Update all controller keys.
  handleTriggers(GAMEPAD_TRIGGER_LEFT, GAMEPAD_TRIGGER_RIGHT);
  if (!GAMEPAD_DPAD_UP.empty())
  {
    mapKeyboard(XINPUT_GAMEPAD_DPAD_UP, GAMEPAD_DPAD_UP);
  }
  if (!GAMEPAD_DPAD_DOWN.empty())
  {
    mapKeyboard(XINPUT_GAMEPAD_DPAD_DOWN, GAMEPAD_DPAD_DOWN);
  }
  if (!GAMEPAD_DPAD_LEFT.empty())
  {
    mapKeyboard(XINPUT_GAMEPAD_DPAD_LEFT, GAMEPAD_DPAD_LEFT);
  }
  if (!GAMEPAD_DPAD_RIGHT.empty())
  {
    mapKeyboard(XINPUT_GAMEPAD_DPAD_RIGHT, GAMEPAD_DPAD_RIGHT);
  }
  if (!GAMEPAD_START.empty())
  {
    mapKeyboard(XINPUT_GAMEPAD_START, GAMEPAD_START);
  }
  if (!GAMEPAD_BACK.empty())
  {
    mapKeyboard(XINPUT_GAMEPAD_BACK, GAMEPAD_BACK);
  }
  if (!GAMEPAD_LEFT_THUMB.empty())
  {
    mapKeyboard(XINPUT_GAMEPAD_LEFT_THUMB, GAMEPAD_LEFT_THUMB);
  }
  if (!GAMEPAD_RIGHT_THUMB.empty())
  {
      mapKeyboard(XINPUT_GAMEPAD_RIGHT_THUMB, GAMEPAD_RIGHT_THUMB);
  }
  if (!GAMEPAD_LEFT_SHOULDER.empty())
  {
      mapKeyboard(XINPUT_GAMEPAD_LEFT_SHOULDER, GAMEPAD_LEFT_SHOULDER);
  }
  if (!GAMEPAD_RIGHT_SHOULDER.empty())
  {
      mapKeyboard(XINPUT_GAMEPAD_RIGHT_SHOULDER, GAMEPAD_RIGHT_SHOULDER);
  }
  if (!GAMEPAD_A.empty())
  {
      mapKeyboard(XINPUT_GAMEPAD_A, GAMEPAD_A);
  }
  if (!GAMEPAD_B.empty())
  {
      mapKeyboard(XINPUT_GAMEPAD_B, GAMEPAD_B);
  }
  if (!GAMEPAD_X.empty())
  {
      mapKeyboard(XINPUT_GAMEPAD_X, GAMEPAD_X);
  }
  if (!GAMEPAD_Y.empty())
  {
    mapKeyboard(XINPUT_GAMEPAD_Y, GAMEPAD_Y);
  }
}

// Description:
//   Sends a vibration pulse to the controller for a duration of time.
//     This is a BLOCKING call. Any inputs during the vibration will be IGNORED.
//
// Params:
//   duration   The length of time in milliseconds to vibrate for
//   l          The speed (intensity) of the left vibration motor
//   r          The speed (intensity) of the right vibration motor
void Gopher::pulseVibrate(const int duration, const int l, const int r) const
{
  if(!_vibrationDisabled)
  {
    _controller->Vibrate(l, r);
    Sleep(duration);
    _controller->Vibrate(0, 0);
  }
}

// Description:
//   Toggles the controller mapping after checking for the disable configuration command.
void Gopher::handleDisableButton()
{
  setXboxClickState(CONFIG_DISABLE);
  if (_xboxClickIsDown[CONFIG_DISABLE])
  {
    int duration = 0;   // milliseconds
    int intensity = 0;  // vibration intensity

    _disabled = !_disabled;

    if (_disabled)
    {
      // Transition to a disabled state.
      duration = 400;
      intensity = 10000;

      // Release all keys currently pressed by the Gopher mapping.
      std::vector<WORD> keyboardEvents;
      while (!_pressedKeys.empty())
      {
        WORD keyEvent = _pressedKeys.front();

        // Handle mouse buttons
        // TODO: support mouse X1 and X2 buttons
        switch (keyEvent)
        {
        case VK_LBUTTON:
          mouseEvent(MOUSEEVENTF_LEFTUP);
          break;
        case VK_RBUTTON:
          mouseEvent(MOUSEEVENTF_RIGHTUP);
          break;
        case VK_MBUTTON:
          mouseEvent(MOUSEEVENTF_MIDDLEUP);
          break;
        default:
          keyboardEvents.push_back(keyEvent);
        }

        _pressedKeys.pop_front();
      }

      if (!keyboardEvents.empty()) inputKeyboardUp(keyboardEvents);
    }
    else
    {
      duration = 400;
      intensity = 65000;
    }

    pulseVibrate(duration, intensity, intensity);
  }
}

// Description:
//   Toggles the vibration support after checking for the diable vibration command. 
//   This function will BLOCK to prevent rapidly toggling the vibration.
void Gopher::handleVibrationButton()
{
  setXboxClickState(CONFIG_DISABLE_VIBRATION);
  if (_xboxClickIsDown[CONFIG_DISABLE_VIBRATION])
  {
    _vibrationDisabled = !_vibrationDisabled;
    printf("Vibration %s\n", _vibrationDisabled ? "Disabled" : "Enabled");
    Sleep(1000);
  }
}

// Description:
//   Toggles the visibility of the window.
void Gopher::toggleWindowVisibility()
{
  _hidden = !_hidden;
  printf("Window %s\n", _hidden ? "hidden" : "unhidden");
  setWindowVisibility(_hidden);
}

// Description:
//   Either hides or shows the window.
// 
// Params:
//   hidden   Hides the window when true
void Gopher::setWindowVisibility(const bool &hidden) const
{
  HWND hWnd = GetConsoleWindow();
  ShowWindow(hWnd, _hidden ? SW_HIDE : SW_SHOW);
}

template <typename T>
int sgn(T val)
{
  return (T(0) < val) - (val < T(0));
}

// Description:
//   Determines if the thumbstick value is valid and converts it to a float.
//
// Params:
//   t  Analog thumbstick value to check and convert
//
// Returns:
//   If the value is valid, t will be returned as-is as a float. If the value is 
//     invalid, 0 will be returned.
float Gopher::getDelta(short t)
{
  //filter non-32768 and 32767, wireless ones can glitch sometimes and send it to the edge of the screen, it'll toss out some HUGE integer even when it's centered
  if (t > 32767) t = 0;
  if (t < -32768) t = 0;

  return t;
}

// Description:
//   Calculates a multiplier for an analog thumbstick based on the update rate.
//
// Params:
//   tValue     The thumbstick value
//   deadzone   The dead zone to use for this thumbstick
//   accel      An exponent to use to create an input curve (Optional). 0 to use a linear input
//   
// Returns:
//   Multiplier used to properly scale the given thumbstick value.
float Gopher::getMult(float lengthsq, float deadzone, float accel = 0.0f)
{
  // Normalize the thumbstick value.
  float mult = (sqrt(lengthsq) - deadzone) / (MAXSHORT - deadzone);

  // Apply a curve to the normalized thumbstick value.
  if (accel > 0.0001f)
  {
    mult = pow(mult, accel);
  }
  return mult / FPS;
}

// Description:
//   Controls the mouse cursor movement by reading the left thumbstick.
void Gopher::handleMouseMovement()
{
  POINT cursor;
  GetCursorPos(&cursor);

  short tx;
  short ty;

  if (SWAP_THUMBSTICKS == 0)
  {
    // Use left stick
    tx = _currentState.Gamepad.sThumbLX;
    ty = _currentState.Gamepad.sThumbLY;
  }
  else
  {
    // Use right stick
    tx = _currentState.Gamepad.sThumbRX;
    ty = _currentState.Gamepad.sThumbRY;
  }

  float x = cursor.x + _xRest;
  float y = cursor.y + _yRest;

  float dx = 0;
  float dy = 0;

  // Handle dead zone
  float lengthsq = tx * tx + ty * ty;
  if (lengthsq > DEAD_ZONE * DEAD_ZONE)
  {
    float mult = speed * getMult(lengthsq, DEAD_ZONE, acceleration_factor);

    dx = getDelta(tx) * mult;
    dy = getDelta(ty) * mult;
  }

  x += dx;
  _xRest = x - (float)((int)x);

  y -= dy;
  _yRest = y - (float)((int)y);

  SetCursorPos((int)x, (int)y); //after all click input processing
}

// Description:
//   Controls the scroll wheel movement by reading the right thumbstick.
void Gopher::handleScrolling()
{
  float tx;
  float ty;
  
  if (SWAP_THUMBSTICKS == 0)
  {
    // Use right stick
    tx = getDelta(_currentState.Gamepad.sThumbRX);
    ty = getDelta(_currentState.Gamepad.sThumbRY);
  }
  else
  {
    // Use left stick
    tx = getDelta(_currentState.Gamepad.sThumbLX);
    ty = getDelta(_currentState.Gamepad.sThumbLY);
  }

  // Handle dead zone
  float magnitude = sqrt(tx * tx + ty * ty);

  if (magnitude > SCROLL_DEAD_ZONE)
  {
    mouseEvent(MOUSEEVENTF_HWHEEL, tx * getMult(tx * tx, SCROLL_DEAD_ZONE) * SCROLL_SPEED);
    mouseEvent(MOUSEEVENTF_WHEEL, ty * getMult(ty * ty, SCROLL_DEAD_ZONE) * SCROLL_SPEED);
  }
}

// Description:
//   Handles the trigger-to-key mapping. The triggers are handled separately since
//     they are analog instead of a simple button press.
//
// Params:
//   lKey   The mapped key for the left trigger
//   rKey   The mapped key for the right trigger
void Gopher::handleTriggers(std::vector<WORD> lKey, std::vector<WORD> rKey)
{
  bool lTriggerIsDown = _currentState.Gamepad.bLeftTrigger > TRIGGER_DEAD_ZONE;
  bool rTriggerIsDown = _currentState.Gamepad.bRightTrigger > TRIGGER_DEAD_ZONE;

  // Handle left trigger
  if (lTriggerIsDown != _lTriggerPrevious)
  {
    _lTriggerPrevious = lTriggerIsDown;
    if (lTriggerIsDown)
    {
      inputKeyboardDown(lKey);
    }
    else
    {
      inputKeyboardUp(lKey);
    }
  }

  // Handle right trigger
  if (rTriggerIsDown != _rTriggerPrevious)
  {
    _rTriggerPrevious = rTriggerIsDown;
    if (rTriggerIsDown)
    {
      inputKeyboardDown(rKey);
    }
    else
    {
      inputKeyboardUp(rKey);
    }
  }
}

// Description:
//   Handles the state of a controller button press.
//
// Params:
//   STATE  The Gopher state, or command, to update
void Gopher::setXboxClickState(DWORD STATE)
{
  _xboxClickIsDown[STATE] = false;
  _xboxClickIsUp[STATE] = false;

  if (!this->xboxClickStateExists(STATE))
  {
    _xboxClickStateLastIteration[STATE] = false;
  }

  bool isDown = (_currentState.Gamepad.wButtons & STATE) == STATE;

  // Detect if the button has been pressed.
  if (isDown && !_xboxClickStateLastIteration[STATE])
  {
    _xboxClickStateLastIteration[STATE] = true;
    _xboxClickIsDown[STATE] = true;
    _xboxClickDownLength[STATE] = 0;
    _xboxClickIsDownLong[STATE] = false;
  }

  // Detect if the button has been held as a long press.
  if (isDown && _xboxClickStateLastIteration[STATE])
  {
    const int LONG_PRESS_TIME = 200;  // milliseconds

    ++_xboxClickDownLength[STATE];
    if (_xboxClickDownLength[STATE] * SLEEP_AMOUNT > LONG_PRESS_TIME)
    {
      _xboxClickIsDownLong[STATE] = true;
    }
  }

  // Detect if the button has been released.
  if (!isDown && _xboxClickStateLastIteration[STATE])
  {
    _xboxClickStateLastIteration[STATE] = false;
    _xboxClickIsUp[STATE] = true;
    _xboxClickIsDownLong[STATE] = false;
  }

  _xboxClickStateLastIteration[STATE] = isDown;
}

// Description:
//   Check to see if a controller state exists in Gopher's button map.
//
// Params:
//   xinput   The Gopher state, or command, to search for
//
// Returns:
//   true if the state is present in the map.
bool Gopher::xboxClickStateExists(DWORD STATE)
{
  auto it = _xboxClickStateLastIteration.find(STATE);
  if (it == _xboxClickStateLastIteration.end())
  {
    return false;
  }

  return true;
}

// Description:
//   Presses or releases a key based on a mapped Gopher state.
//
// Params:
//   STATE  The Gopher state, or command, to trigger a key event
//   key    The key value to input to the system
void Gopher::mapKeyboard(DWORD STATE, std::vector<WORD> keys)
{
  setXboxClickState(STATE);
  if (_xboxClickIsDown[STATE])
  {
    inputKeyboardDown(keys);

    // Add key to the list of pressed keys.
    for (const WORD key : keys) _pressedKeys.push_back(key);
  }

  if (_xboxClickIsUp[STATE])
  {
    inputKeyboardUp(keys);

    // Remove key from the list of pressed keys.
    for (const WORD key : keys) erasePressedKey(key);
  }
}

// Description:
//   Presses or releases a mouse button based on a mapped Gopher state
//
// Params:
//   STATE    The Gopher state, or command, to trigger a mouse event
//   keyDown  The button down event for a mouse event
//   keyUp    The button up event for a mouse event
void Gopher::mapMouseClick(DWORD STATE, DWORD keyDown, DWORD keyUp)
{
  setXboxClickState(STATE);
  if (_xboxClickIsDown[STATE])
  {
    mouseEvent(keyDown);

    // Add key to the list of pressed keys.
    if (keyDown == MOUSEEVENTF_LEFTDOWN)
    {
      _pressedKeys.push_back(VK_LBUTTON);
    }
    else if (keyDown == MOUSEEVENTF_RIGHTDOWN)
    {
      _pressedKeys.push_back(VK_RBUTTON);
    }
    else if (keyDown == MOUSEEVENTF_MIDDLEDOWN)
    {
      _pressedKeys.push_back(VK_MBUTTON);
    }
  }

  if (_xboxClickIsUp[STATE])
  {
    mouseEvent(keyUp);

    // Remove key from the list of pressed keys.
    if (keyUp == MOUSEEVENTF_LEFTUP)
    {
      erasePressedKey(VK_LBUTTON);
    }
    else if (keyUp == MOUSEEVENTF_RIGHTUP)
    {
      erasePressedKey(VK_RBUTTON);
    }
    else if (keyUp == MOUSEEVENTF_MIDDLEUP)
    {
      erasePressedKey(VK_MBUTTON);
    }
  }

  /*if (_xboxClickIsDownLong[STATE])
  {
    mouseEvent(keyDown | keyUp);
    mouseEvent(keyDown | keyUp);
  }*/
}

// Description:
//   Callback function used for the EnumWindows call to determine if we
//     have found the On-Screen Keyboard window.
//
// Params:
//   curWnd   The current window to check
//   lParam   A callback parameter used to store the window if it is found
//
// Returns:
//   FALSE when the the desired window is found.
static BOOL CALLBACK EnumWindowsProc(HWND curWnd, LPARAM lParam)
{
  TCHAR title[256];
  // Check to see if the window title matches what we are looking for.
  if (GetWindowText(curWnd, title, 256) && !_tcscmp(title, _T("On-Screen Keyboard")))
  {
    *(HWND*)lParam = curWnd;
    return FALSE;  // Correct window found, stop enumerating through windows.
  }

  return TRUE;
}

// Description:
//   Finds the On-Screen Keyboard if it is open.
//
// Returns:
//   If found, the handle to the On-Screen Keyboard handle. Otherwise, returns NULL.
HWND Gopher::getOskWindow()
{
  HWND ret = NULL;
  EnumWindows(EnumWindowsProc, (LPARAM)&ret);
  return ret;
}

// Description:
//   Removes an entry for a pressed key from the list.
//
// Params:
//   key  The key value to remove from the pressed key list. 
//
// Returns:
//   True if the given key was found and removed from the list.
bool Gopher::erasePressedKey(WORD key)
{
  for (std::list<WORD>::iterator it = _pressedKeys.begin();
       it != _pressedKeys.end();
       ++it)
  {
    if (*it == key)
    {
      _pressedKeys.erase(it);
      return true;
    }
  }

  return false;
}
