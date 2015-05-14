<script type="text/javascript">
/*ololord global object*/

var lord = lord || {};

/*Functions*/

lord.reloadCaptchaFunction = function() {
    var captcha = lord.id("captcha");
    if (!captcha)
        return lord.showPopup("No captcha", {type: "critical"});
    var image = lord.nameOne("image", captcha);
    var challenge = lord.nameOne("yandexCaptchaChallenge", captcha);
    var response = lord.nameOne("yandexCaptchaResponse", captcha);
    if (!challenge || !response)
        return lord.showPopup("No challenge/response", {type: "critical"});
    response.value = "";
    if (image.firstChild)
        image.removeChild(image.firstChild);
    lord.ajaxRequest("get_yandex_captcha_image", ["%type%"], 16, function(res) {
        challenge.value = res["challenge"];
        var img = lord.node("img");
        img.src = "//" + res["url"].replace("https://", "").replace("http://", "");
        img.onclick = lord.reloadCaptchaFunction.bind(lord);
        image.appendChild(img);
    }, function() {
        var img = lord.node("img");
        img.src = "/" + lord.text("sitePathPrefix") + "img/yandex-hernya.png";
        img.onclick = lord.reloadCaptchaFunction.bind(lord);
        image.appendChild(img);
    });
};

window.addEventListener("load", function load() {
    window.removeEventListener("load", load, false);
    lord.reloadCaptchaFunction();
}, false);
</script>
