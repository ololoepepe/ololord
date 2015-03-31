/*ololord global object*/

var lord = lord || {};

/*Variables*/

lord.postPreviews = {};
lord.lastPostPreview = null;
lord.lastPostPreviewTimer = null;
lord.formSubmitted = null;
lord.popups = [];
lord.images = {};
lord.img = null;
lord.postFormVisible = {
    "Top": false,
    "Bottom": false
};

/*Functions*/

lord.isAudioType = function(type) {
    return type in {"audio/mpeg": true, "audio/ogg": true, "audio/wav": true};
};

lord.isImageType = function(type) {
    return type in {"image/gif": true, "image/jpeg": true, "image/png": true};
};

lord.isVideoType = function(type) {
    return type in {"video/mp4": true, "video/ogg": true, "video/webm": true};
};

lord.isSpecialThumbName = function(thumbName) {
    return lord.isAudioType(thumbName) || lord.isImageType(thumbName) || lord.isVideoType(thumbName);
};

lord.toCenter = function(element, sizeHintX, sizeHintY) {
    var doc = document.documentElement;
    if (!sizeHintX || sizeHintX <= 0)
        sizeHintX = 300;
    if (!sizeHintY  || sizeHintY <= 0)
        sizeHintY = 150;
    element.style.left = (doc.clientWidth / 2 - sizeHintX / 2) + "px";
    element.style.top = (doc.clientHeight / 2 - sizeHintY / 2) + "px";
};

lord.resetScale = function(image) {
    var k = (image.scale / 100);
    var tr = "scale(" + k + ", " + k + ")";
    //Fuck you all who create those stupid browser-specific features
    image.style.webkitTransform = tr;
    image.style.MozTransform = tr;
    image.style.msTransform = tr;
    image.style.OTransform = tr;
    image.style.transform = tr;
};

lord.removeReferences = function(postNumber) {
    postNumber = +postNumber;
    if (isNaN(postNumber))
        return;
    var referencedByTrs = document.querySelectorAll("[name='referencedByTr']");
    if (!referencedByTrs)
        return;
    for (var i = 0; i < referencedByTrs.length; ++i) {
        var referencedByTr = referencedByTrs[i];
        var as = referencedByTr.querySelectorAll("a");
        if (!as)
            continue;
        for (var j = 0; j < as.length; ++j) {
            var a = as[j];
            if (a.innerHTML == ("&gt;&gt;" + postNumber)) {
                a.parentNode.removeChild(a);
                if (as.length < 2)
                    referencedByTr.style.display = "none";
                break;
            }
        }
    }
};

lord.addReferences = function(postNumber, referencedPosts) {
    postNumber = +postNumber;
    if (isNaN(postNumber))
        return;
    if (!referencedPosts)
        return;
    var prefix = document.getElementById("sitePathPrefix").value;
    var currentBoardName = document.getElementById("currentBoardName").value;
    for (key in referencedPosts) {
        if (!referencedPosts.hasOwnProperty(key))
            continue;
        var bn = key.split("/").shift();
        if (bn !== currentBoardName)
            continue;
        var pn = key.split("/").pop();
        var tn = referencedPosts[key];
        var post = document.getElementById("post" + pn);
        if (!post)
            continue;
        var referencedByTr = post.querySelector("[name='referencedByTr']");
        referencedByTr.style.display = "";
        var referencedBy = post.querySelector("[name='referencedBy']");
        var a = document.createElement("a");
        a.href = "/" + prefix + bn + "/thread/" + tn + ".html#" + pn + "";
        a.addEventListener("mouseover", function(e) {
            lord.viewPost(this, bn, postNumber);
        });
        a.onmouseout = function() {
            lord.noViewPost();
        };
        referencedBy.appendChild(document.createTextNode(" "));
        a.appendChild(document.createTextNode(">>" + postNumber));
        referencedBy.appendChild(a);
    }
};

lord.setInitialScale = function(image, sizeHintX, sizeHintY) {
    if (!sizeHintX || !sizeHintY || sizeHintX <= 0 || sizeHintY <= 0)
        return;
    var doc = document.documentElement;
    var maxWidth = doc.clientWidth - 10;
    var maxHeight = doc.clientHeight - 10;
    var kw = 1;
    var kh = 1;
    if (sizeHintX > maxWidth)
        kw = maxWidth / sizeHintX;
    if (sizeHintY > maxHeight)
        kh = maxHeight / sizeHintY;
    image.scale = ((kw < kh) ? kw : kh) * 100;
};

lord.showPopup = function(text, timeout, additionalClassNames) {
    if (!text)
        return;
    if (isNaN(+timeout))
        timeout = 5000;
    var msg = document.createElement("div");
    msg.className = "popup";
    if (!!additionalClassNames)
        msg.className += " " + additionalClassNames;
    if (lord.popups.length > 0) {
        var prev = lord.popups[lord.popups.length - 1];
        msg.style.top = (prev.offsetTop + prev.offsetHeight + 5) + "px";
    }
    msg.appendChild(document.createTextNode(text));
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
    }, 5000);
};

lord.reloadCaptchaFunction = function() {
    if (!!grecaptcha)
        grecaptcha.reset();
};

lord.resetCaptcha = function() {
    var captcha = document.getElementById("googleCaptcha");
    if (!!captcha) {
        var boardName = document.getElementById("currentBoardName").value;
        lord.ajaxRequest("get_captcha_quota", [boardName], 8, function(res) {
            res = +res;
            if (isNaN(res))
                return;
            var hiddenCaptcha = document.getElementById("hiddenCaptcha");
            if (res > 0) {
                hiddenCaptcha.appendChild(captcha);
                ["Top", "Bottom"].forEach(function (pos) {
                    var td = document.getElementById("googleCaptcha" + pos);
                    for (var i = 0; i < td.children.length; ++i) {
                        var child = td.children[i];
                        if (child == captcha)
                            continue;
                        td.removeChild(child);
                    }
                    var span = document.createElement("span");
                    span.className = "noCaptchaText";
                    var text = document.getElementById("noCaptchaText").value + ". "
                        + document.getElementById("captchaQuotaText").value + " " + res;
                    span.appendChild(document.createTextNode(text));
                    td.appendChild(span);
                });
            } else {
                ["Top", "Bottom"].forEach(function (pos) {
                    var td = document.getElementById("googleCaptcha" + pos);
                    for (var i = 0; i < td.children.length; ++i) {
                        var child = td.children[i];
                        if (child == captcha)
                            continue;
                        td.removeChild(child);
                    }
                });
                var pos = !!lord.postFormVisible["Bottom"] ? "Bottom" : "Top";
                document.getElementById("googleCaptcha" + pos).appendChild(captcha);
            }
            if (!!lord.reloadCaptchaFunction && "hiddenCaptcha" !== captcha.parentNode.id)
                lord.reloadCaptchaFunction();
        });
    }
};

