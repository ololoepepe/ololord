var searchFormVisible = {
    "top": false,
    "bottom": false
};

function showHideSearchForm(position) {
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
