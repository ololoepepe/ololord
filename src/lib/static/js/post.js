function deletePost(boardName, postNumber, fromThread) {
    if (!boardName || isNaN(postNumber))
        return;
    var pwd = prompt(document.getElementById("enterPasswordText").value);
    if (null === pwd)
        return;
    if (pwd.length < 1) {
        pwd = getCookie("hashpass");
        if (!pwd)
            return alert(document.getElementById("notLoggedInText").value);
    } else if (!isHashpass(pwd)) {
        pwd = toHashpass(pwd);
    }
    var xhr = new XMLHttpRequest();
    xhr.withCredentials = true;
    var prefix = document.getElementById("sitePathPrefix").value;
    xhr.open("post", "/" + prefix + "api");
    xhr.setRequestHeader("Content-Type", "application/json");
    var request = '{"method": "delete_post", "params": ["' + boardName + '", ' + postNumber + ', "'
        + pwd + '"], "id": 1}';
    xhr.onreadystatechange = function() {
        if (xhr.readyState === 4) {
            if (xhr.status === 200) {
                var res = JSON.parse(xhr.responseText).result;
                if (null === res) {
                    alert("Error"); //TODO
                } else if (res.length < 1) {
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
                } else {
                    alert(res);
                }
            } else {
                alert("Error: " + xhr.status); //TODO
            }
        }
    };
    xhr.send(request);
}