lord.traverseChildren = function(elem) {
    var children = [];
    var q = [];
    q.push(elem);
    function pushAll(elemArray) {
        for (var i = 0; i < elemArray.length; ++i)
            q.push(elemArray[i]);
    }
    while (q.length > 0) {
        var elem = q.pop();
        children.push(elem);
        pushAll(elem.children);
    }
    return children;
};

lord.ajaxRequest = function(method, params, id, callback) {
    var xhr = new XMLHttpRequest();
    xhr.withCredentials = true;
    var prefix = document.getElementById("sitePathPrefix").value;
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
                if (!!err)
                    return alert(err);
                callback(response.result);
            } else {
                alert(document.getElementById("ajaxErrorText").value + " " + xhr.status);
            }
        }
    };
    xhr.send(JSON.stringify(request));
};

lord.createPostFile = function(f, boardName) {
    if (!f)
        return null;
    var sitePrefix = document.getElementById("sitePathPrefix").value;
    if (!boardName)
        boardName = document.getElementById("currentBoardName").value;
    var file = document.createElement("td");
    file.className = "postFile";
    var divFileName = document.createElement("div");
    divFileName.className = "postFileName";
    var aFileName = document.createElement("a");
    aFileName.href = "/" + sitePrefix + boardName + "/" + f["sourceName"];
    aFileName.appendChild(document.createTextNode(f["sourceName"]));
    divFileName.appendChild(aFileName);
    file.appendChild(divFileName);
    var divFileSize = document.createElement("div");
    divFileSize.className = "postFileSize";
    divFileSize.appendChild(document.createTextNode("(" + f["size"] + ")"));
    file.appendChild(divFileSize);
    if (lord.isImageType(f["type"])) {
        var divFileSearch = document.createElement("div");
        divFileSearch.className = "postFileSearch";
        var siteDomain = document.getElementById("siteDomain").value;
        var siteProtocol = document.getElementById("siteProtocol").value;
        [{
            "link": "//www.google.com/searchbyimage?image_url=",
            "text": document.getElementById("findSourceWithGoogleText").value,
            "img": "google.png"
        }, {
            "link": "http://iqdb.org/?url=",
            "text": document.getElementById("findSourceWithIqdbText").value,
            "img": "iqdb.png"
        }].forEach(function(el) {
            var a = document.createElement("a");
            a.href = el.link + siteProtocol + "://" + siteDomain + "/" + sitePrefix + boardName + "/" + f["sourceName"];
            a.title = el.text;
            a.target = "_blank";
            var logo = document.createElement("img");
            logo.src = "/" + sitePrefix + "img/" + el.img;
            a.appendChild(logo);
            divFileSearch.appendChild(a);
            divFileSearch.appendChild(document.createTextNode(" "));
        });
        file.appendChild(divFileSearch);
    }
    var divImage = document.createElement("div");
    var aImage = document.createElement("a");
    aImage.href = "/" + sitePrefix + boardName + "/" + f["sourceName"];
    if (lord.isImageType(f["type"])) {
        aImage.onclick = function(e) {
            return lord.showImage("/" + sitePrefix + boardName + "/" + f["sourceName"], f["type"], f["sizeX"],
                             f["sizeY"]);
        };
    }
    var image = document.createElement("img");
    var thumbSizeX = +f["thumbSizeX"];
    var thumbSizeY = +f["thumbSizeY"];
    if (!isNaN(thumbSizeX) && thumbSizeX > 0)
        image.width = thumbSizeX;
    if (!isNaN(thumbSizeY) && thumbSizeY > 0)
        image.height = thumbSizeY;
    if (lord.isSpecialThumbName(f["thumbName"])) {
        var subtype = f["thumbName"].split("/").pop();
        image.src = "/" + sitePrefix + "img/" + subtype + "_logo.png";
    } else {
        image.src = "/" + sitePrefix + boardName + "/" + f["thumbName"];
    }
    aImage.appendChild(image);
    divImage.appendChild(aImage);
    file.appendChild(divImage);
    return file;
};

