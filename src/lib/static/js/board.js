function selectPost(post, threadNumber) {
    if (isNaN(+post) || isNaN(+threadNumber))
        return;
    window.location.href = window.location.href + "/thread/" + threadNumber + ".html#" + post;
}

function initializeOnLoadBoard() {
    //
}
