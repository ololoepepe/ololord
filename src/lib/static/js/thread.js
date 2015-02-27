var lastSelectedElement = null;

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
    lastSelectedElement.style.backgroundColor = "#EEDACB";
    if (window.location.href.split("#").length < 2)
        window.location.href = window.location.href + "#" + post;
    else
        window.location.href = window.location.href.split("#")[0] + "#" + post;
}

function updateThread(boardName, threadNumber) {
    if (!boardName || isNaN(+threadNumber))
        return;
    var lastPostNumber = document.getElementById("lastPostNumber").value;
    ajaxRequest("get_new_posts", [boardName, +threadNumber, +lastPostNumber], 7, function(res) {
        if (!res)
            return;
        var msg = document.createElement("div");
        msg.className = "popup noNewPostsPopup";
        var txt = document.getElementById((res.length >= 1) ? "newPostsText" : "noNewPostsText").value;
        if (res.length >= 1)
            txt += " " + res.length;
        msg.appendChild(document.createTextNode(txt));
        document.body.appendChild(msg);
        setTimeout(function() {
            document.body.removeChild(msg);
        }, 5000);
        if (res.length < 1)
            return;
        var before = document.getElementById("afterAllPosts");
        if (!before)
            return;
        for (var i = 0; i < res.length; ++i) {
            var post = createPostNode(res[i]);
            if (!post)
                continue;
            document.body.insertBefore(post, before);
            lastPostNumber.value = res["number"];
        }
    });
}

function initializeOnLoadThread() {
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
