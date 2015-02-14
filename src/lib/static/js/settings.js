var Billion = 2 * 1000 * 1000 * 1000;

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

function reloadPage() {
    document.location.reload(true);
}

function changeLocale() {
    var sel = document.getElementById("localeChangeSelect");
    var ln = sel.options[sel.selectedIndex].value;
    setCookie("locale", ln, {
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
