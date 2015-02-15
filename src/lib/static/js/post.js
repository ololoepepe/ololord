function deletePost(boardName, postNumber, fromThread) {
    if (!boardName || isNaN(postNumber))
        return;
    var text = document.getElementById("enterPasswordText").value;
    var pwd = prompt(text);
    if (!pwd)
        return;
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
                if (0 === res.length) {
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
