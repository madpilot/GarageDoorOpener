function renderSSID(e){var s=$("#ssid")[0];e.ui.ssid.disabled?s.setAttribute("disabled","disabled"):s.removeAttribute("disabled");var a=e.aps;if(a.length>0){for(var t=[],n=0;n<a.length;n++){var i=a[n]==e.ui.ssid.selected;t.push('<option value="'+a[n].ssid+'"'+(i?" selected":"")+">"+a[n].ssid+"</option>")}s.innerHTML=t.join("")}else s.innerHTML="<option>Scanning...</option>"}function addClass(e,s){e.className.indexOf(s)==-1&&(e.className+=" "+s)}function removeClass(e,s){for(var a=e.className.split(" "),t=[],n=0;n<a.length;n++)a[n]!=s&&t.push(a[n]);e.className=t.join(" ")}function renderPasskey(e){var s=$("#passkey")[0],a=$("#passkey-wrapper")[0],t=$("#passkey-error")[0];s.value=e.ui.passkey.value,removeClass(a,"hidden"),e.ui.passkey.visible||addClass(a,"hidden"),removeClass(t,"hidden"),e.ui.passkey.changed&&null!==e.ui.passkey.error?t.innerHTML=e.ui.passkey.error:addClass(t,"hidden")}function renderButton(e){var s=$("#save")[0];s.innerHTML=e.ui.button.label,e.ui.button.disabled?s.setAttribute("disabled","disabled"):s.removeAttribute("disabled")}function updatePasskey(e){""==e.ui.passkey.value&&e.ui.passkey.changed?e.ui.passkey.error="is required":e.ui.passkey.value.length>32?e.ui.passkey.error="too long":e.ui.passkey.error=null}function updateButton(e){""==e.ui.passkey.value||e.ui.passkey.value.length>32?e.ui.button.disabled=!0:e.ui.button.disabled=!1}function updateEncryption(e){e.ui.ssid.selected&&7==e.ui.ssid.selected.encryption?(e.ui.passkey.visible=!1,e.ui.button.disabled=!1):(e.ui.passkey.visible=!0,updatePasskey(e))}function update(e){updateEncryption(e),updatePasskey(e),updateButton(e),render(e)}function render(e){renderSSID(e),renderPasskey(e),renderButton(e)}function browse(e){var s=new XMLHttpRequest;s.open("GET","/browse.json"),s.onreadystatechange=function(){4==s.readyState&&200==s.status&&e(JSON.parse(s.responseText))},s.send()}function save(e){var s=$("#ssid")[0].value,a=$("#passkey")[0].value,t="ssid="+s+"&passkey="+a,n=new XMLHttpRequest;n.open("POST","/save"),n.onreadystatechange=function(){4==n.readyState&&200==n.status&&success(),4==n.readyState&&422==n.status&&error(n.responseText)},n.send(t)}function selectAP(e){state.ui.ssid.selected=e}function changeAP(e){for(var s=e.target.value,a=state.aps,t=null,n=0;n<a.length;n++)a[n].ssid==s&&(t=a[n]);selectAP(t),update(state)}function changePasskey(e){e.preventDefault(),state.ui.passkey.value=e.target.value,state.ui.passkey.changed=!0,update(state)}var $=function(e){return document.querySelectorAll(e)},state={ui:{ssid:{disabled:!0,selected:null},passkey:{visible:!1,changed:!1,error:null,value:""},button:{disabled:!0,label:"Save"}},aps:[]};browse(function(e){state.aps=e,e.length>0?(state.ui.ssid.disabled=!1,selectAP(e[0])):(state.ui.ssid.disabled=!0,selectAP(null)),update(state)}),$("#ssid")[0].addEventListener("change",changeAP,!0),$("#passkey")[0].addEventListener("input",changePasskey,!0),update(state);