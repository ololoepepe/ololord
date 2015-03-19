var Billion = 2 * 1000 * 1000 * 1000;

function getCookie(name) {
    var matches = document.cookie.match(
        new RegExp("(?:^|; )" + name.replace(/([\.$?*|{}\(\)\[\]\\\/\+^])/g, '\\$1') + "=([^;]*)"));
    return matches ? decodeURIComponent(matches[1]) : undefined;
}

function setCookie(name, value, options) {
    options = options || {};
    var expires = options.expires;
    if (typeof expires == "number" && expires) {
        var d = new Date();
        d.setTime(d.getTime() + expires * 1000);
        expires = options.expires = d;
    }
    if (expires && expires.toUTCString)
        options.expires = expires.toUTCString();
    value = encodeURIComponent(value);
    var updatedCookie = name + "=" + value;
    for(var propName in options) {
        updatedCookie += "; " + propName;
        var propValue = options[propName];
        if (propValue !== true)
            updatedCookie += "=" + propValue;
    }
    document.cookie = updatedCookie;
}

function deleteCookie(name) {
    setCookie(name, "", {expires: -1});
}

function reloadPage() {
    document.location.reload(true);
}

function showDialog(title, label, body, callback, afterShow) {
    var root = document.createElement("div");
    if (!!title && !!label) {
        var div = document.createElement("div");
        if (!!title) {
            var c = document.createElement("center");
            var t = document.createElement("b");
            t.appendChild(document.createTextNode(title));
            c.appendChild(t);
            div.appendChild(c);
            div.appendChild(document.createElement("br"));
        }
        if (!!label) {
            div.appendChild(document.createTextNode(label));
            div.appendChild(document.createElement("br"));
        }
        root.appendChild(div);
        root.appendChild(document.createElement("br"));
    }
    if (!!body)
        root.appendChild(body);
    var div2 = document.createElement("div");
    var dialog = null;
    var cancel = document.createElement("button");
    cancel.onclick = function() {
        dialog.close();
    };
    cancel.innerHTML = document.getElementById("cancelButtonText").value;
    div2.appendChild(cancel);
    var ok = document.createElement("button");
    ok.onclick = function() {
        if (!!callback)
            callback();
        dialog.close();
    };
    ok.innerHTML = document.getElementById("confirmButtonText").value;
    div2.appendChild(ok);
    root.appendChild(div2);
    dialog = picoModal({
        "content": root
    }).afterShow(function(modal) {
        if (!!afterShow)
            afterShow();
    }).afterClose(function(modal) {
        modal.destroy();
    });
    dialog.show();
}

function changeLocale() {
    var sel = document.getElementById("localeChangeSelect");
    var ln = sel.options[sel.selectedIndex].value;
    setCookie("locale", ln, {
        "expires": Billion, "path": "/"
    });
    reloadPage();
}

function changeStyle() {
    var sel = document.getElementById("styleChangeSelect");
    var sn = sel.options[sel.selectedIndex].value;
    setCookie("style", sn, {
        "expires": Billion, "path": "/"
    });
    reloadPage();
}

function changeTime() {
    var sel = document.getElementById("timeChangeSelect");
    var ln = sel.options[sel.selectedIndex].value;
    setCookie("time", ln, {
        "expires": Billion, "path": "/"
    });
    reloadPage();
}

function isHashpass(s) {
    return !!s.match(/([0-9a-fA-F]{8}\-){4}[0-9a-fA-F]{8}/g);
}

function toHashpass(s) {
    if (!s)
        return "";
    var hash = CryptoJS.SHA1(s).toString(CryptoJS.enc.Hex);
    var parts = hash.match(/.{1,8}/g);
    return parts.join("-");
}

function doLogin() {
    var pwd = document.getElementById("loginInput").value;
    hashpass = isHashpass(pwd) ? pwd : toHashpass(pwd);
    setCookie("hashpass", hashpass, {
        "expires": Billion, "path": "/"
    });
    reloadPage();
}

function doLogout() {
    setCookie("hashpass", "", {
        "expires": Billion, "path": "/"
    });
    reloadPage();
}

function switchShowLogin() {
    var inp = document.getElementById("loginInput");
    if (inp.type === "password")
        inp.type = "text";
    else if (inp.type === "text")
        inp.type = "password";
}

function switchShowTripcode() {
    var sw = document.getElementById("showTripcodeCheckbox");
    if (!!sw.checked) {
        setCookie("show_tripcode", "true", {
            "expires": Billion, "path": "/"
        });
    } else {
        setCookie("show_tripcode", "", {
            "expires": Billion, "path": "/"
        });
    }
}

function initializeOnLoadSettings() {
    if (getCookie("show_tripcode") === "true")
        document.getElementById("showTripcodeCheckbox").checked = true;
}
