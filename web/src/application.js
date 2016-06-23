(function() {
  // Constants
  // Minify will turn these in to short variables
  var FETCH_APS = 1;
  var FETCHING_APS = 2;
  var FETCHED_APS = 3;
  var CHANGE_AP = 4;
  var CHANGE_PASSKEY = 5;
  var NOT_SCANNED = 6;
  var SCANNING = 7;
  var SCANNING_COMPLETE = 8;
  var SAVING = 9;
  var CONNECTED = 10;
  var CONNECTION_ERROR = 11;

  // Initial State
  var state = {
    ui: {
      passkey: {
        changed: false,
        valid: false,
        error: null,
        value: ''
      }
    },
    aps: [],
    ap: null,
    error: '',
    connection: NOT_SCANNED
  };

  function assign() {
    return Object.assign.apply(this, arguments);
  }

  function ajax(url, method, data, success, error) {
    var xhr = new XMLHttpRequest();
    xhr.open(method, url);
    xhr.onreadystatechange = function () {
      if (xhr.readyState == 4) {
        if(xhr.status == 200) {
          success(xhr.responseText);
        } else if(xhr.status == 422) {
          error(xhr.responseText);
        }
      }
    }

    if(data) {
      xhr.send(data);
    } else {
      xhr.send();
    }
  }

  // Reducers
  function reduce_ap(state, action) {
    switch(action.type) {
    case SCANNING_COMPLETE:
      if(action.aps.length == 0) {
        return null;
      } else {
        return assign({}, action.aps[0]);
      }

    case CHANGE_AP:
      return assign({}, action.ap);
    }

    return state;
  }

  function reduce_ui_passkey(state, action) {
    if(action.type == CHANGE_PASSKEY) {
      var valid = action.value.length > 0 && action.value.length <= 32;

      return assign({}, state, {
        value: action.value,
        valid: valid,
        changed: true
      });
    }

    return state;
  }

  function reduce_error(state, action) {
    if(action.type == CONNECTION_ERROR) {
      return action.message
    }
    
    return state;
  }

  function reduce_aps(state, action) {
    if(action.type == SCANNING_COMPLETE) {
      return action.aps;
    }

    return state;
  }

  function reduce_connection(state, action) {
    // Since these types are literally used as state flags,
    // we can just return the type
    switch(action.type) {
    case SCANNING:
    case SCANNING_COMPLETE:
    case CONNECTED:
    case CONNECTION_ERROR:
      return action.type;
    default:
      return state;
    }
  }

  function dispatch(action) {
    var oldState = assign({}, state);
    state = assign({}, oldState);
    var ui = assign({}, state.ui)

    ui.passkey = reduce_ui_passkey(ui.passkey, action);

    state.error = reduce_error(state.error, action);
    state.ap = reduce_ap(state.ap, action);
    state.aps = reduce_aps(state.aps, action);
    state.connection = reduce_connection(state.connection, action);
    state.ui = ui;

    render(state, oldState);
  }

  function getElementById(id) {
    return document.getElementById(id);
  }

  function addClass(el, className) {
    if(el.className.indexOf(className) === -1) {
      var a = el.className.split(' ');
      a.push(className);
      el.className = a.join(' ');
    }
  }

  function removeClass(el, className) {
    var a = el.className.split(' ');
    var index = a.indexOf(className);

    if(index !== -1) {
      a.splice(index, 1)
      el.className = a.join(' ');
    }
  }

  var DISABLED = 'disabled';
  var HIDDEN = 'hidden';
  function enable(el) {
    el.removeAttribute(DISABLED);
  }

  function disable(el) {
    el.setAttribute(DISABLED, DISABLED);
  }

  function show(el) {
    removeClass(el, HIDDEN);
  }

  function hide(el) {
    addClass(el, HIDDEN);
  }

  function innerHTML(el, text) {
    el.innerHTML = text;
  }

  function render_ssid_enabled(state, old) {
    if(state.connection === old.connection) return;

    var ssid = getElementById('ssid');
    switch(state.connection) {
      case NOT_SCANNED:
      case SCANNING:
        disable(ssid);
        break;
      default:
        enable(ssid);
    }
  }

  function render_ssid_aps(state, old) {
    if(state.connection === old.connection && state.aps === old.aps && state.ap === old.ap) return;

    var ssid = getElementById('ssid');
    var aps = state.aps;
    var connection = state.connection;
    var selected = state.ap;

    if(connection === SCANNING) {
      innerHTML(ssid, '<option>Scanning...</option>');
    } else if(connection === NOT_SCANNED || aps.length == 0) {
      innerHTML(ssid, '');
    } else {
      var html = [];
      for(var i = 0; i < aps.length; i++) {
        var ap = aps[i];
        html.push("<option value=\"" + ap.ssid + "\"" + (selected.ssid == ap.ssid ? " selected" : "") + ">" + ap.ssid + "</option>");
      }
      innerHTML(ssid, html.join(''));
    }
  }

  function render_passkey_value(state, old) {
    if(state.ui.passkey.value === old.ui.passkey.value) return;

    var passkey = getElementById('passkey');
    passkey.value = state.ui.passkey.value;
  }

  function render_passkey_visible(state, old) {
    var encrypted = state.ap ? state.ap.encryption : null;
    var oldEncrypted = old.ap ? old.ap.encryption : null;

    var same = state.ui.passkey.value === old.ui.passkey.value;
    same = same && encrypted === oldEncrypted;

    if(same) return;

    var passkey = getElementById('passkey-wrapper');

    if(encrypted == 7) {
      hide(passkey);
    } else {
      show(passkey);
    }
  }

  function render_passkey_error(state, old) {
    var same = state.ui.passkey.changed === old.ui.passkey.changed;
    same = same && state.ui.passkey.valid === old.ui.passkey.valid;
    same = same && state.ui.passkey.value === old.ui.passkey.value;

    if(same) return;
    var value = state.ui.passkey.value;

    var passkeyError = getElementById('passkey-error');
    if(state.ui.passkey.changed && !state.ui.passkey.valid) {
      if(value.length == 0) {
        innerHTML(passkeyError, 'is required');
      } else if(value.length >= 32) {
        innerHTML(passkeyError, 'is too long');
      }
      show(passkeyError);
    } else {
      hide(passkeyError);
    }
  }

  function render_button_disabled(state, old) {
    var encryption = state.ap ? state.ap.encryption : null;
    var oldEncryption = old.ap ? old.ap.encryption: null;

    var same = state.ui.passkey.valid === old.ui.passkey.valid;
    same = same && state.connection === old.connection;
    same = same && encryption === oldEncryption;

    if(same) return;

    var button = getElementById('button');

    if(state.connection === SCANNING_COMPLETE && (encryption === 7 || state.ui.passkey.valid)) {
      enable(button);
    } else {
      disable(button);
    }
  }

  function render_button_value(state, old) {
    if(state.connection === old.connection) return;

    var button = getElementById('button');
    if(state.connection === SAVING) {
      innerHTML(button, "Connecting...");
    } else {
      innerHTML(button, "Connect");
    }
  }

  function render_form_visible(state, old) {
    if(state.connection === old.connection) return;

    var form = getElementById('form');
    if(state.connection === CONNECTED) {
      hide(form);
    } else {
      show(form);
    }
  }

  function render_notification(state, old) {
    if(state.connection === old.connection) return;

    var notification = getElementById('notification');
    if(state.connection === CONNECTED) {
      show(notification);
    } else {
      hide(notification);
    }
  }

  function render_error(state, old) {
    if(state.connection === old.connection) return;

    var error = getElementById('error');
    if(state.connection === CONNECTION_ERROR) {
      error.innerHTML = state.error;
      show(error);
    } else {
      hide(error);
    }
  }

  // Can a function for each element - if the returned dom representation is different
  // to the current dom representation, update the element
  function render(state, old) {
    render_ssid_enabled(state, old);
    render_ssid_aps(state, old);
    render_passkey_value(state, old);
    render_passkey_visible(state, old);
    render_passkey_error(state, old);
    render_button_disabled(state, old);
    render_button_value(state, old);
    render_form_visible(state, old);
    render_notification(state, old);
    render_error(state, old);
  }

  function browse() {
    dispatch({ type: SCANNING });

    ajax("/browse.json", "GET", null, function(text) {
      dispatch({ type: SCANNING_COMPLETE, aps: JSON.parse(text) });
    });
  }

  function changeAP(event) {
    event.preventDefault();

    var ap = null;
    var aps = state.aps;
    for(var i = 0; i < aps.length; i++) {
      if(aps[i].ssid == event.target.value) {
        ap = aps[i];
      }
    }

    dispatch({
      type: CHANGE_AP,
      ap: ap
    });
  }

  function changePasskey(event) {
    event.preventDefault();

    dispatch({
      type: CHANGE_PASSKEY,
      value: event.target.value
    });
  }

  function onSave(event) {
    event.preventDefault();

    var ssid = state.ap.ssid;
    var passkey = state.ui.passkey.value;

    var data = "ssid=" + ssid;
    if(state.ap.encryption != 7) {
      data += "&passkey=" + passkey;
    }

    dispatch({ type: SAVING });

    ajax("/save", "POST", data, function() {
      dispatch({ type: CONNECTED });
    }, function(message) {
      dispatch({ type: CONNECTION_ERROR, message: message });
    });
  }

  getElementById('ssid').addEventListener('change', changeAP, true);
  getElementById('passkey').addEventListener('input', changePasskey, true);
  getElementById('form').addEventListener('submit', onSave, true);

  browse();
})();
