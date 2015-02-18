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
            }
            post.parentNode.removeChild(post);
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
