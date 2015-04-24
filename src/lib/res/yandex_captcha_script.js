<script type="text/javascript">
/*ololord global object*/

var lord = lord || {};

/*Functions*/

lord.reloadCaptchaFunction = function() {
    var captcha = lord.id("captcha");
    if (!captcha)
        return alert("no captcha");
    var image = lord.nameOne("image", captcha);
    var id = lord.nameOne("yandexCaptchaId", captcha);
    var challenge = lord.nameOne("yandexCaptchaChallenge", captcha);
    var response = lord.nameOne("yandexCaptchaResponse", captcha);
    if (!id || !challenge || !response)
        return alert("no chall/resp");
    response.value = "";
    if (image.firstChild)
        image.removeChild(image.firstChild);
    lord.ajaxRequest("get_yandex_captcha_image", ["%type%"], 16, function(res) {
        id.value = res["id"];
        challenge.value = res["challenge"];
        var img = lord.node("img");
        img.src = res["url"];
        img.onclick = lord.reloadCaptchaFunction.bind(lord);
        image.appendChild(img);
    });
}

window.addEventListener("load", function load() {
    window.removeEventListener("load", load, false);
    lord.reloadCaptchaFunction();
}, false);
</script>
