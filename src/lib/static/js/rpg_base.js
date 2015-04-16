/*ololord global object*/

var lord = lord || {};

/*Functions*/

lord.createPostNodeCustom = function(post, res, permanent, boardName) {
    var variants = res["voteVariants"];
    var tr = lord.nameOne("voteTr", post);
    if (variants && variants.length) {
        var enabled = !!res["voteEnabled"];
        var multiple = !!res["voteMultiple"];
        var div = lord.nameOne("voteVariants", post);
        lord.queryOne("input", div).value = multiple ? "true" : "false";
        var i = 0;
        variants.forEach(function(v) {
            var inp = lord.node("input");
            if (multiple) {
                inp.type = "checkbox";
                inp.name = "voteVariant" + i;
            } else {
                inp.type = "radio";
                inp.name = "voteGroup";
                inp.value = i;
            }
            if (!enabled)
                inp.disabled = "true";
            div.appendChild(inp);
            div.appendChild(lord.node("text", " " + v.text + " (" + v.voteCount + ")"));
            div.appendChild(lord.node("br"));
            ++i;
        });
        var btn = lord.queryOne("button", tr);
        if (enabled)
            btn.onclick = lord.vote.bind(lord, +res["number"]);
        else
            btn.disabled = "true";
    } else {
        tr.parentNode.removeChild(tr);
    }
};

lord.addVoteVariant = function(pos) {
    var parent = lord.id("voteVariants" + pos);
    var variants = lord.query("div > input", parent);
    var lastN = (variants && variants.length) ? +lord.last(variants).name.replace("voteVariant", "") : 0;
    var div = lord.node("div");
    var inp = lord.node("input");
    inp.type = "text";
    inp.name = "voteVariant" + (lastN + 1);
    div.appendChild(inp);
    var a = lord.node("a");
    a.href = "javascript:lord.removeVoteVariant('" + pos + "', " + (lastN + 1) + ");";
    var img = lord.node("img");
    img.src = "/" + lord.text("sitePathPrefix") + "img/delete.png";
    img.title = lord.text("removeVoteVariantText");
    a.appendChild(img);
    div.appendChild(a);
    parent.appendChild(div);
    lord.id("voteVariantCount" + pos).value = (lastN + 1);
};

lord.removeVoteVariant = function(pos, n) {
    var parent = lord.id("voteVariants" + pos);
    parent.removeChild(lord.nameOne("voteVariant" + n, parent).parentNode);
    var count = lord.id("voteVariantCount" + pos);
    var i = 0;
    count.value = i;
    lord.query("div > input", parent).forEach(function(inp) {
        ++i;
        inp.name = "voteVariant" + i;
        count.value = i;
    });
};

lord.vote = function(postNumber) {
    postNumber = +postNumber;
    if (isNaN(postNumber) || postNumber <= 0)
        return;
    var post = lord.id("post" + postNumber);
    if (!post)
        return;
    var votes = [];
    var variants = lord.nameOne("voteVariants", post);
    var multiple = ("true" == lord.queryOne("input[type='hidden']", variants).value);
    if (multiple) {
        lord.query("input[type='checkbox']").forEach(function(inp) {
            if (!!inp.checked)
                votes.push(+inp.name.replace("voteVariant", ""));
        });
    } else {
        lord.query("input[type='radio']").forEach(function(inp) {
            if (!!inp.checked)
                votes.push(+inp.value);
        });
    }
    lord.ajaxRequest("vote", [postNumber, votes], 11, function() {
        lord.updatePost("rpg", postNumber, post);
    });
};
