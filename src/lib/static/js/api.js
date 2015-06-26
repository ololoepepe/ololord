/*ololord global object*/

var lord = lord || {};

/*Constants*/

lord.Second = 1000;
lord.Minute = 60 * lord.Second;
lord.Hour = 60 * lord.Minute;
lord.Day = 24 * lord.Hour;
lord.Year = 365 * lord.Day;
lord.Billion = 2 * 1000 * 1000 * 1000;

lord._defineEnum = function(constName, value) {
    if (typeof constName != "string")
        return;
    if (value) {
        lord[constName] = value;
        lord._lastEnumValue = value;
    } else if (typeof lord._lastEnumValue == "number") {
        lord._lastEnumValue += 1;
        lord[constName] = lord._lastEnumValue;
    }
};

lord._defineEnum("RpcBanUserId", 1);
lord._defineEnum("RpcDeleteFileId");
lord._defineEnum("RpcDeletePostId");
lord._defineEnum("RpcEditAudioTagsId");
lord._defineEnum("RpcEditPostId");
lord._defineEnum("RpcGetBoardsId");
lord._defineEnum("RpcGetCaptchaQuotaId");
lord._defineEnum("RpcGetFileExistenceId");
lord._defineEnum("RpcGetFileMetaDataId");
lord._defineEnum("RpcGetNewPostCountId");
lord._defineEnum("RpcGetNewPostCountExId");
lord._defineEnum("RpcGetNewPostsId");
lord._defineEnum("RpcGetPostId");
lord._defineEnum("RpcGetThreadNumbersId");
lord._defineEnum("RpcGetYandexCaptchaImageId");
lord._defineEnum("RpcSetThreadFixedId");
lord._defineEnum("RpcSetThreadOpenedId");
lord._defineEnum("RpcSetVoteOpenedId");
lord._defineEnum("RpcUnvoteId");
lord._defineEnum("RpcVoteId");

/*Variables*/

lord.popups = [];
lord.unloading = false;

/*Functions*/

lord.getCookie = function(name) {
    var matches = document.cookie.match(
        new RegExp("(?:^|; )" + name.replace(/([\.$?*|{}\(\)\[\]\\\/\+^])/g, '\\$1') + "=([^;]*)"));
    return matches ? decodeURIComponent(matches[1]) : undefined;
};

lord.setCookie = function(name, value, options) {
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
    for (var propName in options) {
        updatedCookie += "; " + propName;
        var propValue = options[propName];
        if (propValue !== true)
            updatedCookie += "=" + propValue;
    }
    document.cookie = updatedCookie;
};

lord.deleteCookie = function(name) {
    lord.setCookie(name, "", {expires: -1});
};

lord.getLocalObject = function(key, defValue) {
    if (!key || typeof key != "string")
        return null;
    try {
        var val = localStorage.getItem(key);
        return (null != val) ? JSON.parse(val) : defValue;
    } catch (ex) {
        return null;
    }
};

lord.setLocalObject = function(key, value) {
    if (!key || typeof key != "string")
        return false;
    try {
        if (null != value && typeof value != "undefined")
            localStorage.setItem(key, JSON.stringify(value));
        else
            localStorage.setItem(key, null);
        return true;
    } catch (ex) {
        return false;
    }
};

lord.removeLocalObject = function(key) {
    if (!key || typeof key != "string")
        return;
    try {
        return localStorage.removeItem(key);
    } catch (ex) {
        //
    }
};

lord.in = function(arr, obj, strict) {
    if (!arr || !arr.length)
        return false;
    for (var i = 0; i < arr.length; ++i) {
        if ((strict && obj === arr[i]) || (!strict && obj == arr[i]))
            return true;
    }
    return false;
};

lord.arr = function(obj) {
    var arr = [];
    if (!obj || !obj.length)
        return arr;
    for (var i = 0; i < obj.length; ++i)
        arr.push(obj[i]);
    return arr;
};

lord.hasOwnProperties = function(obj) {
    if (!obj)
        return false;
    for (x in obj) {
        if (obj.hasOwnProperty(x))
            return true;
    }
    return false;
};

lord.forIn = function(obj, f) {
    if (!obj || typeof f != "function")
        return;
    for (x in obj) {
        if (obj.hasOwnProperty(x))
            f(obj[x], x);
    }
};

lord.last = function(arr) {
    if (!arr || !arr.length)
        return null;
    return arr[arr.length - 1];
};

lord.id = function(id) {
    if (typeof id != "string")
        return null;
    return document.getElementById(id);
};

lord.text = function(id) {
    var input = lord.id(id);
    return input ? input.value : "";
};

lord.query = function(query, parent) {
    if (typeof query != "string")
        return null;
    if (!parent)
        parent = document;
    var elements = parent.querySelectorAll(query);
    var list = [];
    if (!elements)
        return list;
    for (var i = 0; i < elements.length; ++i)
        list.push(elements[i]);
    return list;
};

lord.queryOne = function(query, parent) {
    if (typeof query != "string")
        return null;
    if (!parent)
        parent = document;
    return parent.querySelector(query);
};

lord.name = function(name, parent) {
    return lord.query("[name='" + name + "']", parent);
};

lord.nameOne = function(name, parent) {
    return lord.queryOne("[name='" + name + "']", parent);
};

lord.contains = function(s, subs) {
    if (typeof s != "string" || typeof subs != "string")
        return false;
    return s.replace(subs, "") != s;
};

lord.addClass = function(element, classNames) {
    if (!element || !element.tagName || !classNames || typeof classNames != "string")
        return;
    lord.arr(classNames.split(" ")).forEach(function(className) {
        if (!className)
            return;
        if (lord.hasClass(element, className))
            return;
        if (element.className)
            element.className += " " + className;
        else
            element.className = className;
    });
};

