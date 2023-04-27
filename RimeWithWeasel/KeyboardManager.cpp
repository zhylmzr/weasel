#include "stdafx.h"
#include <KeyboardManager.h>
#include <logging.h>

HHOOK KeyboardManager::m_hook_handle;
RimeWithWeaselHandler *KeyboardManager::pHandler;
CComPtr<ITfCompartment> KeyboardManager::spStatusComp;

KeyboardManager::KeyboardManager(RimeWithWeaselHandler *handler) { pHandler = handler; }

KeyboardManager::~KeyboardManager() { StopHook(); }

void KeyboardManager::StartHook() {
    if (!m_hook_handle) {
        m_hook_handle = SetWindowsHookEx(WH_KEYBOARD_LL, _HookProc, GetModuleHandle(NULL), NULL);
        if (!m_hook_handle) {
            DWORD errorCode = GetLastError();
            LOG(ERROR) << "StartLowlevelKeyboardHook failed, error code: " << errorCode;
        }
    }
}

void KeyboardManager::StopHook() {
    if (m_hook_handle) {
        UnhookWindowsHookEx(m_hook_handle);
        m_hook_handle = nullptr;
    }
}

LRESULT CALLBACK KeyboardManager::_HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *kb = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
        if (kb->dwExtraInfo == SUPPRESS_FLAG) {
            return CallNextHookEx(m_hook_handle, nCode, wParam, lParam);
        }

        if (kb->vkCode == VK_CAPITAL) {
            if (wParam == WM_KEYUP && pHandler) {
                auto old_status = pHandler->GlobalToggleAsciiMode();
                bool ascii_mode = !old_status.ascii_mode;
                SetAsciiModeIcon(ascii_mode, ascii_mode && old_status.composing);
            }
            return 1;
        }
    }
    return CallNextHookEx(m_hook_handle, nCode, wParam, lParam);
}

void KeyboardManager::SetAsciiModeIcon(bool ascii_mode, bool send_key) {
    InitStatusComponent();
    if (spStatusComp) {
        VARIANT var;
        VariantInit(&var);
        var.vt = VT_I4;
        var.intVal = (int)ascii_mode;
        HRESULT hr = spStatusComp->SetValue(0, &var);
        VariantClear(&var);

        // trigger update ui
        if (ascii_mode && send_key) {
            const int LEN = 4;
            INPUT inputs[LEN];
            ZeroMemory(inputs, sizeof(inputs));
            for (int i = 0; i < LEN; i++) {
                inputs[i].type = INPUT_KEYBOARD;
            }
            inputs[0].ki.wVk = inputs[1].ki.wVk = VK_SPACE;
            inputs[2].ki.wVk = inputs[3].ki.wVk = VK_BACK;
            inputs[1].ki.dwFlags = inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(LEN, inputs, sizeof(INPUT));
        }
    }
}

void KeyboardManager::InitStatusComponent() {
    if (spStatusComp != nullptr) {
        return;
    }

    CComPtr<ITfThreadMgr> spThreadMgr;
    CComPtr<ITfCompartmentMgr> spCompMgr;

    DWORD hr =
        CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void **)&spThreadMgr);
    if (FAILED(hr)) {
        goto faild;
    }
    hr = spThreadMgr->GetGlobalCompartment(&spCompMgr);
    if (FAILED(hr)) {
        goto faild;
    }
    hr = spCompMgr->GetCompartment(c_guidStatus, &spStatusComp);

    if (FAILED(hr)) {
        goto faild;
    }

    return;
faild:
    LOG(ERROR) << "InitStatusComponent failed: " << hr;
    return;
}
