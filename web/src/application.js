// Alias this so the minifier will rename it.
var $ = function(s) { return document.querySelectorAll(s) }
var state = {
  ui: {
    ssid: {
      disabled: true,
      selected: null
    },
    passkey: {
      visible: false,
      changed: false,
      error: null,
      value: ''
    },
    button: {
      disabled: true,
      label: "Save"
    }
  },
  aps: []
};

function renderSSID(state) {
  var ssid = $('#ssid')[0];

  if(state.ui.ssid.disabled) {
    ssid.setAttribute('disabled', 'disabled');
  } else {
    ssid.removeAttribute('disabled');
  }

  var aps = state.aps;
  if(aps.length > 0) {
    var html = [];
    for(var i = 0; i < aps.length; i++) {
      var selected = aps[i] == state.ui.ssid.selected;
      html.push("<option value=\"" + aps[i].ssid + "\"" + (selected ? " selected" : "") + ">" + aps[i].ssid + "</option>");
    }
    ssid.innerHTML = html.join('');
  } else {
    ssid.innerHTML = '<option>Scanning...</option>';
  }
}

function addClass(el, className) {
  if(el.className.indexOf(className) == -1) {
    el.className += ' ' + className;
  }
}

function removeClass(el, className) {
  var classes = el.className.split(' ');
  var newClasses = [];
  for(var i = 0; i < classes.length; i++) {
    if(classes[i] != className) {
      newClasses.push(classes[i]);
    }
  }
  el.className = newClasses.join(' ');
}

function renderPasskey(state) {
  var passkey = $('#passkey')[0];
  var passkeyWrapper = $('#passkey-wrapper')[0];
  var passkeyError = $('#passkey-error')[0];

  passkey.value = state.ui.passkey.value;

  removeClass(passkeyWrapper, 'hidden');
  if(!state.ui.passkey.visible) {
    addClass(passkeyWrapper, 'hidden');
  }

  removeClass(passkeyError, 'hidden');
  if(state.ui.passkey.changed && state.ui.passkey.error !== null) {
    passkeyError.innerHTML = state.ui.passkey.error;
  } else {
    addClass(passkeyError, 'hidden');
  }
}

function renderButton(state) {
  var button = $('#save')[0];
  button.innerHTML = state.ui.button.label;

  if(state.ui.button.disabled) {
    button.setAttribute('disabled', 'disabled');
  } else {
    button.removeAttribute('disabled');
  }
}

function updatePasskey(state) {
  if(state.ui.passkey.value == '' && state.ui.passkey.changed) {
    state.ui.passkey.error = "is required";
  } else if(state.ui.passkey.value.length > 32) {
    state.ui.passkey.error = "too long";
  } else {
    state.ui.passkey.error = null;
  }
}

function updateButton(state) {
  if(state.ui.passkey.value == '' || state.ui.passkey.value.length > 32) {
    state.ui.button.disabled = true;
  } else {
    state.ui.button.disabled = false;
  }
}

function updateEncryption(state) {
  if(state.ui.ssid.selected && state.ui.ssid.selected.encryption == 7) {
    state.ui.passkey.visible = false;
    state.ui.button.disabled = false;
  } else {
    state.ui.passkey.visible = true;
    updatePasskey(state);
  }
}

function update(state) {
  updateEncryption(state);
  updatePasskey(state);
  updateButton(state);

  render(state);
}

function render(state) {
  renderSSID(state);
  renderPasskey(state);
  renderButton(state);
}

function browse(cb) {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/browse.json");
  xhr.onreadystatechange = function () {
    if (xhr.readyState != 4 || xhr.status != 200) return;
    cb(JSON.parse(xhr.responseText));
  }
  xhr.send();
}

function save(cb) {
  var ssid = $("#ssid")[0].value;
  var passkey = $("#passkey")[0].value;

  var data = "ssid=" + ssid + "&passkey=" + passkey;

  var xhr = new XMLHttpRequest();
  xhr.open("POST", "/save");
  xhr.onreadystatechange = function() {
    if(xhr.readyState == 4 && xhr.status == 200) {
      success();
    }

    if(xhr.readyState == 4 && xhr.status == 422) {
      error(xhr.responseText);
    }
  }
  xhr.send(data);
}

function selectAP(ap) {
  state.ui.ssid.selected = ap;
}

function changeAP(event) {
  var value = event.target.value;
  var aps = state.aps;

  var ap = null;
  for(var i = 0; i < aps.length; i++) {
    if(aps[i].ssid == value) {
      ap = aps[i];
    }
  }

  selectAP(ap);
  update(state);
}

function changePasskey(event) {
  event.preventDefault();
  state.ui.passkey.value = event.target.value;
  state.ui.passkey.changed = true;
  update(state);
}

browse(function(r) {
  state.aps = r;
  if(r.length > 0) {
    state.ui.ssid.disabled = false;
    selectAP(r[0]);
  } else {
    state.ui.ssid.disabled = true;
    selectAP(null);
  }

  update(state);
});

$('#ssid')[0].addEventListener('change', changeAP, true);
$('#passkey')[0].addEventListener('input', changePasskey, true);
update(state);