lord.createPostNode = function(res, permanent, boardName) {
    if (!res)
        return null;
    post = document.getElementById("postTemplate");
    if (!post)
        return null;
    post = post.cloneNode(true);
    post.id = !!permanent ? ("post" + res["number"]) : "";
    post.style.display = "";
    if (!boardName)
        boardName = document.getElementById("currentBoardName").value;
    var hidden = (lord.getCookie("postHidden" + boardName + res["number"]) === "true");
    if (hidden)
        post.className += " hiddenPost";
    var fixed = post.querySelector("[name='fixed']");
    if (!!res["fixed"])
        fixed.style.display = "";
    else
        fixed.parentNode.removeChild(fixed);
    var closed = post.querySelector("[name='closed']");
    if (!!res["closed"])
        closed.style.display = "";
    else
        closed.parentNode.removeChild(closed);
    post.querySelector("[name='subject']").appendChild(document.createTextNode(res["subject"]));
    var registered = post.querySelector("[name='registered']");
    if (!!res["showRegistered"] && !!res["showTripcode"])
        registered.style.display = "";
    else
        registered.parentNode.removeChild(registered);
    var name = post.querySelector("[name='name']");
    if (!!res["email"])
        name.innerHTML = "<a href='mailto:" + res["email"] + "'>" + res["nameRaw"] + "</a>";
    else
        name.innerHTML = res["name"];
    var tripcode = post.querySelector("[name='tripcode']");
    if (!!res["showTripcode"] && "" !== res["tripcode"]) {
        tripcode.style.display = "";
        tripcode.appendChild(document.createTextNode(res["tripcode"]));
    } else {
        tripcode.parentNode.removeChild(tripcode);
    }
    var whois = post.querySelector("[name='whois']");
    var sitePathPrefix = document.getElementById("sitePathPrefix").value;
    if (!!res["flagName"]) {
        whois.style.display = "";
        whois.src = whois.src.replace("%flagName%", res["flagName"]);
        whois.title = res["countryName"];
        if (!!res["cityName"])
            whois.title += ": " + res["cityName"];
    } else {
        whois.parentNode.removeChild(whois);
    }
    post.querySelector("[name='dateTime']").appendChild(document.createTextNode(res["dateTime"]));
    var moder = (document.getElementById("moder").value === "true");
    var number = post.querySelector("[name='number']");
    number.appendChild(document.createTextNode(res["number"]));
    if (moder)
        number.title = res["ip"];
    var postingEnabled = (document.getElementById("postingEnabled").value === "true");
    var inp = document.getElementById("currentThreadNumber");
    if (!!inp && +inp.value === res["threadNumber"]) {
        if (postingEnabled)
            number.href = "javascript:lord.insertPostNumber(" + res["number"] + ");";
        else
            number.href = "#" + res["number"];
    } else {
        number.href = "/" + sitePathPrefix + boardName + "/thread/" + res["threadNumber"] + ".html#"
        if (postingEnabled)
            number.href += "i";
        number.href += res["number"];
    }
    var files = post.querySelector("[name='files']");
    if (!!res["files"]) {
        post.ajaxFiles = {};
        for (var i = 0; i < res["files"].length; ++i) {
            var file = lord.createPostFile(res["files"][i], boardName);
            if (!!file)
                files.insertBefore(file, files.children[files.children.length - 1]);
            post.ajaxFiles[res["files"][i]["thumbName"]] = res["files"][i];
        }
    }
    var blockquoteThread = !!document.getElementById("currentThreadNumber");
    var modificationDateTimeTd = post.querySelector("[name='modificationDateTimeTd']");
    var bannedForTd = post.querySelector("[name='bannedForTd']");
    var referencedByTd = post.querySelector("[name='referencedByTd']");
    var oneFileTr = post.querySelector("[name='files']");
    var manyFilesTr = post.querySelector("[name='manyFilesTr']");
    var textOneFile = post.querySelector("[name='textOneFile']");
    var textManyFiles = post.querySelector("[name='textManyFiles']");
    if (res["files"].length < 2) {
        manyFilesTr.parentNode.removeChild(manyFilesTr);
        textOneFile.innerHTML = res["text"];
        if (blockquoteThread)
            textOneFile.className = "blockquoteThread";
    } else {
        var oneFileTd = post.querySelector("[name='oneFileTd']");
        oneFileTd.removeChild(textOneFile);
        oneFileTd.className = "shrink";
        post.querySelector("[name='manyFilesTd']").colSpan = res["files"].length + 2;
        post.querySelector("[name='textManyFiles']").innerHTML = res["text"];
        if (blockquoteThread)
            textManyFiles.className = "blockquoteThread";
        modificationDateTimeTd.colSpan = res["files"].length + 2;
        bannedForTd.colSpan = res["files"].length + 2;
        referencedByTd.colSpan = res["files"].length + 2;
    }
    var modificationDateTime = post.querySelector("[name='modificationDateTime']");
    if ("" !== res["modificationDateTime"]) {
        modificationDateTime.style.display = "";
        modificationDateTime.childNodes[0].nodeValue += " " + res["modificationDateTime"];
    } else {
        modificationDateTime.parentNode.removeChild(modificationDateTime);
    }
    var bannedFor = post.querySelector("[name='bannedFor']");
    if (!!res["bannedFor"])
        bannedFor.style.display = "";
    else
        bannedFor.parentNode.removeChild(bannedFor);
    var referencedBy = post.querySelector("[name='referencedBy']");
    if (!!res["referencedBy"] && res["referencedBy"].length > 0) {
        post.querySelector("[name='referencedByTr']").style.display = "";
        for (var i = 0; i < res["referencedBy"].length; ++i) {
            var ref = res["referencedBy"][i];
            var bn = ref["boardName"]
            var pn = ref["postNumber"];
            var tn = ref["threadNumber"];
            var a = document.createElement("a");
            a.href = "/" + sitePathPrefix + bn + "/thread/" + tn + ".html#" + pn;
            a.addEventListener("mouseover", lord.viewPost.bind(lord, a, bn, pn));
            a.onmouseout = function() {
                lord.noViewPost();
            };
            referencedBy.appendChild(document.createTextNode(" "));
            a.appendChild(document.createTextNode(">>" + (bn !== boardName ? ("/" + bn + "/") : "") + pn));
            referencedBy.appendChild(a);
        }
    }
    var perm = post.querySelector("[name='permanent']");
    if (!permanent) {
        perm.parentNode.removeChild(perm);
        return post;
    }
    post.className += " newPost";
    post.onmouseover = function() {
        this.className = this.className.replace(" newPost", "");
        this.onmouseover = null;
    }
    if (res["number"] === res["threadNumber"])
        post.className = post.className.replace("post", "opPost");
    if (hidden)
        post.className += " hiddenPost";
    perm.style.display = "";
    var anumber = document.createElement("a");
    number.parentNode.insertBefore(anumber, number);
    number.parentNode.removeChild(number);
    anumber.title = number.title;
    anumber.appendChild(number);
    if (postingEnabled)
        anumber.href = "javascript:lord.insertPostNumber(" + res["number"] + ");";
    else
        anumber.href = "#" + res["number"];
    var deleteButton = post.querySelector("[name='deleteButton']");
    deleteButton.href = deleteButton.href.replace("%postNumber%", res["number"]);
    var hideButton = post.querySelector("[name='hideButton']");
    hideButton.id = hideButton.id.replace("%postNumber%", res["number"]);
    hideButton.href = hideButton.href.replace("%postNumber%", res["number"]);
    
    hideButton.href = hideButton.href.replace("%hide%", !hidden);
    var editButton = post.querySelector("[name='editButton']");
    var fixButton = post.querySelector("[name='fixButton']");
    var unfixButton = post.querySelector("[name='unfixButton']");
    var openButton = post.querySelector("[name='openButton']");
    var closeButton = post.querySelector("[name='closeButton']");
    var banButton = post.querySelector("[name='banButton']");
    var rawText = post.querySelector("[name='rawText']");
    post.querySelector("[name='draft']").value = !!res["draft"];
    if (moder || !!res["draft"]) {
        editButton.href = editButton.href.replace("%postNumber%", res["number"]);
        rawText.value = res["rawPostText"];
        post.querySelector("[name='email']").value = res["email"];
        post.querySelector("[name='name']").value = res["rawName"];
        post.querySelector("[name='subject']").value = res["rawSubject"];
    } else {
        editButton.parentNode.removeChild(editButton);
    }
    if (!moder) {
        fixButton.parentNode.removeChild(fixButton);
        unfixButton.parentNode.removeChild(unfixButton);
        openButton.parentNode.removeChild(openButton);
        closeButton.parentNode.removeChild(closeButton);
        banButton.parentNode.removeChild(banButton);
        return post;
    }
    var toThread = post.querySelector("[name='toThread']");
    if (res["number"] === res["threadNumber"]) {
        if (!!res["fixed"]) {
            unfixButton.style.display = "";
            unfixButton.href = unfixButton.href.replace("%postNumber%", res["number"]);
            fixButton.parentNode.removeChild(fixButton);
        } else {
            fixButton.style.display = "";
            fixButton.href = fixButton.href.replace("%postNumber%", res["number"]);
            unfixButton.parentNode.removeChild(unfixButton);
        }
        if (!!res["closed"]) {
            openButton.style.display = "";
            openButton.href = openButton.href.replace("%postNumber%", res["number"]);
            closeButton.parentNode.removeChild(closeButton);
        } else {
            closeButton.style.display = "";
            closeButton.href = closeButton.href.replace("%postNumber%", res["number"]);
            openButton.parentNode.removeChild(openButton);
        }
        toThread.style.display = "";
        var toThreadLink = post.querySelector("[name='toThreadLink']");
        toThreadLink.href = toThreadLink.href.replace("%postNumber%", res["number"]);
    } else {
        fixButton.parentNode.removeChild(fixButton);
        unfixButton.parentNode.removeChild(unfixButton);
        openButton.parentNode.removeChild(openButton);
        closeButton.parentNode.removeChild(closeButton);
        toThread.parentNode.removeChild(toThread);
    }
    banButton.href = banButton.href.replace("%postNumber%", res["number"]);
    return post;
};

