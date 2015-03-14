function selectPost(post, threadNumber) {
    if (isNaN(+post) || isNaN(+threadNumber))
        return;
    window.location.href = window.location.href + "/thread/" + threadNumber + ".html#" + post;
}

function posted() {
    if (!formSubmitted)
        return;
    var iframe = document.getElementById("kostyleeque");
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var threadNumber = iframeDocument.querySelector("#threadNumber");
    if (!!threadNumber) {
        window.location.href = window.location.href + "/thread/" + threadNumber.value + ".html";
    } else {
        formSubmitted = null;
        var errmsg = iframeDocument.querySelector("#errorMessage");
        var errdesc = iframeDocument.querySelector("#errorDescription");
        showPopup(errmsg.innerHTML + ": " + errdesc.innerHTML);
        grecaptcha.reset();
    }
}

function initializeOnLoadBoard() {
    document.body.onclick = globalOnclick;
}
