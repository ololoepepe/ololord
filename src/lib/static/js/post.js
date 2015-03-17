var postPreviews = {};
var lastPostPreview = null;
var lastPostPreviewTimer = null;
var formSubmitted = null;
var popups = [];
var images = {};
var img = null;

function toCenter(element, sizeHintX, sizeHintY) {
    var doc = document.documentElement;
    element.style.left = (doc.clientWidth / 2 - sizeHintX / 2) + "px";
    element.style.top = (doc.clientHeight / 2 - sizeHintY / 2) + "px";
}

function resetScale(image) {
    var k = (image.scale / 100);
    var tr = "scale(" + k + ", " + k + ")";
    //Fuck you all who create those stupid browser-specific features
    image.style.webkitTransform = tr;
    image.style.MozTransform = tr;
    image.style.msTransform = tr;
    image.style.OTransform = tr;
    image.style.transform = tr;
}

function removeReferences(postNumber) {
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
}

function addReferences(postNumber, referencedPosts) {
    postNumber = +postNumber;
    if (isNaN(postNumber))
        return;
    if (!referencedPosts)
        return;
    var currentBoardName = document.getElementById("currentBoardName").value;
    for (var i = 0; i < referencedPosts.length; ++i) {
        var pn = +referencedPosts[i];
        if (isNaN(pn))
            continue;
        var post = document.getElementById("post" + pn);
        if (!post)
            continue;
        var referencedByTr = post.querySelector("[name='referencedByTr']");
        referencedByTr.style.display = "";
        var referencedBy = post.querySelector("[name='referencedBy']");
        var a = document.createElement("a");
        a.href = "javascript:void(0);";
        a.addEventListener("mouseover", function(e) {
            viewPost(this, currentBoardName, postNumber);
        });
        a.onmouseout = function() {
            noViewPost();
        };
        referencedBy.appendChild(document.createTextNode(" "));
        a.appendChild(document.createTextNode(">>" + postNumber));
        referencedBy.appendChild(a);
    }
}

function setInitialScale(image, sizeHintX, sizeHintY) {
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
}