lord.clearFileInput = function(div) {
    var preview = div.querySelector("img");
    if (!!preview && div == preview.parentNode)
        preview.src = "/" + document.getElementById("sitePathPrefix").value + "img/addfile.png";
    var span = div.querySelector("span");
    if (!!span && div == span.parentNode && !!span.childNodes && !!span.childNodes[0])
        span.removeChild(span.childNodes[0]);
    div.fileHash = null;
};

lord.readableSize = function(sz) {
    sz = +sz;
    if (isNaN(sz))
        return "";
    if (sz / 1024 >= 1) {
        sz /= 1024;
        if (sz / 1024 >= 1) {
            sz = (sz / 1024).toFixed(1);
            sz += " " + document.getElementById("megabytesText").value;
        } else {
            sz = sz.toFixed(1);
            sz += " " + document.getElementById("kilobytesText").value;
        }
    } else {
        sz = sz.toString();
        sz += " " + document.getElementById("bytesText").value;
    }
    return sz;
};

lord.getFileHashes = function(div) {
    return div.parentNode.parentNode.parentNode.parentNode.parentNode.querySelector("[name='fileHashes']");
};

lord.hideImage = function() {
    if (!!lord.img) {
        if (lord.isAudioType(lord.img.fileType) || lord.isVideoType(lord.img.fileType)) {
            lord.img.pause();
            lord.img.load();
        }
        lord.img.style.display = "none";
        lord.img = null;
    }
};

lord.globalOnclick = function(e) {
    if (!!e.button)
        return;
    var t = e.target;
    if (!!t && !!lord.img && t == lord.img)
        return;
    while (!!t) {
        if (t.tagName === "A" && (!!t.onclick || !!t.onmousedown || !!t.href))
            return;
        t = t.parentNode;
    }
    lord.hideImage();
};

lord.showPasswordDialog = function(title, label, callback) {
    var div = document.createElement("div");
    var input = document.createElement("input");
    input.type = "password";
    input.className = "input";
    input.maxlength = 150;
    input.size = 30;
    div.appendChild(input);
    var sw = document.createElement("input");
    sw.type = "checkbox";
    sw.className = "checkbox";
    sw.title = document.getElementById("showPasswordText").value;
    sw.onclick = function() {
        if (input.type === "password")
            input.type = "text";
        else if (input.type === "text")
            input.type = "password";
    };
    div.appendChild(sw);
    lord.showDialog(title, label, div, function() {
        callback(input.value);
    }, function() {
        input.focus();
    });
};

lord.showHidePostForm = function(position) {
    var theButton = document.getElementById("showHidePostFormButton" + position);
    if (lord.postFormVisible[position]) {
        theButton.innerHTML = document.getElementById("showPostFormText").value;
        document.getElementById("postForm" + position).className = "postFormInvisible";
    } else {
        theButton.innerHTML = document.getElementById("hidePostFormText").value;
        document.getElementById("postForm" + position).className = "postFormVisible";
        var p = ("Top" === position) ? "Bottom" : "Top";
        if (lord.postFormVisible[p])
            lord.showHidePostForm(p);
        var captcha = document.getElementById("googleCaptcha");
        if (!!captcha && "hiddenCaptcha" !== captcha.parentNode.id)
            document.getElementById("googleCaptcha" + position).appendChild(captcha);
    }
    lord.postFormVisible[position] = !lord.postFormVisible[position];
};

