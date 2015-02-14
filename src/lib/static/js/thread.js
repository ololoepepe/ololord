/*var searchFormVisible = {
    "top": false,
    "bottom": false
};
var postFormVisible = {
    "top": false,
    "bottom": false
};*/

var lastSelectedElement = null;

/*function showHideSearchForm(position) {
    if (postFormVisible[position])
        showHidePostForm(position);
    var theButton = document.getElementById("showHideSearchFormButton" + position);
    if (searchFormVisible[position]) {
        theButton.innerHTML = document.getElementById("showSearchFormText").value;
        document.getElementById("searchForm" + position).className = "searchFormInvisible";
    } else {
        theButton.innerHTML = document.getElementById("hideSearchFormText").value;
        document.getElementById("searchForm" + position).className = "searchFormVisible";
    }
    searchFormVisible[position] = !searchFormVisible[position];
}

function showHidePostForm(position) {
    if (searchFormVisible[position])
        showHideSearchForm(position);
    var theButton = document.getElementById("showHidePostFormButton" + position);
    if (postFormVisible[position]) {
        theButton.innerHTML = document.getElementById("showPostFormText").value;
        document.getElementById("postForm" + position).className = "postFormInvisible";
    } else {
        theButton.innerHTML = document.getElementById("hidePostFormText").value;
        document.getElementById("postForm" + position).className = "postFormVisible";
        var captcha = document.getElementById("googleCaptcha");
        if (!!captcha) {
            var p = ("Top" === position) ? "Bottom" : "Top";
            if (postFormVisible[p])
                showHidePostForm(p);
            document.getElementById("googleCaptcha" + position).appendChild(captcha);
        }
    }
    postFormVisible[position] = !postFormVisible[position];
}*/

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
