#include <Core/CorePCH.h>

#include <Core/Input/InputManager.h>

nsStringView nsInputManager::ConvertScanCodeToEngineName(nsUInt8 uiScanCode, bool bIsExtendedKey)
{
  const nsUInt8 uiFinalScanCode = bIsExtendedKey ? (uiScanCode + 128) : uiScanCode;

  switch (uiFinalScanCode)
  {
    case 1:
      return nsInputSlot_KeyEscape;
    case 2:
      return nsInputSlot_Key1;
    case 3:
      return nsInputSlot_Key2;
    case 4:
      return nsInputSlot_Key3;
    case 5:
      return nsInputSlot_Key4;
    case 6:
      return nsInputSlot_Key5;
    case 7:
      return nsInputSlot_Key6;
    case 8:
      return nsInputSlot_Key7;
    case 9:
      return nsInputSlot_Key8;
    case 10:
      return nsInputSlot_Key9;
    case 11:
      return nsInputSlot_Key0;
    case 12:
      return nsInputSlot_KeyHyphen;
    case 13:
      return nsInputSlot_KeyEquals;
    case 14:
      return nsInputSlot_KeyBackspace;
    case 15:
      return nsInputSlot_KeyTab;
    case 16:
      return nsInputSlot_KeyQ;
    case 17:
      return nsInputSlot_KeyW;
    case 18:
      return nsInputSlot_KeyE;
    case 19:
      return nsInputSlot_KeyR;
    case 20:
      return nsInputSlot_KeyT;
    case 21:
      return nsInputSlot_KeyY;
    case 22:
      return nsInputSlot_KeyU;
    case 23:
      return nsInputSlot_KeyI;
    case 24:
      return nsInputSlot_KeyO;
    case 25:
      return nsInputSlot_KeyP;
    case 26:
      return nsInputSlot_KeyBracketOpen;
    case 27:
      return nsInputSlot_KeyBracketClose;
    case 28:
      return nsInputSlot_KeyReturn;
    case 29:
      return nsInputSlot_KeyLeftCtrl;
    case 30:
      return nsInputSlot_KeyA;
    case 31:
      return nsInputSlot_KeyS;
    case 32:
      return nsInputSlot_KeyD;
    case 33:
      return nsInputSlot_KeyF;
    case 34:
      return nsInputSlot_KeyG;
    case 35:
      return nsInputSlot_KeyH;
    case 36:
      return nsInputSlot_KeyJ;
    case 37:
      return nsInputSlot_KeyK;
    case 38:
      return nsInputSlot_KeyL;
    case 39:
      return nsInputSlot_KeySemicolon;
    case 40:
      return nsInputSlot_KeyApostrophe;
    case 41:
      return nsInputSlot_KeyTilde;
    case 42:
      return nsInputSlot_KeyLeftShift;
    case 43:
      return nsInputSlot_KeyBackslash;
    case 44:
      return nsInputSlot_KeyZ;
    case 45:
      return nsInputSlot_KeyX;
    case 46:
      return nsInputSlot_KeyC;
    case 47:
      return nsInputSlot_KeyV;
    case 48:
      return nsInputSlot_KeyB;
    case 49:
      return nsInputSlot_KeyN;
    case 50:
      return nsInputSlot_KeyM;
    case 51:
      return nsInputSlot_KeyComma;
    case 52:
      return nsInputSlot_KeyPeriod;
    case 53:
      return nsInputSlot_KeySlash;
    case 54:
      return nsInputSlot_KeyRightShift;
    case 55:
      return nsInputSlot_KeyNumpadStar;
    case 56:
      return nsInputSlot_KeyLeftAlt;
    case 57:
      return nsInputSlot_KeySpace;
    case 58:
      return nsInputSlot_KeyCapsLock;
    case 59:
      return nsInputSlot_KeyF1;
    case 60:
      return nsInputSlot_KeyF2;
    case 61:
      return nsInputSlot_KeyF3;
    case 62:
      return nsInputSlot_KeyF4;
    case 63:
      return nsInputSlot_KeyF5;
    case 64:
      return nsInputSlot_KeyF6;
    case 65:
      return nsInputSlot_KeyF7;
    case 66:
      return nsInputSlot_KeyF8;
    case 67:
      return nsInputSlot_KeyF9;
    case 68:
      return nsInputSlot_KeyF10;
    case 69:
      return nsInputSlot_KeyNumLock;
    case 70:
      return nsInputSlot_KeyScroll;
    case 71:
      return nsInputSlot_KeyNumpad7;
    case 72:
      return nsInputSlot_KeyNumpad8;
    case 73:
      return nsInputSlot_KeyNumpad9;
    case 74:
      return nsInputSlot_KeyNumpadMinus;
    case 75:
      return nsInputSlot_KeyNumpad4;
    case 76:
      return nsInputSlot_KeyNumpad5;
    case 77:
      return nsInputSlot_KeyNumpad6;
    case 78:
      return nsInputSlot_KeyNumpadPlus;
    case 79:
      return nsInputSlot_KeyNumpad1;
    case 80:
      return nsInputSlot_KeyNumpad2;
    case 81:
      return nsInputSlot_KeyNumpad3;
    case 82:
      return nsInputSlot_KeyNumpad0;
    case 83:
      return nsInputSlot_KeyNumpadPeriod;


    case 86:
      return nsInputSlot_KeyPipe;
    case 87:
      return nsInputSlot_KeyF11;
    case 88:
      return nsInputSlot_KeyF12;


    case 91:
      return nsInputSlot_KeyLeftWin;
    case 92:
      return nsInputSlot_KeyRightWin;
    case 93:
      return nsInputSlot_KeyApps;



    case 128 + 16:
      return nsInputSlot_KeyPrevTrack;
    case 128 + 25:
      return nsInputSlot_KeyNextTrack;
    case 128 + 28:
      return nsInputSlot_KeyNumpadEnter;
    case 128 + 29:
      return nsInputSlot_KeyRightCtrl;
    case 128 + 32:
      return nsInputSlot_KeyMute;
    case 128 + 34:
      return nsInputSlot_KeyPlayPause;
    case 128 + 36:
      return nsInputSlot_KeyStop;
    case 128 + 46:
      return nsInputSlot_KeyVolumeDown;
    case 128 + 48:
      return nsInputSlot_KeyVolumeUp;
    case 128 + 53:
      return nsInputSlot_KeyNumpadSlash;
    case 128 + 55:
      return nsInputSlot_KeyPrint;
    case 128 + 56:
      return nsInputSlot_KeyRightAlt;
    case 128 + 70:
      return nsInputSlot_KeyPause;
    case 128 + 71:
      return nsInputSlot_KeyHome;
    case 128 + 72:
      return nsInputSlot_KeyUp;
    case 128 + 73:
      return nsInputSlot_KeyPageUp;
    case 128 + 75:
      return nsInputSlot_KeyLeft;
    case 128 + 77:
      return nsInputSlot_KeyRight;
    case 128 + 79:
      return nsInputSlot_KeyEnd;
    case 128 + 80:
      return nsInputSlot_KeyDown;
    case 128 + 81:
      return nsInputSlot_KeyPageDown;
    case 128 + 82:
      return nsInputSlot_KeyInsert;
    case 128 + 83:
      return nsInputSlot_KeyDelete;

    default:

      // for extended keys fall back to the non-extended name
      if (bIsExtendedKey)
        return ConvertScanCodeToEngineName(uiScanCode, false);

      break;
  }

  return "unknown_key";
}