function showPopup(text, timeout, additionalClassNames) {
    if (!text)
        return;
    if (isNaN(+timeout))
        timeout = 5000;
    var msg = document.createElement("div");
    msg.className = "popup";
    if (!!additionalClassNames)
        msg.className += " " + additionalClassNames;
    if (popups.length > 0) {
        var prev = popups[popups.length - 1];
        msg.style.top = (prev.offsetTop + prev.offsetHeight + 5) + "px";
    }
    msg.appendChild(document.createTextNode(text));
    document.body.appendChild(msg);
    popups.push(msg);
    setTimeout(function() {
        var offsH = msg.offsetHeight + 5;
        document.body.removeChild(msg);
        var ind = popups.indexOf(msg);
        if (ind < 0)
            return;
        popups.splice(ind, 1);
        for (var i = 0; i < popups.length; ++i) {
            var top = +popups[i].style.top.replace("px", "");
            top -= offsH;
            popups[i].style.top = top + "px";
        }
    }, 5000);
}

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
    var post = document.getElementById("post" + postNumber);
    if (!post)
        return;
    var postText = document.getElementById("post" + postNumber + "RawText");
    if (!postText)
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
    text.appendChild(document.createTextNode(postText.value));
    var moder = (document.getElementById("moder").value === "true");
    showDialog(title, null, form, function() {
        var params = {
            "boardName": boardName,
            "postNumber": +postNumber,
            "text": text.value,
            "email": email.value,
            "name": name.value,
            "subject": subject.value,
            "raw": !!moder ? form.querySelector("[name='raw']").checked : false
        };
        ajaxRequest("edit_post", [params], 5, function(rese) {
            ajaxRequest("get_post", [boardName, +postNumber], 6, function(res) {
                var newPost = createPostNode(res, true);
                if (!newPost)
                    return;
                removeReferences(postNumber);
                addReferences(postNumber, rese);
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
                removeReferences(postNumber);
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
    divFileSize.appendChild(document.createTextNode("(" + f["size"] + ")"));
    file.appendChild(divFileSize);
    var divImage = document.createElement("div");
    var aImage = document.createElement("a");
    aImage.href = "/" + sitePrefix + currentBoardName + "/" + f["sourceName"];
    if ("image" === f["type"]) {
        aImage.onclick = function(e) {
            return showImage("/" + sitePrefix + currentBoardName + "/" + f["sourceName"], f["type"], f["sizeX"],
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
    var bannedForTd = post.querySelector("[name='bannedForTd']");
    var referencedByTd = post.querySelector("[name='referencedByTd']");
    var manyFilesTr = post.querySelector("[name='manyFilesTr']");
    var textOneFile = post.querySelector("[name='textOneFile']");
    if (res["files"].length <= 1) {
        manyFilesTr.parentNode.removeChild(manyFilesTr);
        textOneFile.innerHTML = res["text"];
    } else {
        post.querySelector("[name='oneFileTd']").className = "shrink";
        textOneFile.parentNode.removeChild(textOneFile);
        post.querySelector("[name='manyFilesTd']").colSpan = res["files"].length + 2;
        post.querySelector("[name='textManyFiles']").innerHTML = res["text"];
        bannedForTd.colSpan = res["files"].length + 2;
        referencedByTd.colSpan = res["files"].length + 2;
    }
    var bannedFor = post.querySelector("[name='bannedFor']");
    if (!!res["bannedFor"])
        bannedFor.style.display = "";
    else
        bannedFor.parentNode.removeChild(bannedFor);
    var referencedBy = post.querySelector("[name='referencedBy']");
    if (!!res["referencedBy"] && res["referencedBy"].length > 0) {
        referencedBy.style.display = "";
        for (var i = 0; i < res["referencedBy"].length; ++i) {
            var pn = res["referencedBy"][i];
            var a = document.createElement("a");
            a.href = "javascript:void(0);";
            a.addEventListener("mouseover", function(e) {
                viewPost(this, currentBoardName, pn);
            });
            a.onmouseout = function() {
                noViewPost();
            };
            referencedBy.appendChild(document.createTextNode(" "));
            a.appendChild(document.createTextNode(">>" + pn));
            referencedBy.appendChild(a);
        }
    } else {
        referencedBy.parentNode.removeChild(referencedBy);
    }
    var perm = post.querySelector("[name='permanent']");
    if (!permanent) {
        perm.parentNode.removeChild(perm);
        return post;
    }
    post.className += " newPost";
    post.onmouseover = function() {
        post.className = post.className.replace(" newPost", "");
        post.onmouseover = null;
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
    var fixButton = post.querySelector("[name='fixButton']");
    var unfixButton = post.querySelector("[name='unfixButton']");
    var openButton = post.querySelector("[name='openButton']");
    var closeButton = post.querySelector("[name='closeButton']");
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
    var banButton = post.querySelector("[name='banButton']");
    banButton.href = banButton.href.replace("%postNumber%", res["number"]);
    var rawText = post.querySelector("[name='rawText']");
    rawText.id = rawText.id.replace("%postNumber%", res["number"]);
    rawText.value = res["rawPostText"];
    post.querySelector("[name='email']").value = res["email"];
    post.querySelector("[name='name']").value = res["rawName"];
    post.querySelector("[name='subject']").value = res["rawSubject"];
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
        if (!!post.parentNode)
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
    if (!!lastPostPreviewTimer) {
        clearTimeout(lastPostPreviewTimer);
        lastPostPreviewTimer = null;
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
            post["fromAjax"] = true;
            viewPostStage2(link, postNumber, post);
        });
    } else {
        var fromAjax = !!post.fromAjax;
        post = post.cloneNode(true);
        if (fromAjax) {
            post.addEventListener("mouseover", function(e) {
                var a = e.target;
                if (a.tagName != "A")
                    return;
                var pn = +a.innerHTML.replace("&gt;&gt;", "");
                if (isNaN(pn))
                    return;
                viewPost(a, boardName, pn);
            });
            post.addEventListener("mouseout", function(e) {
                var a = e.target;
                if (a.tagName != "A")
                    return;
                var pn = +a.innerHTML.replace("&gt;&gt;", "");
                if (isNaN(pn))
                    return;
                noViewPost();
            });
        }
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
        if (!!lastPostPreview.mustHide && !!lastPostPreview.parentNode)
            lastPostPreview.parentNode.removeChild(lastPostPreview);
    }, 500);
}

function clearFileInput(div) {
    var preview = div.querySelector("img");
    if (!!preview && div == preview.parentNode)
        preview.src = "/" + document.getElementById("sitePathPrefix").value + "img/addfile.png";
    var span = div.querySelector("span");
    if (!!span && div == span.parentNode && !!span.childNodes && !!span.childNodes[0])
        span.removeChild(span.childNodes[0]);
}

function readableSize(sz) {
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
}

function fileSelected(current) {
    if (!current)
        return;
    var inp = document.getElementById("maxFileCount");
    if (!inp)
        return;
    var maxCount = +inp.value;
    if (isNaN(maxCount))
        return;
    var div = current.parentNode;
    clearFileInput(div);
    var file = current.files[0];
    div.querySelector("span").appendChild(document.createTextNode(file.name + " (" + readableSize(file.size) + ")"));
    if (!!current.value.match(/\.(jpe?g|png|gif)$/i)) {
        var reader = new FileReader();
        reader.readAsDataURL(file);
        var oldDiv = div;
        reader.onload = function(e) {
            oldDiv.querySelector("img").src = e.target.result;
        };
    } else if (!!current.value.match(/\.(webm)$/i)) {
        div.querySelector("img").src = "/" + document.getElementById("sitePathPrefix").value + "img/webm_file.png";
    } else {
        div.querySelector("img").src = "/" + document.getElementById("sitePathPrefix").value + "img/file.png";
    }
    div.querySelector("a").style.display = "inline";
    var parent = div.parentNode;
    if (parent.children.length >= maxCount)
        return;
    for (var i = 0; i < parent.children.length; ++i) {
        if (parent.children[i].querySelector("input").value === "")
            return;
        parent.children[i].querySelector("a").style.display = "inline";
    }
    div = div.cloneNode(true);
    clearFileInput(div);
    div.querySelector("a").style.display = "none";
    div.innerHTML = div.innerHTML; //NOTE: Workaround since we can't clear it other way
    parent.appendChild(div);
}

function removeFile(current) {
    if (!current)
        return;
    var div = current.parentNode;
    var parent = div.parentNode;
    parent.removeChild(div);
    clearFileInput(div);
    if (parent.children.length > 1) {
        for (var i = 0; i < parent.children.length; ++i) {
            if (parent.children[i].querySelector("input").value === "")
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
    if (parent.children.length > 0 && parent.children[0].querySelector("input").value === "")
        parent.children[0].querySelector("a").style.display = "none";
    if (parent.children.length < 1) {
        div.querySelector("a").style.display = "none";
        div.innerHTML = div.innerHTML; //NOTE: Workaround since we can't clear it other way
        parent.appendChild(div);
    }
}

function browseFile(e, div) {
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
}

function submitted(form) {
    formSubmitted = form;
    form.querySelector("[name='submit']").disabled = true;
}

function hideImage() {
    if (!!img) {
        if ("webm" === img.fileType) {
            img.pause();
            img.load();
        }
        img.style.display = "none";
        img = null;
    }
}

function globalOnclick(e) {
    if (!!e.button)
        return;
    var t = e.target;
    if (!!t && !!img && t == img)
        return;
    while (!!t) {
        if (t.tagName === "A" && (!!t.onclick || !!t.onmousedown || !!t.href))
            return;
        t = t.parentNode;
    }
    hideImage();
}

function showImage(href, type, sizeHintX, sizeHintY) {
    hideImage();
    if (!href || !type)
        return true;
    img = images[href];
    if (!!img) {
        setInitialScale(img, sizeHintX, sizeHintY);
        resetScale(img);
        img.style.display = "";
        toCenter(img, sizeHintX, sizeHintY);
        if ("webm" === img.fileType) {
            setTimeout(function() {
                img.play();
            }, 500);
        }
        return false;
    }
    if (!sizeHintX || !sizeHintY || sizeHintX <= 0 || sizeHintY <= 0)
        return true;
    if ("image" === type) {
        img = document.createElement("img");
        img.src = href;
    } else if ("webm" === type) {
        img = document.createElement("video");
        img.controls = "controls";
        var src = document.createElement("source");
        src.src = href;
        src.type = "video/webm";
        img.appendChild(src);
    }
    img.fileType = type;
    setInitialScale(img, sizeHintX, sizeHintY);
    resetScale(img);
    img.moving = false;
    img.coord = {
        "x": 0,
        "y": 0
    };
    img.initialCoord = {
        "x": 0,
        "y": 0
    };
    img.className = "movableImage";
    var wheelHandler = function(e) {
        var e = window.event || e; //Old IE support
        e.preventDefault();
        var delta = Math.max(-1, Math.min(1, (e.wheelDelta || -e.detail)));
        img.scale += delta;
        resetScale(img);
    };
    if (img.addEventListener) {
    	img.addEventListener("mousewheel", wheelHandler, false); //IE9, Chrome, Safari, Opera
	    img.addEventListener("DOMMouseScroll", wheelHandler, false); //Firefox
    } else {
        img.attachEvent("onmousewheel", wheelHandler); //IE 6/7/8
    }
    if ("image" === type) {
        img.onmousedown = function(e) {
            if (!!e.button)
                return;
            e.preventDefault();
            img.moving = true;
            img.coord.x = e.clientX;
            img.coord.y = e.clientY;
            img.initialCoord.x = e.clientX;
            img.initialCoord.y = e.clientY;
        };
        img.onmouseup = function(e) {
            if (!!e.button)
                return;
            e.preventDefault();
            img.moving = false;
            if (img.initialCoord.x === e.clientX && img.initialCoord.y === e.clientY) {
                if ("webm" === type) {
                    img.pause();
                    img.currentTime = 0;
                }
                img.style.display = "none";
            }
        };
        img.onmousemove = function(e) {
            if (!img.moving)
                return;
            e.preventDefault();
            var dx = e.clientX - img.coord.x;
            var dy = e.clientY - img.coord.y;
            img.style.left = (img.offsetLeft + dx) + "px";
            img.style.top = (img.offsetTop + dy) + "px";
            img.coord.x = e.clientX;
            img.coord.y = e.clientY;
        };
    }
    document.body.appendChild(img);
    toCenter(img, sizeHintX, sizeHintY);
    if ("webm" === img.fileType) {
        setTimeout(function() {
            img.play();
        }, 500);
    }
    images[href] = img;
    return false;
}

function complain() {
    alert(document.getElementById("complainMessage").value);
}
