var searchFormVisible = {
    "top": false,
    "bottom": false
};
var threadFormVisible = {
    "top": false,
    "bottom": false
};

function showHideSearchForm(position) {
    if (threadFormVisible[position])
        showHideThreadForm(position);
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

function showHideThreadForm(position) {
    if (searchFormVisible[position])
        showHideSearchForm(position);
    var theButton = document.getElementById("showHideThreadFormButton" + position);
    if (threadFormVisible[position]) {
        theButton.innerHTML = document.getElementById("showThreadFormText").value;
        document.getElementById("threadForm" + position).className = "postFormInvisible";
    } else {
        theButton.innerHTML = document.getElementById("hideThreadFormText").value;
        document.getElementById("threadForm" + position).className = "postFormVisible";
        var captcha = document.getElementById("googleCaptcha");
        if (!!captcha) {
            var p = ("Top" === position) ? "Bottom" : "Top";
            if (threadFormVisible[p])
                showHideThreadForm(p);
            document.getElementById("googleCaptcha" + position).appendChild(captcha);
        }
    }
    threadFormVisible[position] = !threadFormVisible[position];
}

function initializeOnLoad() {
    //
}
