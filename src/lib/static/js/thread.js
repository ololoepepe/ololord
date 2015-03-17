var lastSelectedElement = null;
var autoUpdateTimer = null;

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

function updateThread(boardName, threadNumber, autoUpdate, extraCallback) {
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
        if (!!extraCallback)
            extraCallback();
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

function posted() {
    if (!formSubmitted)
        return;
    var iframe = document.getElementById("kostyleeque");
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var postNumber = iframeDocument.querySelector("#postNumber");
    var referencedPosts = iframeDocument.querySelectorAll("[name='referencedPost']");
    formSubmitted.querySelector("[name='submit']").disabled = false;
    if (!!postNumber) {
        formSubmitted.reset();
        formSubmitted = null;
        var boardName = document.getElementById("currentBoardName").value;
        var threadNumber = document.getElementById("currentThreadNumber").value;
        updateThread(boardName, threadNumber, true, function() {
            selectPost(postNumber.value);
        });
        if (!!referencedPosts) {
            var refs = [];
            for (var i = 0; i < referencedPosts.length; ++i)
                refs.push(+referencedPosts[i].value);
            addReferences(postNumber.value, refs);
        }
        grecaptcha.reset();
    } else {
        formSubmitted = null;
        var errmsg = iframeDocument.querySelector("#errorMessage");
        var errdesc = iframeDocument.querySelector("#errorDescription");
        showPopup(errmsg.innerHTML + ": " + errdesc.innerHTML);
        grecaptcha.reset();
    }
}

function initializeOnLoadThread() {
    document.body.onclick = globalOnclick;
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