lord.deletePost = function(boardName, postNumber, fromThread) {
    if (!boardName || isNaN(+postNumber))
        return;
    var title = document.getElementById("enterPasswordTitle").value;
    var label = document.getElementById("enterPasswordText").value;
    lord.showPasswordDialog(title, label, function(pwd) {
        if (null === pwd)
            return;
        if (pwd.length < 1) {
            if (!lord.getCookie("hashpass"))
                return alert(document.getElementById("notLoggedInText").value);
        } else if (!lord.isHashpass(pwd)) {
            pwd = lord.toHashpass(pwd);
        }
        lord.ajaxRequest("delete_post", [boardName, +postNumber, pwd], 1, function(res) {
            var post = document.getElementById("post" + postNumber);
            if (!post) {
                if (!!fromThread) {
                    var suffix = "thread/" + postNumber + ".html";
                    window.location.href = window.location.href.replace(suffix, "").split("#").shift();
                } else {
                    lord.reloadPage();
                }
                return;
            } else if (post.className.indexOf("opPost") > -1) {
                var suffix = "thread/" + postNumber + ".html";
                window.location.href = window.location.href.replace(suffix, "").split("#").shift();
            } else {
                post.parentNode.removeChild(post);
                lord.removeReferences(postNumber);
                var postLinks = document.body.querySelectorAll("a");
                if (!!postLinks) {
                    for (var i = 0; i < postLinks.length; ++i) {
                        var link = postLinks[i];
                        if (("&gt;&gt;" + postNumber) !== link.innerHTML)
                            continue;
                        var text = link.innerHTML.replace("&gt;&gt;", ">>");
                        link.parentNode.replaceChild(document.createTextNode(text), link);
                    }
                }
                if (!!lord.postPreviews[boardName + "/" + postNumber])
                    delete lord.postPreviews[boardName + "/" + postNumber];
            }
        });
    });
};

lord.setThreadFixed = function(boardName, postNumber, fixed) {
    if (!boardName || isNaN(+postNumber))
        return;
    if (!lord.getCookie("hashpass"))
        return alert(document.getElementById("notLoggedInText").value);
    lord.ajaxRequest("set_thread_fixed", [boardName, +postNumber, !!fixed], 2, lord.reloadPage);
};

lord.setThreadOpened = function(boardName, postNumber, opened) {
    if (!boardName || isNaN(+postNumber))
        return;
    if (!lord.getCookie("hashpass"))
        return alert(document.getElementById("notLoggedInText").value);
    lord.ajaxRequest("set_thread_opened", [boardName, +postNumber, !!opened], 3, lord.reloadPage);
};

lord.banUser = function(boardName, postNumber) {
    if (!boardName || isNaN(+postNumber))
        return;
    var title = document.getElementById("banUserText").value;
    var div = document.createElement("div");
    var div1 = document.createElement("div");
    div1.appendChild(document.createTextNode(document.getElementById("boardLabelText").value));
    var selBoard = document.getElementById("availableBoardsSelect").cloneNode(true);
    selBoard.style.display = "block";
    div1.appendChild(selBoard);
    div.appendChild(div1);
    var div2 = document.createElement("div");
    div2.appendChild(document.createTextNode(document.getElementById("banLevelLabelText").value));
    var selLevel = document.getElementById("banLevelsSelect").cloneNode(true);
    selLevel.style.display = "block";
    div2.appendChild(selLevel);
    div.appendChild(div2);
    var div3 = document.createElement("div");
    div3.appendChild(document.createTextNode(document.getElementById("banReasonLabelText").value));
    var inputReason = document.createElement("input");
    inputReason.type = "text";
    inputReason.className = "input";
    div3.appendChild(inputReason);
    div.appendChild(div3);
    var div4 = document.createElement("div");
    div4.appendChild(document.createTextNode(document.getElementById("banExpiresLabelText").value));
    var inputExpires = document.createElement("input");
    inputExpires.type = "text";
    inputExpires.placeholder = "dd.MM.yyyy:hh";
    inputExpires.className = "input";
    div4.appendChild(inputExpires);
    div.appendChild(div4);
    lord.showDialog(title, null, div, function() {
        var params = {
            "boardName": boardName,
            "postNumber": +postNumber,
            "board": selBoard.options[selBoard.selectedIndex].value,
            "level": +selLevel.options[selLevel.selectedIndex].value,
            "reason": inputReason.value,
            "expires": inputExpires.value
        };
        lord.ajaxRequest("ban_user", [params], 4, function(res) {
            lord.reloadPage();
        });
    });
};

lord.editPost = function(boardName, postNumber) {
    if (!boardName || isNaN(+postNumber))
        return;
    var post = document.getElementById("post" + postNumber);
    if (!post)
        return;
    var title = document.getElementById("editPostText").value;
    var form = document.getElementById("editPostTemplate").cloneNode(true);
    form.id = "";
    form.style.display = "";
    var email = form.querySelector("[name='email']");
    var name = form.querySelector("[name='name']");
    var subject = form.querySelector("[name='subject']");
    var text = form.querySelector("[name='text']");
    email.value = post.querySelector("[name='email']").value;
    name.value = post.querySelector("[name='name']").value;
    subject.value = post.querySelector("[name='subject']").value;
    text.appendChild(document.createTextNode(post.querySelector("[name='rawText']").value));
    var moder = (document.getElementById("moder").value === "true");
    var draftField = form.querySelector("[name='draft']");
    var rawField = form.querySelector("[name='raw']");
    if (!!draftField) {
        if (post.querySelector("[name='draft']").value == "true")
            draftField.checked = true;
        else
            draftField.parentNode.parentNode.style.display = "none";
    }
    if (!!rawField && post.querySelector("[name='rawHtml']").value == "true")
        rawField.checked = true;
    lord.showDialog(title, null, form, function() {
        var pwd = form.querySelector("[name='password']").value;
        if (pwd.length < 1) {
            if (!lord.getCookie("hashpass"))
                return alert(document.getElementById("notLoggedInText").value);
        } else if (!lord.isHashpass(pwd)) {
            pwd = lord.toHashpass(pwd);
        }
        var params = {
            "boardName": boardName,
            "postNumber": +postNumber,
            "text": text.value,
            "email": email.value,
            "name": name.value,
            "subject": subject.value,
            "raw": !!rawField ? form.querySelector("[name='raw']").checked : false,
            "draft": !!draftField ? draftField.checked : false,
            "password": pwd
        };
        lord.ajaxRequest("edit_post", [params], 5, function(rese) {
            lord.ajaxRequest("get_post", [boardName, +postNumber], 6, function(res) {
                var newPost = lord.createPostNode(res, true);
                if (!newPost)
                    return;
                lord.removeReferences(postNumber);
                lord.addReferences(postNumber, rese);
                var postLimit = post.querySelector("[name='postLimit']");
                var bumpLimit = post.querySelector("[name='bumpLimit']");
                if (!!postLimit || !!bumpLimit) {
                    var postHeader = newPost.querySelector(".postHeader");
                    if (!!postLimit)
                        postHeader.appendChild(postLimit.cloneNode(true));
                    if (!!bumpLimit)
                        postHeader.appendChild(bumpLimit.cloneNode(true));
                }
                post.parentNode.replaceChild(newPost, post);
            });
        });
    });
};

