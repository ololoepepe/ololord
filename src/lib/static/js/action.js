var searchFormVisible = {
    "top": false,
    "bottom": false
};
var postFormVisible = {
    "top": false,
    "bottom": false
};

function showHideSearchForm(position) {
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
}