lord.hasClass = function(element, className) {
    if (!element || !element.tagName || !className || typeof className != "string")
        return false;
    return !!element.className.match(new RegExp("(^| )" + className + "( |$)"));
};

lord.removeClass = function(element, classNames) {
    if (!element || !element.tagName || !classNames || typeof classNames != "string")
        return;
    lord.arr(classNames.split(" ")).forEach(function(className) {
        if (!className)
            return;
        element.className = element.className.replace(new RegExp("(^| )" + className + "$"), "");
        element.className = element.className.replace(new RegExp("^" + className + "( |$)"), "");
        element.className = element.className.replace(new RegExp(" " + className + " "), " ");
    });
};

lord.node = function(type, text) {
    if (typeof type != "string")
        return null;
    type = type.toUpperCase();
    return ("TEXT" == type) ? document.createTextNode(text ? text : "") : document.createElement(type);
};

lord.toCenter = function(element, sizeHintX, sizeHintY) {
    var doc = document.documentElement;
    if (!sizeHintX || sizeHintX <= 0)
        sizeHintX = +element.offsetWidth;
    if (!sizeHintY  || sizeHintY <= 0)
        sizeHintY = +element.offsetHeight;
    element.style.left = (doc.clientWidth / 2 - sizeHintX / 2) + "px";
    element.style.top = (doc.clientHeight / 2 - sizeHintY / 2) + "px";
};

lord.reloadPage = function() {
    document.location.reload(true);
};

lord.showPopup = function(text, options) {
    if (!text)
        return;
    var timeout = (options && !isNaN(+options.timeout)) ? +options.timeout : 5 * 1000;
    var classNames = (options && typeof options.classNames == "string") ? options.classNames : "";
    if (options && typeof options.type == "string" && lord.in(["critical", "warning"], options.type.toLowerCase()))
        classNames += options.type.toLowerCase() + (("" != classNames) ? " " : "");
    var msg = lord.node("div");
    lord.addClass(msg, "popup");
    lord.addClass(msg, classNames);
    if (lord.popups.length > 0) {
        var prev = lord.popups[lord.popups.length - 1];
        msg.style.top = (prev.offsetTop + prev.offsetHeight + 5) + "px";
    }
    msg.appendChild(lord.node("text", text));
    document.body.appendChild(msg);
    lord.popups.push(msg);
    setTimeout(function() {
        var offsH = msg.offsetHeight + 5;
        document.body.removeChild(msg);
        var ind = lord.popups.indexOf(msg);
        if (ind < 0)
            return;
        lord.popups.splice(ind, 1);
        for (var i = 0; i < lord.popups.length; ++i) {
            var top = +lord.popups[i].style.top.replace("px", "");
            top -= offsH;
            lord.popups[i].style.top = top + "px";
        }
    }, timeout);
};

lord.showDialog = function(title, label, body, callback, afterShow) {
    var root = lord.node("div");
    if (!!title || !!label) {
        var div = lord.node("div");
        if (!!title) {
            var c = lord.node("center");
            var t = lord.node("b");
            t.appendChild(lord.node("text", title));
            c.appendChild(t);
            div.appendChild(c);
            div.appendChild(lord.node("br"));
        }
        if (!!label) {
            div.appendChild(lord.node("text", label));
            div.appendChild(lord.node("br"));
        }
        root.appendChild(div);
        root.appendChild(lord.node("br"));
    }
    if (!!body) {
        root.appendChild(body);
        root.appendChild(lord.node("br"));
    }
    var div2 = lord.node("div");
    var dialog = null;
    var cancel = lord.node("button");
    cancel.onclick = function() {
        dialog.close();
    };
    cancel.innerHTML = lord.text("cancelButtonText");
    div2.appendChild(cancel);
    var ok = lord.node("button");
    ok.onclick = function() {
        if (!!callback)
            callback();
        dialog.close();
    };
    ok.innerHTML = lord.text("confirmButtonText");
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
};

lord.ajaxRequest = function(method, params, id, callback, errorCallback) {
    var xhr = new XMLHttpRequest();
    xhr.withCredentials = true;
    var prefix = lord.text("sitePathPrefix");
    xhr.open("post", "/" + prefix + "api");
    xhr.setRequestHeader("Content-Type", "application/json");
    var request = {
        "method": method,
        "params": params,
        "id": id
    };
    xhr.onreadystatechange = function() {
        if (xhr.readyState === 4) {
            if (xhr.status === 200) {
                var response = JSON.parse(xhr.responseText);
                var err = response.error;
                if (!!err) {
                    lord.showPopup(err, {type: "critical"});
                    if (typeof errorCallback == "function")
                        errorCallback();
                    return;
                }
                if (typeof callback == "function")
                    callback(response.result);
            } else {
                if (!lord.unloading) {
                    var text = lord.text("ajaxErrorText") + " " + xhr.status;
                    switch (+xhr.status) {
                    case 413:
                        text = lord.text("error" + xhr.status + "Text");
                        break;
                    default:
                        break;
                    }
                    lord.showPopup(text, {type: "critical"});
                    if (typeof errorCallback == "function")
                        errorCallback();
                }
            }
        }
    };
    xhr.send(JSON.stringify(request));
};

lord.isHashpass = function(s) {
    return !!s.match(/([0-9a-fA-F]{8}\-){4}[0-9a-fA-F]{8}/g);
};

lord.toHashpass = function(s) {
    if (!s)
        return "";
    var hash = CryptoJS.SHA1(s).toString(CryptoJS.enc.Hex);
    var parts = hash.match(/.{1,8}/g);
    return parts.join("-");
};
