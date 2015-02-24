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
        post.style.display = "none";
        if (post.previousPostPreview)
            post.previousPostPreview.onmouseout(event);
    };
    post.onmouseover = function(event) {
        post.mustHide = false;
    };
    post.style.position = "absolute";
    var offs = cumulativeOffset(link);
    post.style.left = (offs.left + link.offsetWidth + 20) + "px";
    post.style.top = offs.top + "px";
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
}

function viewPost(link, boardName, postNumber, threadNumber) {
    if (!link || !boardName || isNaN(+postNumber) || isNaN(+threadNumber))
        return;
    var post = postPreviews[postNumber];
    if (!post)
        post = document.getElementById("post" + postNumber);
    if (!post) {
        ajaxRequest("get_post", [boardName, +postNumber, +threadNumber], 6, function(res) {
            post = document.getElementById("postTemplate");
            if (!post)
                return;
            post = post.cloneNode(true);
            post.id = "";
            post.style.display = "";
            var list = traverseChildren(post);
            for (var i = 0; i < list.length; ++i) {
                var c = list[i];
                switch (c.id) {
                case "postTemplateSubject":
                    c.appendChild(document.createTextNode(res["subject"]));
                    break;
                case "postTemplateRegistered":
                    if (!!res["showRegistered"] && !!res["showTripcode"])
                        c.style.display = "";
                    break;
                case "postTemplateName":
                    if (!!res["email"])
                        c.innerHTML = "<a href='mailto:" + res["email"] + "'>" + res["nameRaw"] + "</a>";
                    else
                        c.innerHTML = res["name"];
                    break;
                case "postTemplateTripcode":
                    if (!!res["showRegistered"] && !!res["showTripcode"] && !!res["tripcode"])
                        c.style.display = "";
                    break;
                case "postTemplateWhois":
                    if (!!res["flagName"]) {
                        c.style.display = "";
                        c.href = "/" + document.getElementById("sitePathPrefix") + "img/flag/" + res["flagName"];
                        c.title = res["countryName"];
                        if (!!res["cityName"])
                            c.title += ": " + res["cityName"];
                    }
                    break;
                case "postTemplateDateTime":
                    c.appendChild(document.createTextNode(res["dateTime"]));
                    break;
                case "postTemplateNumber":
                    c.appendChild(document.createTextNode(res["number"]));
                    break;
                case "postTemplateFiles":
                    var files = res["files"];
                    if (!!files) {
                        for (var i = 0; i < files.length; ++i) {
                            var file = createPostFile(files[i]);
                            if (!!file)
                                c.insertBefore(file, c.children[c.children.length - 1]);
                        }
                    }
                    break;
                case "postTemplateText":
                    c.innerHTML= res["text"];
                    break;
                case "postTemplateBannedFor":
                    if (!!res["bannedFor"])
                        c.style.display = "";
                    break;
                default:
                    break;
                }
                c.id = "";
            }
            viewPostStage2(link, postNumber, post);
        });
    } else {
        post = post.cloneNode(true);
        post.className = post.className.replace("opPost", "post");
        var list = traverseChildren(post);
        for (var i = 0; i < list.length; ++i)
            list[i].id = "";
        viewPostStage2(link, postNumber, post);
    }
}

function noViewPost() {
    lastPostPreviewTimer = setTimeout(function() {
        if (!lastPostPreview)
            return;
        if (!!lastPostPreview.mustHide)
            lastPostPreview.style.display = "none";
    }, 500);
}