lord.setPostHidden = function(boardName, postNumber, hidden) {
    if (!boardName || isNaN(+postNumber))
        return;
    var post = document.getElementById("post" + postNumber);
    var sw = document.getElementById("hidePost" + postNumber);
    if (!!hidden) {
        post.className += " hiddenPost";
        sw.href = sw.href.replace("true", "false");
        lord.setCookie("postHidden" + boardName + postNumber, "true", {
            "expires": lord.Billion, "path": "/"
        });
    } else {
        post.className = post.className.replace(" hiddenPost", "");
        sw.href = sw.href.replace("false", "true");
        lord.setCookie("postHidden" + boardName + postNumber, "true", {
            "expires": -1, "path": "/"
        });
    }
};

lord.setThreadHidden = function(boardName, postNumber, hidden) {
    if (!boardName || isNaN(+postNumber))
        return;
    var post = document.getElementById("post" + postNumber);
    var sw = document.getElementById("hidePost" + postNumber);
    var thread = document.getElementById("thread" + postNumber);
    var omitted = document.getElementById("threadOmitted" + postNumber);
    var posts = document.getElementById("threadPosts" + postNumber);
    if (!!hidden) {
        post.className += " hiddenPost";
        if (!!thread)
            thread.className = "hiddenThread";
        if (!!omitted)
            omitted.className += " hiddenPosts";
        if (!!posts)
            posts.className += " hiddenPosts";
        sw.href = sw.href.replace("true", "false");
        lord.setCookie("postHidden" + boardName + postNumber, "true", {
            "expires": lord.Billion, "path": "/"
        });
    } else {
        post.className = post.className.replace(" hiddenPost", "");
        if (!!thread)
            thread.className = "";
        if (!!omitted)
            omitted.className = omitted.className.replace(" hiddenPosts", "");
        if (!!posts)
            posts.className = posts.className.replace(" hiddenPosts", "");
        sw.href = sw.href.replace("false", "true");
        lord.setCookie("postHidden" + boardName + postNumber, "true", {
            "expires": -1, "path": "/"
        });
    }
};

lord.viewPostStage2 = function(link, boardName, postNumber, post) {
    post.onmouseout = function(event) {
        var next = post;
        while (!!next) {
            var list = lord.traverseChildren(next);
            var e = event.toElement || event.relatedTarget;
            if (list.indexOf(e) >= 0)
                return;
            next = next.nextPostPreview;
        }
        if (!!post.parentNode)
            post.parentNode.removeChild(post);
        if (post.previousPostPreview)
            post.previousPostPreview.onmouseout(event);
    };
    post.onmouseover = function(event) {
        post.mustHide = false;
    };
    if (!lord.postPreviews[boardName + "/" + postNumber])
        lord.postPreviews[boardName + "/" + postNumber] = post;
    else
        post.style.display = "";
    post.previousPostPreview = lord.lastPostPreview;
    if (!!lord.lastPostPreview)
        lord.lastPostPreview.nextPostPreview = post;
    lord.lastPostPreview = post;
    post.mustHide = true;
    if (!!lord.lastPostPreviewTimer) {
        clearTimeout(lord.lastPostPreviewTimer);
        lord.lastPostPreviewTimer = null;
    }
    document.body.appendChild(post);
    post.style.position = "absolute";
    var doc = document.documentElement;
    var coords = link.getBoundingClientRect();
    var linkCenter = coords.left + (coords.right - coords.left) / 2;
    if (linkCenter < 0.6 * doc.clientWidth) {
        post.style.maxWidth = doc.clientWidth - linkCenter + "px";
        post.style.left = linkCenter + "px";
    } else {
        post.style.maxWidth = linkCenter + "px";
        post.style.left = linkCenter - post.scrollWidth + "px";
    }
    var scrollTop = doc.scrollTop;
    if (!scrollTop) //NOTE: Workaround for Chrome/Safari. I really HATE you, HTML/CSS/JS!
        scrollTop = document.body.scrollTop;
    post.style.top = (doc.clientHeight - coords.bottom >= post.scrollHeight)
        ? (scrollTop + coords.bottom - 4 + "px")
        : (scrollTop + coords.top - post.scrollHeight - 4 + "px");
    post.style.zIndex = 9001;
};

