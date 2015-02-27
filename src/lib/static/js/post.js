var postPreviews = {};
var lastPostPreview = null;
var lastPostPreviewTimer = null;

function cumulativeOffset(element) {
    var top = 0;
    var left = 0;
    do {
        top += element.offsetTop || 0;
        left += element.offsetLeft || 0;
        element = element.offsetParent;
    } while(element);
    return {
        "top": top,
        "left": left
    };
};

function traverseChildren(elem) {
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
}

function ajaxRequest(method, params, id, callback) {
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
}

function showPasswordDialog(title, label, callback) {
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
    showDialog(title, label, div, function() {
        callback(input.value);
    }, function() {
        input.focus();
    });
}

function editPost(boardName, postNumber) {
    if (!boardName || isNaN(+postNumber))
        return;
    var postText = document.getElementById("post" + postNumber + "RawText");
    if (!postText)
        return;
    var title = document.getElementById("editPostText").value;
    var div = document.createElement("div");
    var textarea = document.createElement("textarea");
    textarea.appendChild(document.createTextNode(postText.value));
    div.appendChild(textarea);
    showDialog(title, null, div, function() {
        ajaxRequest("edit_post", [boardName, +postNumber, textarea.value], 5, function(res) {
            reloadPage();
        });
    });
}

function setThreadFixed(boardName, postNumber, fixed) {
    if (!boardName || isNaN(+postNumber))
        return;
    if (!getCookie("hashpass"))
        return alert(document.getElementById("notLoggedInText").value);
    ajaxRequest("set_thread_fixed", [boardName, +postNumber, !!fixed], 2, reloadPage);
}

function setThreadOpened(boardName, postNumber, opened) {
    if (!boardName || isNaN(+postNumber))
        return;
    if (!getCookie("hashpass"))
        return alert(document.getElementById("notLoggedInText").value);
    ajaxRequest("set_thread_opened", [boardName, +postNumber, !!opened], 3, reloadPage);
}

function banUser(boardName, postNumber) {
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
    showDialog(title, null, div, function() {
        var params = {
            "boardName": boardName,
            "postNumber": +postNumber,
            "board": selBoard.options[selBoard.selectedIndex].value,
            "level": +selLevel.options[selLevel.selectedIndex].value,
            "reason": inputReason.value,
            "expires": inputExpires.value
        };
        ajaxRequest("ban_user", [params], 4, function(res) {
            reloadPage();
        });
    });
}

