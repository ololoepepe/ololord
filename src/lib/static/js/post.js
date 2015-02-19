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