lord.viewPost = function(link, boardName, postNumber) {
    if (!link || !boardName || isNaN(+postNumber))
        return;
    var currentBoardName = document.getElementById("currentBoardName").value;
    var post = null;
    if (boardName === currentBoardName)
        post = document.getElementById("post" + postNumber);
    if (!post)
        post = lord.postPreviews[boardName + "/" + postNumber];
    if (!post) {
        lord.ajaxRequest("get_post", [boardName, +postNumber], 6, function(res) {
            post = lord.createPostNode(res, false, boardName);
            if (!post)
                return;
            post["fromAjax"] = true;
            lord.viewPostStage2(link, boardName, postNumber, post);
        });
    } else {
        var fromAjax = !!post.fromAjax;
        if (fromAjax)
            var ajaxFiles = post.ajaxFiles;
        post = post.cloneNode(true);
        if (fromAjax) {
            post.addEventListener("mouseover", function(e) {
                var a = e.target;
                if (a.tagName != "A")
                    return;
                var pn = a.innerHTML.replace("&gt;&gt;", "");
                var ind = pn.lastIndexOf("/");
                var bn = boardName;
                if (ind > 0) {
                    bn = pn.substring(1, ind);
                    pn = pn.substring(ind + 1);
                }
                pn = +pn;
                if (isNaN(pn))
                    return;
                lord.viewPost(a, bn, pn);
            });
            post.addEventListener("mouseout", function(e) {
                var a = e.target;
                if (a.tagName != "A")
                    return;
                var pn = +a.innerHTML.replace("&gt;&gt;", "");
                if (isNaN(pn))
                    return;
                lord.noViewPost();
            });
            post.addEventListener("click", function(e) {
                var img = e.target;
                if (img.tagName != "IMG")
                    return;
                if (!ajaxFiles)
                    return;
                var ajaxFile = ajaxFiles[img.src.split("/").pop()];
                if (!ajaxFile)
                    return;
                var ind = img.src.lastIndexOf("/");
                var href = img.src.substring(0, ind) + "/" + ajaxFile["sourceName"];
                if (lord.showImage(href, ajaxFile["type"], ajaxFile["sizeX"], ajaxFile["sizeY"]) === false)
                    e.preventDefault();
            });
        }
        post.className = post.className.replace("opPost", "post");
        var list = lord.traverseChildren(post);
        for (var i = 0; i < list.length; ++i) {
            switch (list[i].name) {
            case "editButton":
            case "fixButton":
            case "closeButton":
            case "banButton":
            case "deleteButton":
            case "hideButton":
                list[i].parentNode.removeChild(list[i]);
                break;
            default:
                break;
            }
            list[i].id = "";
        }
        lord.viewPostStage2(link, boardName, postNumber, post);
    }
};

lord.noViewPost = function() {
    lord.lastPostPreviewTimer = setTimeout(function() {
        if (!lord.lastPostPreview)
            return;
        if (!!lord.lastPostPreview.mustHide && !!lord.lastPostPreview.parentNode)
            lord.lastPostPreview.parentNode.removeChild(lord.lastPostPreview);
    }, 500);
};

lord.fileSelected = function(current) {
    if (!current)
        return;
    var inp = document.getElementById("maxFileCount");
    if (!inp)
        return;
    var maxCount = +inp.value;
    if (isNaN(maxCount))
        return;
    var div = current.parentNode;
    if (current.value == "")
        return lord.removeFile(div.querySelector("a"));
    lord.clearFileInput(div);
    var file = current.files[0];
    div.querySelector("span").appendChild(document.createTextNode(file.name
        + " (" + lord.readableSize(file.size) + ")"));
    var binaryReader = new FileReader();
    binaryReader.readAsArrayBuffer(file);
    var oldDiv = div;
    var previous = current;
    binaryReader.onload = function(e) {
        var wordArray = CryptoJS.lib.WordArray.create(e.target.result);
        var currentBoardName = document.getElementById("currentBoardName").value;
        var fileHash = lord.toHashpass(wordArray);
        lord.ajaxRequest("get_file_existence", [currentBoardName, fileHash], 8, function(res) {
            if (!res)
                return;
            var fileHashes = lord.getFileHashes(oldDiv);
            if (fileHashes.value.indexOf(fileHash) < 0)
                fileHashes.value = fileHashes.value + (fileHashes.value.length > 0 ? "," : "") + fileHash;
            var f = previous.onchange;
            delete previous.onchange;
            previous.value = "";
            previous.onchange = f;
            oldDiv.fileHash = fileHash;
        });
    };
    if (!!current.value.match(/\.(jpe?g|png|gif)$/i)) {
        var reader = new FileReader();
        reader.readAsDataURL(file);
        reader.onload = function(e) {
            oldDiv.querySelector("img").src = e.target.result;
        };
    } else if (!!current.value.match(/\.(mp3|mp4)$/i)) {
        div.querySelector("img").src = "/" + document.getElementById("sitePathPrefix").value + "img/mpeg_file.png";
    } else if (!!current.value.match(/\.(ogg)$/i)) {
        div.querySelector("img").src = "/" + document.getElementById("sitePathPrefix").value + "img/ogg_file.png";
    } else if (!!current.value.match(/\.(webm)$/i)) {
        div.querySelector("img").src = "/" + document.getElementById("sitePathPrefix").value + "img/webm_file.png";
    } else if (!!current.value.match(/\.(wav)$/i)) {
        div.querySelector("img").src = "/" + document.getElementById("sitePathPrefix").value + "img/wav_file.png";
    } else {
        div.querySelector("img").src = "/" + document.getElementById("sitePathPrefix").value + "img/file.png";
    }
    div.querySelector("a").style.display = "inline";
    var parent = div.parentNode;
    if (parent.children.length >= maxCount)
        return;
    for (var i = 0; i < parent.children.length; ++i) {
        if (!parent.children[i].fileHash && parent.children[i].querySelector("input").value === "")
            return;
        parent.children[i].querySelector("a").style.display = "inline";
    }
    div = div.cloneNode(true);
    lord.clearFileInput(div);
    div.querySelector("a").style.display = "none";
    div.innerHTML = div.innerHTML; //NOTE: Workaround since we can't clear it other way
    parent.appendChild(div);
};

lord.removeFile = function(current) {
    if (!current)
        return;
    var div = current.parentNode;
    if (!!div.fileHash) {
        var fileHashes = lord.getFileHashes(div);
        var val = fileHashes.value.replace("," + div.fileHash, "");
        if (val === fileHashes.value)
            val = fileHashes.value.replace(div.fileHash + ",", "");
        if (val === fileHashes.value)
            val = fileHashes.value.replace(div.fileHash, "");
        fileHashes.value = val;
    }
    var parent = div.parentNode;
    parent.removeChild(div);
    lord.clearFileInput(div);
    if (parent.children.length > 1) {
        for (var i = 0; i < parent.children.length; ++i) {
            if (!parent.children[i].fileHash && parent.children[i].querySelector("input").value === "")
                return;
        }
        var inp = document.getElementById("maxFileCount");
        if (!inp)
            return;
        var maxCount = +inp.value;
        if (isNaN(maxCount))
            return;
        if (parent.children.length >= maxCount)
            return;
        div = div.cloneNode(true);
        div.querySelector("a").style.display = "none";
        div.innerHTML = div.innerHTML; //NOTE: Workaround since we can't clear it other way
        parent.appendChild(div);
    }
    if (parent.children.length > 0 && !parent.children[0].fileHash
            && parent.children[0].querySelector("input").value === "") {
        parent.children[0].querySelector("a").style.display = "none";
    }
    if (parent.children.length < 1) {
        div.querySelector("a").style.display = "none";
        div.innerHTML = div.innerHTML; //NOTE: Workaround since we can't clear it other way
        parent.appendChild(div);
    }
};