function deletePost(boardName, postNumber, fromThread) {
    if (!boardName || isNaN(+postNumber))
        return;
    var title = document.getElementById("enterPasswordTitle").value;
    var label = document.getElementById("enterPasswordText").value;
    showPasswordDialog(title, label, function(pwd) {
        if (null === pwd)
            return;
        if (pwd.length < 1) {
            if (!getCookie("hashpass"))
                return alert(document.getElementById("notLoggedInText").value);
        } else if (!isHashpass(pwd)) {
            pwd = toHashpass(pwd);
        }
        ajaxRequest("delete_post", [boardName, +postNumber, pwd], 1, function(res) {
            var post = document.getElementById("post" + postNumber);
            if (!post) {
                if (!!fromThread) {
                    var suffix = "thread/" + postNumber + ".html";
                    window.location.href = window.location.href.replace(suffix, "");
                } else {
                    reloadPage();
                }
                return;
            } else if (post.className.indexOf("opPost") > -1) {
                var suffix = "thread/" + postNumber + ".html";
                window.location.href = window.location.href.replace(suffix, "");
            } else {
                post.parentNode.removeChild(post);
                window.location.href = window.location.href.replace(/#\d+/g, "");
            }
        });
    });
}

function setPostHidden(boardName, postNumber, hidden) {
    if (!boardName || isNaN(+postNumber))
        return;
    var post = document.getElementById("post" + postNumber);
    var sw = document.getElementById("hidePost" + postNumber);
    if (!!hidden) {
        post.className += " hiddenPost";
        sw.href = sw.href.replace("true", "false");
        setCookie("postHidden" + boardName + postNumber, "true", {
            "expires": Billion, "path": "/"
        });
    } else {
        post.className = post.className.replace(" hiddenPost", "");
        sw.href = sw.href.replace("false", "true");
        setCookie("postHidden" + boardName + postNumber, "true", {
            "expires": -1, "path": "/"
        });
    }
}

function setThreadHidden(boardName, postNumber, hidden) {
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
        setCookie("postHidden" + boardName + postNumber, "true", {
            "expires": Billion, "path": "/"
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
        setCookie("postHidden" + boardName + postNumber, "true", {
            "expires": -1, "path": "/"
        });
    }
}

function createPostFile(f) {
    if (!f)
        return null;
    var sitePrefix = document.getElementById("sitePathPrefix").value;
    var currentBoardName = document.getElementById("currentBoardName").value;
    var file = document.createElement("td");
    file.className = "postFile";
    var divFileName = document.createElement("div");
    divFileName.className = "postFileName";
    var aFileName = document.createElement("a");
    aFileName.href = "/" + sitePrefix + currentBoardName + "/" + f["sourceName"];
    aFileName.appendChild(document.createTextNode(f["sourceName"]));
    divFileName.appendChild(aFileName);
    file.appendChild(divFileName);
    var divFileSize = document.createElement("div");
    divFileSize.className = "postFileSize";
    divFileSize.appendChild(document.createTextNode(f["size"]));
    file.appendChild(divFileSize);
    var divImage = document.createElement("div");
    var aImage = document.createElement("a");
    aImage.href = "/" + sitePrefix + currentBoardName + "/" + f["sourceName"];
    var image = document.createElement("img");
    var sizeX = +f["sizeX"];
    var sizeY = +f["sizeY"];
    if (!isNaN(sizeX) && sizeX > 0)
        image.width = sizeX;
    if (!isNaN(sizeY) && sizeY > 0)
        image.height = sizeY;
    if ("webm" === f["thumbName"]) {
        image.src = "/" + sitePrefix + "img/webm_logo.png";
    } else {
        image.src = "/" + sitePrefix + currentBoardName + "/" + f["thumbName"];
    }
    aImage.appendChild(image);
    divImage.appendChild(aImage);
    file.appendChild(divImage);
    return file;
}

function createPostNode(res, permanent) {
    if (!res)
        return null;
    post = document.getElementById("postTemplate");
    if (!post)
        return null;
    post = post.cloneNode(true);
    post.id = !!permanent ? ("post" + res["number"]) : "";
    post.style.display = "";
    var currentBoardName = document.getElementById("currentBoardName").value;
    var hidden = (getCookie("postHidden" + currentBoardName + res["number"]) === "true");
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
    if (!!res["showRegistered"] && !!res["showTripcode"] && !!res["tripcode"]) {
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
            number.href = "javascript:insertPostNumber(" + res["number"] + ");";
        else
            number.href = "#" + res["number"];
    } else {
        number.href = "/" + sitePathPrefix + currentBoardName + "/thread/" + res["threadNumber"] + ".html#"
        if (postingEnabled)
            number.href += "i";
        number.href += res["number"];
    }
    var files = post.querySelector("[name='files']");
    if (!!res["files"]) {
        for (var i = 0; i < res["files"].length; ++i) {
            var file = createPostFile(res["files"][i]);
            if (!!file)
                files.insertBefore(file, files.children[files.children.length - 1]);
        }
    }
    post.querySelector("[name='text']").innerHTML = res["text"];
    var bannedFor = post.querySelector("[name='bannedFor']");
    if (!!res["bannedFor"])
        bannedFor.style.display = "";
    else
        bannedFor.parentNode.removeChild(bannedFor);
    var perm = post.querySelector("[name='permanent']");
    if (!permanent) {
        perm.parentNode.removeChild(perm);
        return post;
    }
    perm.style.display = "";
    var anumber = document.createElement("a");
    number.parentNode.insertBefore(anumber, number);
    number.parentNode.removeChild(number);
    anumber.title = number.title;
    anumber.appendChild(number);
    if (postingEnabled)
        anumber.href = "javascript:insertPostNumber(" + res["number"] + ");";
    else
        anumber.href = "#" + res["number"];
    var deleteButton = post.querySelector("[name='deleteButton']");
    deleteButton.href = deleteButton.href.replace("%postNumber%", res["number"]);
    var hideButton = post.querySelector("[name='hideButton']");
    hideButton.id = hideButton.id.replace("%postNumber%", res["number"]);
    hideButton.href = hideButton.href.replace("%postNumber%", res["number"]);
    
    hideButton.href = hideButton.href.replace("%hide%", !hidden);
    var m = post.querySelector("[name='moder']");
    if (!moder) {
        m.parentNode.removeChild(m);
        return post;
    }
    m.style.display = "";
    var editButton = post.querySelector("[name='editButton']");
    editButton.href = editButton.href.replace("%postNumber%", res["number"]);
    var banButton = post.querySelector("[name='banButton']");
    banButton.href = banButton.href.replace("%postNumber%", res["number"]);
    var rawText = post.querySelector("[name='rawText']");
    rawText.id = rawText.id.replace("%postNumber%", res["number"]);
    rawText.value = res["rawPostText"];
    return post;
}

function viewPostStage2(link, postNumber, post) {
    post.onmouseout = function(event) {
        var next = post;
        while (!!next) {
            var list = traverseChildren(next);
            var e = event.toElement || event.relatedTarget;
            if (list.indexOf(e) >= 0)
                return;
            next = next.nextPostPreview;
        }
        post.parentNode.removeChild(post);
        if (post.previousPostPreview)
            post.previousPostPreview.onmouseout(event);
    };
    post.onmouseover = function(event) {
        post.mustHide = false;
    };
    if (!postPreviews[postNumber])
        postPreviews[postNumber] = post;
    else
        post.style.display = "";
    post.previousPostPreview = lastPostPreview;
    if (!!lastPostPreview)
        lastPostPreview.nextPostPreview = post;
    lastPostPreview = post;
    post.mustHide = true;
    if (!!lastPostPreviewTimer)
        clearTimeout(lastPostPreviewTimer);
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
    post.style.top = (doc.clientHeight - coords.bottom >= post.scrollHeight)
        ? (doc.scrollTop + coords.bottom - 4 + "px")
        : (doc.scrollTop + coords.top - post.scrollHeight - 4 + "px");
    post.style.zIndex = 9001;
}

function viewPost(link, boardName, postNumber) {
    if (!link || !boardName || isNaN(+postNumber))
        return;
    var post = document.getElementById("post" + postNumber);
    if (!post)
        post = postPreviews[postNumber];
    if (!post) {
        ajaxRequest("get_post", [boardName, +postNumber], 6, function(res) {
            post = createPostNode(res);
            if (!post)
                return;
            viewPostStage2(link, postNumber, post);
        });
    } else {
        post = post.cloneNode(true);
        post.className = post.className.replace("opPost", "post");
        var list = traverseChildren(post);
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
        viewPostStage2(link, postNumber, post);
    }
}

function noViewPost() {
    lastPostPreviewTimer = setTimeout(function() {
        if (!lastPostPreview)
            return;
        if (!!lastPostPreview.mustHide)
            lastPostPreview.parentNode.removeChild(lastPostPreview);
    }, 500);
}
