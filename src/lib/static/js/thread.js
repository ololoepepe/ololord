var lastSelectedElement = null;
var popups = [];
var autoUpdateTimer = null;

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

function insertPostNumberInternal(postNumber, position) {
    var field = document.getElementById("postFormInputText" + position);
    var value = ">>" + postNumber;
    if (document.selection) {
        field.focus();
        var sel = document.selection.createRange();
        sel.text = value;
    } else if (field.selectionStart || field.selectionStart == "0") {
        var startPos = field.selectionStart;
        var endPos = field.selectionEnd;
        field.value = field.value.substring(0, startPos) + value + field.value.substring(endPos, field.value.length);
    } else {
        field.value += value;
    }
}

function insertPostNumber(postNumber) {
    insertPostNumberInternal(postNumber, "Top");
    insertPostNumberInternal(postNumber, "Bottom");
}

function selectPost(post) {
    if (isNaN(+post))
        return;
    if (!!lastSelectedElement)
        lastSelectedElement.style.backgroundColor = "#DDDDDD";
    lastSelectedElement = document.getElementById("post" + post);
    if (!!lastSelectedElement)
        lastSelectedElement.style.backgroundColor = "#EEDACB";
    if (window.location.href.split("#").length < 2)
        window.location.href = window.location.href + "#" + post;
    else
        window.location.href = window.location.href.split("#")[0] + "#" + post;
}

function updateThread(boardName, threadNumber, autoUpdate) {
    if (!boardName || isNaN(+threadNumber))
        return;
    var posts = document.querySelectorAll(".opPost, .post");
    if (!posts)
        return;
    var lastPostN = posts[posts.length - 1].id.replace("post", "");
    ajaxRequest("get_new_posts", [boardName, +threadNumber, +lastPostN], 7, function(res) {
        if (!res)
            return;
        var txt = document.getElementById((res.length >= 1) ? "newPostsText" : "noNewPostsText").value;
        if (res.length >= 1)
            txt += " " + res.length;
        if (!autoUpdate)
            showPopup(txt, 5000, "noNewPostsPopup");
        if (res.length < 1)
            return;
        var before = document.getElementById("afterAllPosts");
        if (!before)
            return;
        for (var i = 0; i < res.length; ++i) {
            var post = createPostNode(res[i], true);
            if (!post)
                continue;
            document.body.insertBefore(post, before);
        }
    });
}

function setAutoUpdateEnabled(cbox) {
    var enabled = !!cbox.checked;
    document.getElementById("autoUpdate_top").checked = enabled;
    document.getElementById("autoUpdate_bottom").checked = enabled;
    if (enabled) {
        autoUpdateTimer = setInterval(function() {
            var boardName = document.getElementById("currentBoardName").value;
            var threadNumber = document.getElementById("currentThreadNumber").value;
            updateThread(boardName, threadNumber, true);
        }, 15000);
    } else {
        if (!!autoUpdateTimer) {
            clearInterval(autoUpdateTimer);
            autoUpdateTimer = null;
        }
    }
    setCookie("auto_update", enabled, {
        "expires": Billion
    });
}

function initializeOnLoadThread() {
    if (getCookie("auto_update") === "true") {
        var cbox = document.getElementById("autoUpdate_top");
        cbox.checked = true;
        setAutoUpdateEnabled(cbox);
    }
    var sl = window.location.href.split("#");
    if (sl.length != 2)
        return;
    var post = sl[1];
    if (post.substring(0, 1) === "i") {
        post = post.substring(1);
        if (isNaN(+post))
            return;
        showHidePostForm("Top");
        insertPostNumber(post);
    } else {
        selectPost(post);
    }
}