lord.browseFile = function(e, div) {
    var inp = div.querySelector("input");
    if (!inp)
        return;
    var e = window.event || e;
    var a = e.target;
    while (!!a) {
        if (a.tagName === "A")
            return;
        a = a.parentNode;
    }
    inp.click();
};

lord.showImage = function(href, type, sizeHintX, sizeHintY) {
    lord.hideImage();
    if (!href || !type)
        return true;
    lord.img = lord.images[href];
    if (!!lord.img) {
        lord.setInitialScale(lord.img, sizeHintX, sizeHintY);
        lord.resetScale(lord.img);
        lord.img.style.display = "";
        lord.toCenter(lord.img, sizeHintX, sizeHintY);
        if (lord.isAudioType(lord.img.fileType) || lord.isVideoType(lord.img.fileType)) {
            setTimeout(function() {
                lord.img.play();
            }, 500);
        }
        return false;
    }
    if (lord.isAudioType(type)) {
        sizeHintX = 400;
        lord.img = document.createElement("audio");
        lord.img.width = sizeHintX + "px";
        lord.img.controls = "controls";
        var src = document.createElement("source");
        src.src = href;
        src.type = type;
        lord.img.appendChild(src);
    } else if (lord.isImageType(type)) {
        if (!sizeHintX || !sizeHintY || sizeHintX <= 0 || sizeHintY <= 0)
            return true;
        lord.img = document.createElement("img");
        lord.img.width = sizeHintX;
        lord.img.height = sizeHintY;
        lord.img.src = href;
    } else if (lord.isVideoType(type)) {
        lord.img = document.createElement("video");
        lord.img.controls = "controls";
        var src = document.createElement("source");
        src.src = href;
        src.type = type;
        lord.img.appendChild(src);
    }
    lord.img.fileType = type;
    lord.setInitialScale(lord.img, sizeHintX, sizeHintY);
    lord.resetScale(lord.img);
    lord.img.moving = false;
    lord.img.coord = {
        "x": 0,
        "y": 0
    };
    lord.img.initialCoord = {
        "x": 0,
        "y": 0
    };
    lord.img.className = "movableImage";
    var wheelHandler = function(e) {
        var e = window.event || e; //Old IE support
        e.preventDefault();
        var delta = Math.max(-1, Math.min(1, (e.wheelDelta || -e.detail)));
        lord.img.scale += delta;
        lord.resetScale(lord.img);
    };
    if (lord.img.addEventListener) {
    	lord.img.addEventListener("mousewheel", wheelHandler, false); //IE9, Chrome, Safari, Opera
	    lord.img.addEventListener("DOMMouseScroll", wheelHandler, false); //Firefox
    } else {
        lord.img.attachEvent("onmousewheel", wheelHandler); //IE 6/7/8
    }
    if (lord.isImageType(type)) {
        lord.img.onmousedown = function(e) {
            if (!!e.button)
                return;
            e.preventDefault();
            lord.img.moving = true;
            lord.img.coord.x = e.clientX;
            lord.img.coord.y = e.clientY;
            lord.img.initialCoord.x = e.clientX;
            lord.img.initialCoord.y = e.clientY;
        };
        lord.img.onmouseup = function(e) {
            if (!!e.button)
                return;
            e.preventDefault();
            lord.img.moving = false;
            if (lord.img.initialCoord.x === e.clientX && lord.img.initialCoord.y === e.clientY) {
                if (lord.isAudioType(type) || lord.isVideoType(type)) {
                    lord.img.pause();
                    lord.img.currentTime = 0;
                }
                lord.img.style.display = "none";
            }
        };
        lord.img.onmousemove = function(e) {
            if (!lord.img.moving)
                return;
            e.preventDefault();
            var dx = e.clientX - lord.img.coord.x;
            var dy = e.clientY - lord.img.coord.y;
            lord.img.style.left = (lord.img.offsetLeft + dx) + "px";
            lord.img.style.top = (lord.img.offsetTop + dy) + "px";
            lord.img.coord.x = e.clientX;
            lord.img.coord.y = e.clientY;
        };
    }
    document.body.appendChild(lord.img);
    lord.toCenter(lord.img, sizeHintX, sizeHintY);
    if (lord.isAudioType(lord.img.fileType) || lord.isVideoType(lord.img.fileType)) {
        setTimeout(function() {
            lord.img.play();
        }, 500);
    }
    lord.images[href] = lord.img;
    return false;
};

lord.complain = function() {
    alert(document.getElementById("complainMessage").value);
};

lord.submitted = function(form) {
    lord.formSubmitted = form;
    form.querySelector("[name='submit']").disabled = true;
};

lord.postedOnBoard = function() {
    if (!lord.formSubmitted)
        return;
    var iframe = document.getElementById("kostyleeque");
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var threadNumber = iframeDocument.querySelector("#threadNumber");
    lord.formSubmitted.querySelector("[name='submit']").disabled = false;
    if (!!threadNumber) {
        var href = window.location.href.split("#").shift();
        window.location.href = href + (href.substring(href.length - 1) != "/" ? "/" : "") + "thread/"
            + threadNumber.value + ".html";
    } else {
        lord.formSubmitted = null;
        var errmsg = iframeDocument.querySelector("#errorMessage");
        var errdesc = iframeDocument.querySelector("#errorDescription");
        lord.showPopup(errmsg.innerHTML + ": " + errdesc.innerHTML);
        resetCaptcha();
    }
};

lord.initializeOnLoadBaseBoard = function() {
    lord.initializeOnLoadSettings();
    document.body.onclick = lord.globalOnclick;
};
