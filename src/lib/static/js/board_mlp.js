function nextStage() {
    document.getElementById("theVideo").pause();
    document.getElementById("firstStageVideo").className += " hiddenVideo";
    document.getElementById("nextStageButton").className += " hiddenButton";
    var nsv = document.getElementById("nextStageVideo");
    nsv.className = nsv.className.replace(" hiddenVideo", "");
    document.getElementById("theVideo2").play();
}
