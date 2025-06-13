mergeInto(LibraryManager.library, {
  // Called from Lua (via C++) ----------------------------
  LuaBridge_JS_Emit: function(evPtr, dataPtr) {
    const event = UTF8ToString(evPtr);
    const data  = UTF8ToString(dataPtr);
    if (window.defoldBridge && window.defoldBridge._onEmit)
        window.defoldBridge._onEmit(event, data);
  },

  // Called from JS to forward msg to Lua -----------------
  defold_emit_to_lua: function(eventPtr, dataPtr) {
    // eventPtr & dataPtr are already UTF8 in WASM linear memory
    _LuaBridge_Dispatch(eventPtr, dataPtr);
  }
});

// High‑level JS helper (optional)
window.defoldBridge = {
  _onEmit: null,                    // set by Lua side when listener added
  on:   cb => (window.defoldBridge._onEmit = cb),
  emit: (ev, obj) => {
    const json = typeof obj === "string" ? obj : JSON.stringify(obj || {});
    const evPtr   = allocateUTF8(ev);
    const dataPtr = allocateUTF8(json);
    _defold_emit_to_lua(evPtr, dataPtr);   // exported by Emscripten
    _free(evPtr); _free(dataPtr);
  }
};
